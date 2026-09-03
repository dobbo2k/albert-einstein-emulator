#include "card80.h"

namespace ein {

void Card80::reset() {
    ram_.fill(0x20);  // spaces, so the text reads as blank rather than as NULs
    for (auto& r : reg_) r = 0;
    selected_ = 0;
    reg_writes_ = 0;
    ram_writes_ = 0;
    phase_ = 0;
}

void Card80::tick(int tstates) {
    if (!present) return;
    phase_ += tstates;
    while (phase_ >= kLineTStates) phase_ -= kLineTStates;
}

bool Card80::decodes(std::uint8_t low) const {
    if (!present) return false;
    return (low & 0xf8) == 0x40 || low == 0x48 || low == 0x49 || low == 0x4c;
}

std::uint8_t Card80::read(std::uint16_t port) const {
    const std::uint8_t low = static_cast<std::uint8_t>(port & 0xff);
    if ((low & 0xf8) == 0x40) {
        const int addr = ((low & 0x07) << 8) | ((port >> 8) & 0xff);
        return ram_[addr & (kRamSize - 1)];
    }
    if (low == 0x49) return reg_[selected_ % kRegisters];
    if (low == 0x4c) {
        // Bit 0 is the access window MOS waits on before every character.
        // Bit 1 must be set or Z80INIT reports a failure (Mos12.zsm:Z80INIT);
        // bit 2 clear selects the 50 Hz parameter table, which is the PAL
        // machine this emulator is.
        std::uint8_t v = 0x02;
        if (phase_ < kBlankTStates) v |= 0x01;
        return v;
    }
    return 0xff;
}

void Card80::write(std::uint16_t port, std::uint8_t value) {
    const std::uint8_t low = static_cast<std::uint8_t>(port & 0xff);
    if ((low & 0xf8) == 0x40) {
        const int addr = ((low & 0x07) << 8) | ((port >> 8) & 0xff);
        ram_[addr & (kRamSize - 1)] = value;
        ++ram_writes_;
        return;
    }
    if (low == 0x48) {
        selected_ = static_cast<std::uint8_t>(value & 0x1f);
        return;
    }
    if (low == 0x49) {
        reg_[selected_ % kRegisters] = value;
        ++reg_writes_;
        return;
    }
}

std::string Card80::line(int row) const {
    std::string out;
    if (row < 0 || row >= kRows) return out;
    const int base = start_address() + row * kColumns;
    for (int i = 0; i < kColumns; ++i) {
        const std::uint8_t c = ram_[(base + i) & (kRamSize - 1)];
        out.push_back((c >= 0x20 && c < 0x7f) ? static_cast<char>(c) : ' ');
    }
    while (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

}  // namespace ein
