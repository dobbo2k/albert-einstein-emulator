#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "disasm.h"

namespace ein {

// Hohne's commented sources, wired to addresses.
//
// The problem is that a .zsm file says what the instructions are but not where
// they live, and assembling it properly would mean writing an assembler. It
// does not need one. Two things make a much cheaper approach exact:
//
//   * Most of Hohne's labels are of the form Lxxxx, where xxxx *is* the
//     address -- L0763, L0FC2, LFB14. Every one of those is an anchor, and so
//     is every ORG.
//   * Between anchors the source is a faithful one-line-per-instruction
//     disassembly, so walking it in lockstep with the real instruction lengths
//     taken from the ROM assigns every line its address.
//
// Data lines (DEFB/DEFW/DEFM/DC/DEFS) take their length from the source
// instead, since the bytes there are not instructions.
//
// The walk checks itself: on reaching the next anchor, the address it has
// arrived at either matches the label or it does not. Mismatches re-anchor and
// are counted, so the quality of the mapping is a number the tests assert on
// rather than something to be hoped for.
class Symbols {
public:
    struct Line {
        std::uint16_t addr = 0;
        std::string label;
        std::string text;     // mnemonic and operands, as written
        std::string comment;  // without the ';'
        std::string file;
        int line_no = 0;
        bool is_data = false;
    };

    // Reads EQU definitions only -- for Einstein.zsm and MOSEQU.GEN, which are
    // equates with no code.
    void load_equates(const std::filesystem::path& path);

    // Reads a source and maps it onto addresses, using read for instruction
    // lengths. read must see the same bytes the source was disassembled from.
    void load_source(const std::filesystem::path& path, const ReadByte& read);

    const Line* line_at(std::uint16_t addr) const;
    const std::string* label_at(std::uint16_t addr) const;
    // The address of a named symbol, or -1.
    int lookup(const std::string& name) const;

    std::size_t line_count() const { return lines_.size(); }
    std::size_t equate_count() const { return equates_.size(); }
    int anchors() const { return anchors_; }
    int anchors_matched() const { return anchors_matched_; }
    int equates_used() const { return equates_used_; }

    // Every label, for a symbol list in the debugger.
    const std::map<std::uint16_t, std::string>& labels() const { return labels_; }

private:
    std::map<std::uint16_t, Line> lines_;
    std::map<std::uint16_t, std::string> labels_;
    // Code labels are kept apart from EQU definitions on purpose: MOSEQU.GEN
    // defines ZZTIME, ZMOUT and friends as *MCAL numbers*, and those must not
    // shadow the addresses the same names have as labels in the source.
    std::unordered_map<std::string, int> label_addr_;
    std::unordered_map<std::string, int> equates_;
    int anchors_ = 0;
    int anchors_matched_ = 0;
    int equates_used_ = 0;

    bool parse_number(const std::string& tok, int& out) const;
};

}  // namespace ein
