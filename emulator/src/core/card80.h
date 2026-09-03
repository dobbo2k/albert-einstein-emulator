#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace ein {

// The 80-column card: 2K of character RAM, a 6845 CRTC, and a status input.
// Ports from MOSEQU.GEN:79-84 --
//
//   40-47  video RAM      48  6845 address     49  6845 data     4C  input
//
// **How the RAM is addressed** is the interesting part, and it is settled by
// the output routine at Mos12.zsm:L0C13. MOS computes a linear offset, hands
// the high and low bytes to 6845 registers 0E/0F, and then writes the
// character with `OUT (C),A` where C is `40 + (addr >> 8 & 7)` and B is the
// low byte. `OUT (C),A` puts B on A8-A15, so the full 16-bit port carries the
// offset in its top half and the page in its bottom: eight pages of 256 bytes,
// 2K in all, which is exactly 80 x 24 with room to spare.
//
// **Detection** (Mos12.zsm:L0E31) reads port 40, writes back the complement
// and reads again; equal means a card is there. Plain read/write RAM satisfies
// it, and an empty bus reading FF does not -- so a machine without the card
// reports none, which is what a base TC-01 should do.
//
// What is *not* modelled is the picture. The card carries its own character
// generator ROM and that ROM is not in this archive, so there is nothing to
// draw glyphs from. Everything the card is asked to display is recorded as
// character codes and can be read back as text, which is what the tests and
// the debugger use; inventing a font to put on screen would be making up
// hardware.
class Card80 {
public:
    static constexpr int kRamSize = 2048;
    static constexpr int kColumns = 80;
    static constexpr int kRows = 24;
    static constexpr int kRegisters = 18;

    // Off by default: the card was an add-on, and a base machine has to report
    // that it has none.
    bool present = false;

    // Port 4C bit 0 is the card's access window: L0BEF (Mos12.zsm) waits for it
    // to go high and then low again before every character it writes, which is
    // the anti-snow handshake real CRTC cards need. It has to move, or MOS
    // spins there forever -- so the card is clocked.
    static constexpr int kLineTStates = 256;   // ~64 us, one scan line at 4 MHz
    static constexpr int kBlankTStates = 64;   // the part of it that is blanking

    void reset();
    void tick(int tstates);

    // port is the full 16-bit value, because the RAM offset rides on A8-A15.
    std::uint8_t read(std::uint16_t port) const;
    void write(std::uint16_t port, std::uint8_t value);
    bool decodes(std::uint8_t low) const;

    std::uint8_t ram(int offset) const { return ram_[offset & (kRamSize - 1)]; }
    std::uint8_t reg(int i) const { return reg_[i % kRegisters]; }
    std::uint8_t selected_reg() const { return selected_; }
    std::uint64_t reg_writes() const { return reg_writes_; }
    std::uint64_t ram_writes() const { return ram_writes_; }

    // Where the 6845 says the display starts (registers 0C/0D).
    int start_address() const { return ((reg_[12] & 0x3f) << 8) | reg_[13]; }
    // And where the cursor is (0E/0F), which is also the address MOS sets
    // before writing a character.
    int cursor_address() const { return ((reg_[14] & 0x3f) << 8) | reg_[15]; }

    // The card's RAM read back as 24 lines of 80 characters.
    std::string line(int row) const;

private:
    std::array<std::uint8_t, kRamSize> ram_{};
    std::uint8_t reg_[kRegisters]{};
    std::uint8_t selected_ = 0;
    std::uint64_t reg_writes_ = 0;
    std::uint64_t ram_writes_ = 0;
    int phase_ = 0;
};

}  // namespace ein
