#include "keyboard.h"

#include <cstring>

namespace ein {
namespace {

// Rows 1-7 are the ROM table at L10C5 transcribed byte for byte, in the ROM's
// own order: eight normal codes, eight shifted, eight control, per matrix row
// 02..80. test_core checks every one of these 168 bytes against the ROM image,
// so this stays honest.
//
// Row 0 (matrix row 01) has no table entry -- ZKSCAN decodes it in code
// (Mos12.zsm:1441-1465), column by column: 0 BREAK, 1 unused, 2 F0 (80H),
// 3 F7 (87H), 4 ALPHA (toggles caps, produces no code), 5 ENTER (0DH),
// 6 SPACE (20H), 7 ESC (1BH). The same codes are repeated across all three
// planes because the firmware ignores the modifiers for them.
constexpr std::uint8_t kCodes[8][3][8] = {
    // row 01 -- decoded inline by ZKSCAN, not from the table
    {{0x00, 0x00, 0x80, 0x87, 0x00, 0x0d, 0x20, 0x1b},
     {0x00, 0x00, 0x80, 0x87, 0x00, 0x0d, 0x20, 0x1b},
     {0x00, 0x00, 0x80, 0x87, 0x00, 0x0d, 0x20, 0x1b}},
    // row 02 -- i o p [ _ LF | 0
    {{0x69, 0x6f, 0x70, 0x5b, 0x5f, 0x0a, 0x7c, 0x30},
     {0x49, 0x4f, 0x50, 0x7b, 0x60, 0x0b, 0x5c, 0x40},
     {0x09, 0x0f, 0x10, 0x1b, 0x1f, 0x0a, 0x1c, 0x30}},
    // row 04 -- k l ; : ] EOT 9 F5
    {{0x6b, 0x6c, 0x3b, 0x3a, 0x5d, 0x04, 0x39, 0x85},
     {0x4b, 0x4c, 0x2b, 0x2a, 0x7d, 0x08, 0x29, 0x85},
     {0x0b, 0x0c, 0x3b, 0x3a, 0x1d, 0x04, 0x39, 0x85}},
    // row 08 -- , . / 8 EM = ^ F4
    {{0x2c, 0x2e, 0x2f, 0x38, 0x19, 0x3d, 0x5e, 0x84},
     {0x3c, 0x3e, 0x3f, 0x28, 0x1a, 0x2d, 0x7e, 0x84},
     {0x2c, 0x2e, 0x2f, 0x38, 0x06, 0x3d, 0x1e, 0x84}},
    // row 10 -- 7 6 5 4 3 2 1 F3
    {{0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x83},
     {0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x83},
     {0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x83}},
    // row 20 -- u y t r e w q F2
    {{0x75, 0x79, 0x74, 0x72, 0x65, 0x77, 0x71, 0x82},
     {0x55, 0x59, 0x54, 0x52, 0x45, 0x57, 0x51, 0x82},
     {0x15, 0x19, 0x14, 0x12, 0x05, 0x17, 0x11, 0x82}},
    // row 40 -- j h g f d s a F1
    {{0x6a, 0x68, 0x67, 0x66, 0x64, 0x73, 0x61, 0x81},
     {0x4a, 0x48, 0x47, 0x46, 0x44, 0x53, 0x41, 0x81},
     {0x0a, 0x08, 0x07, 0x06, 0x04, 0x13, 0x01, 0x81}},
    // row 80 -- m n b v c x z F6
    {{0x6d, 0x6e, 0x62, 0x76, 0x63, 0x78, 0x7a, 0x86},
     {0x4d, 0x4e, 0x42, 0x56, 0x43, 0x58, 0x5a, 0x86},
     {0x0d, 0x0e, 0x02, 0x16, 0x03, 0x18, 0x1a, 0x86}},
};

}  // namespace

std::uint8_t Keyboard::code(int row, int col, Plane p) {
    if (row < 0 || row > 7 || col < 0 || col > 7) return 0;
    return kCodes[row][p][col];
}

void Keyboard::reset() {
    std::memset(rows_, 0, sizeof rows_);
    shift_ = control_ = graph_ = false;
    port20_low_ = 0;
}

void Keyboard::release_all() {
    reset();
}

void Keyboard::set_key(int row, int col, bool down) {
    if (row < 0 || row > 7 || col < 0 || col > 7) return;
    const auto bit = static_cast<std::uint8_t>(1u << col);
    if (down) rows_[row] |= bit;
    else rows_[row] = static_cast<std::uint8_t>(rows_[row] & ~bit);
}

void Keyboard::set_modifier(Modifier m, bool down) {
    switch (m) {
        case kShift: shift_ = down; break;
        case kControl: control_ = down; break;
        case kGraph: graph_ = down; break;
    }
}

std::uint8_t Keyboard::columns(std::uint8_t row_mask) const {
    std::uint8_t out = 0;
    for (int r = 0; r < 8; ++r) {
        if (row_mask & (1u << r)) out |= rows_[r];
    }
    return out;
}

std::uint8_t Keyboard::modifier_port() const {
    // Active low: a set bit means "not pressed".
    std::uint8_t v = 0xff;
    if (shift_) v = static_cast<std::uint8_t>(v & ~0x80);
    if (control_) v = static_cast<std::uint8_t>(v & ~0x40);
    if (graph_) v = static_cast<std::uint8_t>(v & ~0x20);
    v = static_cast<std::uint8_t>(v & ~port20_low_);
    return v;
}

bool Keyboard::any_pressed() const {
    for (std::uint8_t r : rows_) {
        if (r) return true;
    }
    return false;
}

bool Keyboard::map_ascii(char ch, int& row, int& col, bool& shift) {
    const auto want = static_cast<std::uint8_t>(ch);
    if (want == 0 || want >= 0x80) return false;
    for (int p = 0; p < 2; ++p) {
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                if (kCodes[r][p][c] != want) continue;
                row = r;
                col = c;
                shift = (p == kShifted);
                return true;
            }
        }
    }
    return false;
}

}  // namespace ein
