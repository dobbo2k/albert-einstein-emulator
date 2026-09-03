#pragma once

#include <array>
#include <cstdint>

namespace ein {

// TMS9129 video display processor (TMS9918A family, PAL).
//
// Ports on the Einstein: 08 = VRAM data, 09 = control/status
// (Einstein.zsm:15-18).
//
// Control port protocol, confirmed against the MOS routines at 0C8D/0C98:
// two writes to port 09. The first latches the low byte; the second selects
// what happens, from its top two bits:
//
//   1x -> write register (b2 & 7) with the latched byte
//   01 -> set the VRAM address for writing
//   00 -> set the VRAM address for reading (and prefetch)
class Tms9129 {
public:
    static constexpr int kVramSize = 0x4000;  // 16K
    static constexpr int kWidth = 256;
    static constexpr int kHeight = 192;

    // PAL: 313 lines at 50.16 Hz. At the Einstein's 4 MHz that is 79749
    // T-states per frame, 254.8 per line.
    static constexpr int kLinesPerFrame = 313;
    static constexpr int kActiveLines = 192;
    static constexpr int kTStatesPerFrame = 79749;
    // T-state offset at which the active display ends and the frame flag sets.
    static constexpr int kVblankTStates = kActiveLines * kTStatesPerFrame / kLinesPerFrame;

    enum Mode { kGraphics1, kGraphics2, kMultiColour, kText, kUnknownMode };

    // Status register bits. Reading the port clears the top three; the low five
    // hold a sprite number (the fifth sprite on a line, or the last one the
    // scan looked at).
    enum Status : std::uint8_t {
        kStatusFrame = 0x80,      // F: end of active display
        kStatusFifthSprite = 0x40,  // 5S
        kStatusCollision = 0x20,  // C: two sprites' patterns overlapped
        kStatusSpriteMask = 0x1f,
    };

    // No more than four sprites are displayed on any one line; the fifth and
    // beyond are dropped, which is what produces the flicker games rely on.
    static constexpr int kMaxSpritesPerLine = 4;

    void reset();

    std::uint8_t read_data();
    void write_data(std::uint8_t v);
    std::uint8_t read_status();
    void write_ctrl(std::uint8_t v);

    void tick(int tstates);

    // The scanline the raster is on, 0-312.
    int line() const { return line_; }

    // The TMS9918 family needs about 8 us between consecutive VRAM accesses
    // while the display is active -- 32 T-states at 4 MHz -- or the access is
    // lost to the display's own fetch. MOS respects it: the character-set
    // upload at Mos12.zsm:2370 pads each OUTI with PUSH AF / POP AF.
    //
    // Accesses that break it are counted rather than dropped. Dropping them is
    // what the hardware does, but a program that relied on the emulator being
    // forgiving would then fail in a way that looks like a bug in the program,
    // and nothing in this archive violates the constraint anyway. The counter
    // is there so that if a title ever misbehaves, the debugger can say whether
    // this is why.
    static constexpr int kMinAccessTStates = 32;
    std::uint64_t fast_accesses() const { return fast_accesses_; }
    int min_access_gap() const { return min_access_gap_; }

    // The VDP's INT output. Not yet wired to the CPU -- there is no VDP entry
    // in the MOS vector table (Einstein.zsm:40-49), so how it reaches the Z80
    // is an open question for M4.
    bool irq() const { return (status_ & 0x80) && (reg_[1] & 0x20); }

    Mode mode() const;
    bool display_enabled() const { return (reg_[1] & 0x40) != 0; }
    std::uint16_t name_base() const { return std::uint16_t((reg_[2] & 0x0f) << 10); }
    std::uint16_t pattern_base() const { return std::uint16_t((reg_[4] & 0x07) << 11); }
    std::uint16_t colour_base() const { return std::uint16_t(reg_[3] << 6); }
    std::uint16_t sprite_attr_base() const { return std::uint16_t((reg_[5] & 0x7f) << 7); }
    std::uint16_t sprite_pat_base() const { return std::uint16_t((reg_[6] & 0x07) << 11); }

    std::uint8_t reg(int i) const { return reg_[i & 7]; }
    std::uint8_t vram(std::uint16_t a) const { return vram_[a & (kVramSize - 1)]; }
    std::uint16_t address() const { return addr_; }
    std::uint64_t frames() const { return frames_; }
    std::uint64_t reg_writes() const { return reg_writes_; }
    std::uint64_t vram_writes() const { return vram_writes_; }

    // Renders the active display as 0xAARRGGBB, kWidth * kHeight pixels.
    void render(std::uint32_t* out) const;

    static std::uint32_t palette(int index);

private:
    std::array<std::uint8_t, kVramSize> vram_{};
    std::array<std::uint8_t, 8> reg_{};
    std::uint8_t status_ = 0;
    int line_ = 0;
    int line_accum_ = 0;
    std::uint64_t ts_ = 0;
    std::uint64_t last_access_ts_ = 0;
    std::uint64_t fast_accesses_ = 0;
    int min_access_gap_ = 1 << 30;
    void note_access();
    std::uint8_t latch_value_ = 0;
    bool latched_ = false;
    std::uint8_t read_buffer_ = 0;
    std::uint16_t addr_ = 0;

    int frame_ts_ = 0;
    bool vblank_hit_ = false;
    std::uint64_t frames_ = 0;
    std::uint64_t reg_writes_ = 0;
    std::uint64_t vram_writes_ = 0;

    void bump_addr() { addr_ = std::uint16_t((addr_ + 1) & (kVramSize - 1)); }

    void render_text(std::uint32_t* out) const;
    void render_graphics1(std::uint32_t* out) const;
    void render_graphics2(std::uint32_t* out) const;
    void render_multicolour(std::uint32_t* out) const;
    void render_sprites(std::uint32_t* out) const;

    // Sprites visible on a line, in sprite order. Returns how many were found
    // for display (at most four); fifth is the number of the first one over
    // that limit, or -1, and last is the highest number the scan examined.
    int line_sprites(int line, int out[kMaxSpritesPerLine], int& fifth, int& last) const;
    // The eight or sixteen pattern bits of one sprite on one line, plus the
    // screen x its leftmost pixel lands on.
    std::uint32_t sprite_row(int sprite, int line, int& x0, int& width) const;
    void scan_line_status(int line);
};

}  // namespace ein
