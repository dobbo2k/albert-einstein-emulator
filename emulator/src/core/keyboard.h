#pragma once

#include <cstdint>

namespace ein {

// Einstein keyboard: an 8x8 matrix reached through the AY-3-8910's I/O ports,
// plus three modifier keys on port 20.
//
// From the MOS scan routines at 0F99/0FA5:
//   OUT (02),0E ; OUT (03),~row   select rows via PSG register 14 (port A)
//   OUT (02),0F ; IN  (02)        read columns from PSG register 15 (port B)
// Both directions are active low, which is why MOS complements each way.
//
// Modifiers come from reading port 20 (KBDMSK), also active low: ZKSCAN does
// IN A,(20) / CPL / AND 0E0H / RRCA at Mos12.zsm:1349, which lands shift in
// KFLAG bit 6, control in bit 5 and graph in bit 4.
class Keyboard {
public:
    enum Modifier { kShift, kControl, kGraph };

    // The three planes of the firmware's key table.
    enum Plane { kPlain = 0, kShifted = 1, kControlled = 2 };

    // L10C5 (Mos12.zsm:2827) -- "KBD CHARS (NORM,SHIFT,CONT)". 24 bytes per
    // matrix row, eight per plane, for the seven rows 02..80 in that order.
    // Row 01 is absent: ZKSCAN decodes it inline (Mos12.zsm:1441-1465) because
    // its keys are BREAK, ALPHA and the function keys rather than characters.
    static constexpr std::uint16_t kRomTableAddr = 0x10c5;
    static constexpr int kRomTableRows = 7;
    static constexpr int kRomTableBytes = kRomTableRows * 3 * 8;

    void reset();

    // row and col are 0-7; row r is the mask bit 1<<r, so row 0 is mask 01.
    void set_key(int row, int col, bool down);
    void set_modifier(Modifier m, bool down);
    void release_all();

    // Active-high column bits for every row selected in row_mask.
    std::uint8_t columns(std::uint8_t row_mask) const;

    // What IN A,(20) returns: modifiers active low in bits 5-7.
    std::uint8_t modifier_port() const;

    // Extra active-low lines on that same port. The joystick fire buttons are
    // read here: waiting at Oh Mummy's "PRESS 'C'/'FIRE' TO CONTINUE" the game
    // polls ports 02 and 20 and nothing else, so fire is not on the ADC and
    // not a matrix position. Which bits of port 20 it uses is not documented
    // anywhere in this archive, so the mask is set from outside rather than
    // being guessed at in here.
    void set_port20_low(std::uint8_t mask) { port20_low_ = mask; }
    std::uint8_t port20_low() const { return port20_low_; }

    bool any_pressed() const;

    // The code the firmware produces for the key at (row, col) in one plane.
    // 0 means the position carries no code in that plane.
    static std::uint8_t code(int row, int col, Plane p);

    // Maps a printable character to a matrix position, reporting whether shift
    // is required. Returns false if the character is not on the keyboard.
    static bool map_ascii(char ch, int& row, int& col, bool& shift);

private:
    std::uint8_t rows_[8]{};  // active-high column bits
    std::uint8_t port20_low_ = 0;
    bool shift_ = false;
    bool control_ = false;
    bool graph_ = false;
};

}  // namespace ein
