#include "tms9129.h"

namespace ein {
namespace {

// Standard TMS9918A palette. Index 0 is transparent; with nothing behind the
// screen it shows the backdrop, so it is rendered as the backdrop by callers.
constexpr std::uint32_t kPalette[16] = {
    0xff000000,  // 0  transparent
    0xff000000,  // 1  black
    0xff21c842,  // 2  medium green
    0xff5edc78,  // 3  light green
    0xff5455ed,  // 4  dark blue
    0xff7d76fc,  // 5  light blue
    0xffd4524d,  // 6  dark red
    0xff42ebf5,  // 7  cyan
    0xfffc5554,  // 8  medium red
    0xffff7978,  // 9  light red
    0xffd4c154,  // 10 dark yellow
    0xffe6ce80,  // 11 light yellow
    0xff21b03b,  // 12 dark green
    0xffc95bba,  // 13 magenta
    0xffcccccc,  // 14 grey
    0xffffffff,  // 15 white
};

}  // namespace

std::uint32_t Tms9129::palette(int index) { return kPalette[index & 15]; }

void Tms9129::reset() {
    vram_.fill(0);
    reg_.fill(0);
    status_ = 0;
    line_ = 0;
    line_accum_ = 0;
    ts_ = 0;
    last_access_ts_ = 0;
    fast_accesses_ = 0;
    min_access_gap_ = 1 << 30;
    latched_ = false;
    latch_value_ = 0;
    read_buffer_ = 0;
    addr_ = 0;
    frame_ts_ = 0;
    vblank_hit_ = false;
    frames_ = 0;
    reg_writes_ = 0;
    vram_writes_ = 0;
}

// Accessing the data port always clears the control-port latch.
std::uint8_t Tms9129::read_data() {
    latched_ = false;
    note_access();
    const std::uint8_t v = read_buffer_;
    read_buffer_ = vram_[addr_];
    bump_addr();
    return v;
}

void Tms9129::write_data(std::uint8_t v) {
    latched_ = false;
    note_access();
    vram_[addr_] = v;
    read_buffer_ = v;
    bump_addr();
    ++vram_writes_;
}

// Records how close together the CPU is driving VRAM accesses while the
// display is fetching. See kMinAccessTStates.
void Tms9129::note_access() {
    if (display_enabled() && line_ < kActiveLines) {
        const int gap = static_cast<int>(ts_ - last_access_ts_);
        if (gap < min_access_gap_) min_access_gap_ = gap;
        if (gap < kMinAccessTStates) ++fast_accesses_;
    }
    last_access_ts_ = ts_;
}

std::uint8_t Tms9129::read_status() {
    const std::uint8_t v = status_;
    // Reading the status clears the frame flag, the collision flag and the
    // control-port latch, and releases INT.
    status_ &= 0x1f;
    latched_ = false;
    return v;
}

void Tms9129::write_ctrl(std::uint8_t v) {
    if (!latched_) {
        latch_value_ = v;
        latched_ = true;
        // A first write also updates the low bits of the address on real
        // hardware; harmless to mirror.
        addr_ = std::uint16_t((addr_ & 0x3f00) | v);
        return;
    }
    latched_ = false;

    if (v & 0x80) {
        reg_[v & 0x07] = latch_value_;
        ++reg_writes_;
        return;
    }
    addr_ = std::uint16_t(((v & 0x3f) << 8) | latch_value_);
    if (!(v & 0x40)) {
        // Read setup: the VDP prefetches the first byte.
        read_buffer_ = vram_[addr_];
        bump_addr();
    }
}

void Tms9129::tick(int tstates) {
    // Called for every bus access, so this is the hottest path in the emulator:
    // compare against a precomputed threshold rather than dividing per tick.
    frame_ts_ += tstates;
    ts_ += static_cast<std::uint64_t>(tstates);

    // Scanline position, scaled by the line count so it stays exact in
    // integers: one line costs kTStatesPerFrame units of line_accum_.
    line_accum_ += tstates * kLinesPerFrame;
    while (line_accum_ >= kTStatesPerFrame) {
        line_accum_ -= kTStatesPerFrame;
        // The sprite flags are set as the raster crosses each active line, not
        // once a frame, because software polls them mid-frame.
        if (line_ < kActiveLines) scan_line_status(line_);
        if (++line_ >= kLinesPerFrame) line_ = 0;
    }

    if (!vblank_hit_ && frame_ts_ >= kVblankTStates) {
        vblank_hit_ = true;
        status_ |= kStatusFrame;
        ++frames_;
    }
    if (frame_ts_ >= kTStatesPerFrame) {
        frame_ts_ -= kTStatesPerFrame;
        vblank_hit_ = false;
    }
}

int Tms9129::line_sprites(int line, int out[kMaxSpritesPerLine], int& fifth, int& last) const {
    const std::uint16_t attr = sprite_attr_base();
    const bool big = (reg_[1] & 0x02) != 0;
    const bool mag = (reg_[1] & 0x01) != 0;
    const int size = (big ? 16 : 8) * (mag ? 2 : 1);

    int count = 0;
    fifth = -1;
    last = 0;
    for (int s = 0; s < 32; ++s) {
        const std::uint16_t a = std::uint16_t((attr + s * 4) & (kVramSize - 1));
        const std::uint8_t vy = vram_[a];
        // D0 in the Y byte ends the list: no sprite after it is processed.
        if (vy == 0xd0) break;
        last = s;
        int sy = vy + 1;
        if (vy > 224) sy = vy - 255;  // wraps in from above the screen
        if (line < sy || line >= sy + size) continue;
        if (count < kMaxSpritesPerLine) {
            out[count++] = s;
        } else {
            fifth = s;
            break;
        }
    }
    return count;
}

std::uint32_t Tms9129::sprite_row(int sprite, int line, int& x0, int& width) const {
    const std::uint16_t attr = sprite_attr_base();
    const std::uint16_t pat = sprite_pat_base();
    const bool big = (reg_[1] & 0x02) != 0;
    const bool mag = (reg_[1] & 0x01) != 0;
    const int size = (big ? 16 : 8) * (mag ? 2 : 1);

    const std::uint16_t a = std::uint16_t((attr + sprite * 4) & (kVramSize - 1));
    const std::uint8_t vy = vram_[a];
    int sy = vy + 1;
    if (vy > 224) sy = vy - 255;
    const int sx = vram_[std::uint16_t(a + 1)];
    const std::uint8_t name = vram_[std::uint16_t(a + 2)];
    const std::uint8_t colour = vram_[std::uint16_t(a + 3)];
    x0 = sx - ((colour & 0x80) ? 32 : 0);  // the early-clock bit shifts it left
    width = size;

    const std::uint16_t base = std::uint16_t(pat + (big ? (name & 0xfc) : name) * 8);
    const int row = mag ? ((line - sy) / 2) : (line - sy);

    std::uint32_t bits = 0;
    const int cols = big ? 16 : 8;
    for (int c = 0; c < cols; ++c) {
        const int quad = big ? ((c / 8) * 16) : 0;
        const std::uint8_t byte = vram_[std::uint16_t((base + quad + (row & 15)) & (kVramSize - 1))];
        if (byte & (0x80 >> (c & 7))) bits |= (1u << c);
    }
    // Magnification doubles each pattern column.
    if (mag) {
        std::uint32_t wide = 0;
        for (int c = 0; c < cols; ++c) {
            if (bits & (1u << c)) wide |= (3u << (c * 2));
        }
        return wide;
    }
    return bits;
}

// Fifth-sprite and coincidence, per line. Both are sticky until the status
// register is read. Coincidence counts a sprite's pattern bits whatever its
// colour -- a transparent sprite still collides.
void Tms9129::scan_line_status(int line) {
    if (!display_enabled() || mode() == kText) return;

    int idx[kMaxSpritesPerLine];
    int fifth = -1, last = 0;
    const int n = line_sprites(line, idx, fifth, last);

    if (fifth >= 0) {
        if (!(status_ & kStatusFifthSprite)) {
            status_ = static_cast<std::uint8_t>((status_ & ~kStatusSpriteMask) |
                                                kStatusFifthSprite | (fifth & kStatusSpriteMask));
        }
    } else if (!(status_ & kStatusFifthSprite)) {
        status_ = static_cast<std::uint8_t>((status_ & ~kStatusSpriteMask) |
                                            (last & kStatusSpriteMask));
    }

    if (status_ & kStatusCollision) return;  // already set this frame
    bool hit[kWidth] = {};
    for (int i = 0; i < n; ++i) {
        int x0 = 0, width = 0;
        const std::uint32_t bits = sprite_row(idx[i], line, x0, width);
        for (int c = 0; c < width; ++c) {
            if (!(bits & (1u << c))) continue;
            const int px = x0 + c;
            if (px < 0 || px >= kWidth) continue;
            if (hit[px]) {
                status_ |= kStatusCollision;
                return;
            }
            hit[px] = true;
        }
    }
}

Tms9129::Mode Tms9129::mode() const {
    const bool m3 = (reg_[0] & 0x02) != 0;
    const bool m2 = (reg_[1] & 0x08) != 0;
    const bool m1 = (reg_[1] & 0x10) != 0;
    if (!m1 && !m2 && !m3) return kGraphics1;
    if (!m1 && !m2 && m3) return kGraphics2;
    if (!m1 && m2 && !m3) return kMultiColour;
    if (m1 && !m2 && !m3) return kText;
    return kUnknownMode;
}

void Tms9129::render(std::uint32_t* out) const {
    const std::uint32_t backdrop = kPalette[reg_[7] & 0x0f];
    for (int i = 0; i < kWidth * kHeight; ++i) out[i] = backdrop;
    if (!display_enabled()) return;

    switch (mode()) {
        case kText: render_text(out); return;
        case kGraphics1: render_graphics1(out); break;
        case kGraphics2: render_graphics2(out); break;
        case kMultiColour: render_multicolour(out); break;
        default: return;
    }
    // Text mode has no sprites.
    render_sprites(out);
}

// 40 columns of 6x8 characters. Two colours from R7; the 240 active pixels sit
// inside a backdrop border.
void Tms9129::render_text(std::uint32_t* out) const {
    const std::uint32_t fg = kPalette[(reg_[7] >> 4) & 0x0f];
    const std::uint32_t bg = kPalette[reg_[7] & 0x0f];
    const std::uint16_t name = name_base();
    const std::uint16_t pat = pattern_base();

    for (int row = 0; row < 24; ++row) {
        for (int col = 0; col < 40; ++col) {
            const std::uint8_t ch = vram_[(name + row * 40 + col) & (kVramSize - 1)];
            for (int y = 0; y < 8; ++y) {
                const std::uint8_t bits = vram_[(pat + ch * 8 + y) & (kVramSize - 1)];
                std::uint32_t* line = out + (row * 8 + y) * kWidth + 8 + col * 6;
                for (int x = 0; x < 6; ++x) {
                    line[x] = (bits & (0x80 >> x)) ? fg : bg;
                }
            }
        }
    }
}

// 32 columns of 8x8 characters, one foreground/background pair per group of 8
// consecutive patterns.
void Tms9129::render_graphics1(std::uint32_t* out) const {
    const std::uint16_t name = name_base();
    const std::uint16_t pat = pattern_base();
    const std::uint16_t col = colour_base();
    const std::uint32_t backdrop = kPalette[reg_[7] & 0x0f];

    for (int row = 0; row < 24; ++row) {
        for (int c = 0; c < 32; ++c) {
            const std::uint8_t ch = vram_[(name + row * 32 + c) & (kVramSize - 1)];
            const std::uint8_t colour = vram_[(col + (ch >> 3)) & (kVramSize - 1)];
            const std::uint32_t fg = (colour >> 4) ? kPalette[colour >> 4] : backdrop;
            const std::uint32_t bg = (colour & 0x0f) ? kPalette[colour & 0x0f] : backdrop;
            for (int y = 0; y < 8; ++y) {
                const std::uint8_t bits = vram_[(pat + ch * 8 + y) & (kVramSize - 1)];
                std::uint32_t* line = out + (row * 8 + y) * kWidth + c * 8;
                for (int x = 0; x < 8; ++x) line[x] = (bits & (0x80 >> x)) ? fg : bg;
            }
        }
    }
}

// Three independent 256x64 banks, each with its own pattern and colour data.
void Tms9129::render_graphics2(std::uint32_t* out) const {
    const std::uint16_t name = name_base();
    // In Graphics II only the top bits of R4/R3 select the bank base.
    const std::uint16_t pat = std::uint16_t(reg_[4] & 0x04 ? 0x2000 : 0x0000);
    const std::uint16_t col = std::uint16_t(reg_[3] & 0x80 ? 0x2000 : 0x0000);
    const std::uint16_t pat_mask = std::uint16_t(((reg_[4] & 0x03) << 11) | 0x07ff);
    const std::uint16_t col_mask = std::uint16_t(((reg_[3] & 0x7f) << 6) | 0x003f);
    const std::uint32_t backdrop = kPalette[reg_[7] & 0x0f];

    for (int row = 0; row < 24; ++row) {
        const int bank = (row / 8) * 0x800;
        for (int c = 0; c < 32; ++c) {
            const std::uint8_t ch = vram_[(name + row * 32 + c) & (kVramSize - 1)];
            for (int y = 0; y < 8; ++y) {
                const int offset = bank + ch * 8 + y;
                const std::uint8_t bits = vram_[(pat + (offset & pat_mask)) & (kVramSize - 1)];
                const std::uint8_t colour = vram_[(col + (offset & col_mask)) & (kVramSize - 1)];
                const std::uint32_t fg = (colour >> 4) ? kPalette[colour >> 4] : backdrop;
                const std::uint32_t bg = (colour & 0x0f) ? kPalette[colour & 0x0f] : backdrop;
                std::uint32_t* line = out + (row * 8 + y) * kWidth + c * 8;
                for (int x = 0; x < 8; ++x) line[x] = (bits & (0x80 >> x)) ? fg : bg;
            }
        }
    }
}

// 64x48 blocks of 4x4 pixels.
void Tms9129::render_multicolour(std::uint32_t* out) const {
    const std::uint16_t name = name_base();
    const std::uint16_t pat = pattern_base();
    const std::uint32_t backdrop = kPalette[reg_[7] & 0x0f];

    for (int row = 0; row < 24; ++row) {
        for (int c = 0; c < 32; ++c) {
            const std::uint8_t ch = vram_[(name + row * 32 + c) & (kVramSize - 1)];
            for (int y = 0; y < 8; ++y) {
                const int seg = (row & 3) * 2 + (y >> 2);
                const std::uint8_t byte = vram_[(pat + ch * 8 + seg) & (kVramSize - 1)];
                const std::uint8_t left = std::uint8_t(byte >> 4);
                const std::uint8_t right = std::uint8_t(byte & 0x0f);
                std::uint32_t* line = out + (row * 8 + y) * kWidth + c * 8;
                for (int x = 0; x < 4; ++x) line[x] = left ? kPalette[left] : backdrop;
                for (int x = 4; x < 8; ++x) line[x] = right ? kPalette[right] : backdrop;
            }
        }
    }
}

// 32 sprites, highest priority first; 8x8 or 16x16, optionally magnified.
void Tms9129::render_sprites(std::uint32_t* out) const {
    const std::uint16_t attr = sprite_attr_base();

    for (int line = 0; line < kHeight; ++line) {
        int idx[kMaxSpritesPerLine];
        int fifth = -1, last = 0;
        const int n = line_sprites(line, idx, fifth, last);
        // Draw back to front so the lowest-numbered sprite ends up on top.
        for (int i = n - 1; i >= 0; --i) {
            const std::uint16_t a = std::uint16_t((attr + idx[i] * 4) & (kVramSize - 1));
            const int ci = vram_[std::uint16_t(a + 3)] & 0x0f;
            if (ci == 0) continue;  // transparent: still collides, but not drawn

            int x0 = 0, width = 0;
            const std::uint32_t bits = sprite_row(idx[i], line, x0, width);
            for (int c = 0; c < width; ++c) {
                if (!(bits & (1u << c))) continue;
                const int px = x0 + c;
                if (px < 0 || px >= kWidth) continue;
                out[line * kWidth + px] = kPalette[ci];
            }
        }
    }
}

}  // namespace ein
