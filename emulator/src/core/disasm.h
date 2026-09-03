#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace ein {

// Z80 disassembler. Covers the whole instruction set including the ED/CB/DD/FD
// prefixes and the undocumented IXH/IXL and SLL forms, because the ROM and the
// software that runs on it use them and a debugger that says "???" at the one
// instruction you are chasing is worse than useless.
//
// Reads through a callback so it can be pointed at a running Machine (which
// bank is paged in matters) or at a flat ROM image.
struct Instruction {
    std::uint16_t addr = 0;
    int length = 1;
    std::string text;
    // The absolute address this instruction refers to, if it names one -- the
    // target of a jump, call or absolute load. -1 when it does not.
    int target = -1;
};

using ReadByte = std::function<std::uint8_t(std::uint16_t)>;

Instruction disassemble(const ReadByte& read, std::uint16_t addr);

// Just the length, for walking without formatting.
int instruction_length(const ReadByte& read, std::uint16_t addr);

}  // namespace ein
