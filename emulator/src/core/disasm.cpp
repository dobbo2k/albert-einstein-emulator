#include "disasm.h"

#include <cstdio>

namespace ein {
namespace {

const char* const kR[8] = {"B", "C", "D", "E", "H", "L", "(HL)", "A"};
const char* const kRp[4] = {"BC", "DE", "HL", "SP"};
const char* const kRp2[4] = {"BC", "DE", "HL", "AF"};
const char* const kCc[8] = {"NZ", "Z", "NC", "C", "PO", "PE", "P", "M"};
const char* const kAlu[8] = {"ADD A,", "ADC A,", "SUB ", "SBC A,", "AND ", "XOR ", "OR ", "CP "};
const char* const kRot[8] = {"RLC", "RRC", "RL", "RR", "SLA", "SRA", "SLL", "SRL"};
const char* const kBlock[4][4] = {
    {"LDI", "CPI", "INI", "OUTI"},
    {"LDD", "CPD", "IND", "OUTD"},
    {"LDIR", "CPIR", "INIR", "OTIR"},
    {"LDDR", "CPDR", "INDR", "OTDR"},
};

std::string hex8(std::uint8_t v) {
    char b[8];
    std::snprintf(b, sizeof b, "%02XH", v);
    // A leading letter would read as a label, so assemblers want a 0 in front.
    return (v >= 0xa0) ? ("0" + std::string(b)) : std::string(b);
}

std::string hex16(std::uint16_t v) {
    char b[12];
    std::snprintf(b, sizeof b, "%04XH", v);
    return (v >= 0xa000) ? ("0" + std::string(b)) : std::string(b);
}

// State for one decode.
struct Decoder {
    const ReadByte& read;
    std::uint16_t base;
    int len = 0;
    int target = -1;

    std::uint8_t fetch() {
        const std::uint8_t v = read(static_cast<std::uint16_t>(base + len));
        ++len;
        return v;
    }
    std::uint16_t fetch16() {
        const std::uint8_t lo = fetch();
        const std::uint8_t hi = fetch();
        return static_cast<std::uint16_t>(lo | (hi << 8));
    }
    std::string abs16() {
        const std::uint16_t a = fetch16();
        target = a;
        return hex16(a);
    }
    std::string rel() {
        const auto d = static_cast<std::int8_t>(fetch());
        const auto a = static_cast<std::uint16_t>(base + len + d);
        target = a;
        return hex16(a);
    }
};

// The index-register substitutions: HL becomes IX/IY, H and L become the
// halves, and (HL) becomes (IX+d) -- which also swallows a displacement byte.
struct Index {
    const char* rp = "HL";
    const char* h = "H";
    const char* l = "L";
    bool active = false;
};

std::string idx_mem(Decoder& d, const Index& ix) {
    const auto disp = static_cast<std::int8_t>(d.fetch());
    char b[24];
    std::snprintf(b, sizeof b, "(%s%c%d)", ix.rp, disp < 0 ? '-' : '+',
                  disp < 0 ? -disp : disp);
    return b;
}

// r[] with the index substitutions applied. mem_ok is false for the operand of
// a DD CB instruction, where the displacement has already been read.
std::string reg(Decoder& d, const Index& ix, int i) {
    if (!ix.active) return kR[i];
    switch (i) {
        case 4: return ix.h;
        case 5: return ix.l;
        case 6: return idx_mem(d, ix);
        default: return kR[i];
    }
}

std::string decode_cb(Decoder& d, const Index& ix) {
    // For DD CB / FD CB the displacement comes before the opcode, and the
    // operand is always the indexed memory location -- with the undocumented
    // forms also copying the result into a register.
    std::string mem;
    if (ix.active) mem = idx_mem(d, ix);
    const std::uint8_t op = d.fetch();
    const int x = op >> 6, y = (op >> 3) & 7, z = op & 7;
    const std::string operand = ix.active ? mem : std::string(kR[z]);
    char buf[64];
    switch (x) {
        case 0:
            if (ix.active && z != 6) {
                std::snprintf(buf, sizeof buf, "%-5s%s,%s", kRot[y], operand.c_str(), kR[z]);
            } else {
                std::snprintf(buf, sizeof buf, "%-5s%s", kRot[y], operand.c_str());
            }
            return buf;
        case 1:
            std::snprintf(buf, sizeof buf, "BIT  %d,%s", y, operand.c_str());
            return buf;
        default: {
            const char* name = (x == 2) ? "RES" : "SET";
            if (ix.active && z != 6) {
                std::snprintf(buf, sizeof buf, "%-5s%d,%s,%s", name, y, operand.c_str(), kR[z]);
            } else {
                std::snprintf(buf, sizeof buf, "%-5s%d,%s", name, y, operand.c_str());
            }
            return buf;
        }
    }
}

std::string decode_ed(Decoder& d) {
    const std::uint8_t op = d.fetch();
    const int x = op >> 6, y = (op >> 3) & 7, z = op & 7;
    const int p = y >> 1, q = y & 1;
    char buf[64];

    if (x == 1) {
        switch (z) {
            case 0:
                if (y == 6) return "IN   (C)";
                std::snprintf(buf, sizeof buf, "IN   %s,(C)", kR[y]);
                return buf;
            case 1:
                if (y == 6) return "OUT  (C),0";
                std::snprintf(buf, sizeof buf, "OUT  (C),%s", kR[y]);
                return buf;
            case 2:
                std::snprintf(buf, sizeof buf, "%-5sHL,%s", q ? "ADC" : "SBC", kRp[p]);
                return buf;
            case 3: {
                const std::string a = d.abs16();
                if (q) std::snprintf(buf, sizeof buf, "LD   %s,(%s)", kRp[p], a.c_str());
                else std::snprintf(buf, sizeof buf, "LD   (%s),%s", a.c_str(), kRp[p]);
                return buf;
            }
            case 4: return "NEG";
            case 5: return (y == 1) ? "RETI" : "RETN";
            case 6: {
                static const int kIm[8] = {0, 0, 1, 2, 0, 0, 1, 2};
                std::snprintf(buf, sizeof buf, "IM   %d", kIm[y]);
                return buf;
            }
            default:
                switch (y) {
                    case 0: return "LD   I,A";
                    case 1: return "LD   R,A";
                    case 2: return "LD   A,I";
                    case 3: return "LD   A,R";
                    case 4: return "RRD";
                    case 5: return "RLD";
                    default: return "NOP";
                }
        }
    }
    if (x == 2 && z <= 3 && y >= 4) return kBlock[y - 4][z];
    return "NOP";  // everything else on ED is an undocumented two-byte NOP
}

std::string decode_main(Decoder& d, const Index& ix_in) {
    Index ix = ix_in;
    std::uint8_t op = d.fetch();

    // Chained DD/FD prefixes: only the last one counts.
    while (op == 0xdd || op == 0xfd) {
        ix.active = true;
        ix.rp = (op == 0xdd) ? "IX" : "IY";
        ix.h = (op == 0xdd) ? "IXH" : "IYH";
        ix.l = (op == 0xdd) ? "IXL" : "IYL";
        op = d.fetch();
    }
    if (op == 0xcb) return decode_cb(d, ix);
    if (op == 0xed) return decode_ed(d);

    const int x = op >> 6, y = (op >> 3) & 7, z = op & 7;
    const int p = y >> 1, q = y & 1;
    const char* hl = ix.active ? ix.rp : "HL";
    char buf[80];

    switch (x) {
        case 0:
            switch (z) {
                case 0:
                    switch (y) {
                        case 0: return "NOP";
                        case 1: return "EX   AF,AF'";
                        case 2: std::snprintf(buf, sizeof buf, "DJNZ %s", d.rel().c_str()); return buf;
                        case 3: std::snprintf(buf, sizeof buf, "JR   %s", d.rel().c_str()); return buf;
                        default:
                            std::snprintf(buf, sizeof buf, "JR   %s,%s", kCc[y - 4],
                                          d.rel().c_str());
                            return buf;
                    }
                case 1:
                    if (q == 0) {
                        const char* r = (p == 2) ? hl : kRp[p];
                        const std::uint16_t n = d.fetch16();
                        std::snprintf(buf, sizeof buf, "LD   %s,%s", r, hex16(n).c_str());
                        return buf;
                    }
                    std::snprintf(buf, sizeof buf, "ADD  %s,%s", hl, (p == 2) ? hl : kRp[p]);
                    return buf;
                case 2:
                    if (q == 0) {
                        switch (p) {
                            case 0: return "LD   (BC),A";
                            case 1: return "LD   (DE),A";
                            case 2:
                                std::snprintf(buf, sizeof buf, "LD   (%s),%s", d.abs16().c_str(), hl);
                                return buf;
                            default:
                                std::snprintf(buf, sizeof buf, "LD   (%s),A", d.abs16().c_str());
                                return buf;
                        }
                    }
                    switch (p) {
                        case 0: return "LD   A,(BC)";
                        case 1: return "LD   A,(DE)";
                        case 2:
                            std::snprintf(buf, sizeof buf, "LD   %s,(%s)", hl, d.abs16().c_str());
                            return buf;
                        default:
                            std::snprintf(buf, sizeof buf, "LD   A,(%s)", d.abs16().c_str());
                            return buf;
                    }
                case 3:
                    std::snprintf(buf, sizeof buf, "%-5s%s", q ? "DEC" : "INC",
                                  (p == 2) ? hl : kRp[p]);
                    return buf;
                case 4: {
                    const std::string r = reg(d, ix, y);
                    std::snprintf(buf, sizeof buf, "INC  %s", r.c_str());
                    return buf;
                }
                case 5: {
                    const std::string r = reg(d, ix, y);
                    std::snprintf(buf, sizeof buf, "DEC  %s", r.c_str());
                    return buf;
                }
                case 6: {
                    // The displacement comes before the immediate.
                    const std::string r = reg(d, ix, y);
                    const std::uint8_t n = d.fetch();
                    std::snprintf(buf, sizeof buf, "LD   %s,%s", r.c_str(), hex8(n).c_str());
                    return buf;
                }
                default: {
                    static const char* const kMisc[8] = {"RLCA", "RRCA", "RLA", "RRA",
                                                         "DAA",  "CPL",  "SCF", "CCF"};
                    return kMisc[y];
                }
            }
        case 1: {
            if (y == 6 && z == 6) return "HALT";
            // Only one side gets the index substitution when the other is (HL).
            std::string dst, src;
            if (ix.active && (y == 6 || z == 6)) {
                if (y == 6) {
                    dst = reg(d, ix, 6);
                    src = kR[z];
                } else {
                    dst = kR[y];
                    src = reg(d, ix, 6);
                }
            } else {
                dst = reg(d, ix, y);
                src = reg(d, ix, z);
            }
            std::snprintf(buf, sizeof buf, "LD   %s,%s", dst.c_str(), src.c_str());
            return buf;
        }
        case 2: {
            const std::string r = reg(d, ix, z);
            std::snprintf(buf, sizeof buf, "%s%s", kAlu[y], r.c_str());
            return buf;
        }
        default:
            switch (z) {
                case 0:
                    std::snprintf(buf, sizeof buf, "RET  %s", kCc[y]);
                    return buf;
                case 1:
                    if (q == 0) {
                        std::snprintf(buf, sizeof buf, "POP  %s", (p == 2) ? hl : kRp2[p]);
                        return buf;
                    }
                    switch (p) {
                        case 0: return "RET";
                        case 1: return "EXX";
                        case 2: std::snprintf(buf, sizeof buf, "JP   (%s)", hl); return buf;
                        default: std::snprintf(buf, sizeof buf, "LD   SP,%s", hl); return buf;
                    }
                case 2:
                    std::snprintf(buf, sizeof buf, "JP   %s,%s", kCc[y], d.abs16().c_str());
                    return buf;
                case 3:
                    switch (y) {
                        case 0: std::snprintf(buf, sizeof buf, "JP   %s", d.abs16().c_str()); return buf;
                        case 2: {
                            const std::uint8_t n = d.fetch();
                            std::snprintf(buf, sizeof buf, "OUT  (%s),A", hex8(n).c_str());
                            return buf;
                        }
                        case 3: {
                            const std::uint8_t n = d.fetch();
                            std::snprintf(buf, sizeof buf, "IN   A,(%s)", hex8(n).c_str());
                            return buf;
                        }
                        case 4: std::snprintf(buf, sizeof buf, "EX   (SP),%s", hl); return buf;
                        case 5: return "EX   DE,HL";
                        case 6: return "DI";
                        default: return "EI";
                    }
                case 4:
                    std::snprintf(buf, sizeof buf, "CALL %s,%s", kCc[y], d.abs16().c_str());
                    return buf;
                case 5:
                    if (q == 0) {
                        std::snprintf(buf, sizeof buf, "PUSH %s", (p == 2) ? hl : kRp2[p]);
                        return buf;
                    }
                    std::snprintf(buf, sizeof buf, "CALL %s", d.abs16().c_str());
                    return buf;
                case 6: {
                    const std::uint8_t n = d.fetch();
                    std::snprintf(buf, sizeof buf, "%s%s", kAlu[y], hex8(n).c_str());
                    return buf;
                }
                default:
                    d.target = y * 8;
                    std::snprintf(buf, sizeof buf, "RST  %02XH", y * 8);
                    return buf;
            }
    }
}

}  // namespace

Instruction disassemble(const ReadByte& read, std::uint16_t addr) {
    Decoder d{read, addr};
    Instruction out;
    out.addr = addr;
    out.text = decode_main(d, Index{});
    out.length = d.len;
    out.target = d.target;
    return out;
}

int instruction_length(const ReadByte& read, std::uint16_t addr) {
    Decoder d{read, addr};
    decode_main(d, Index{});
    return d.len;
}

}  // namespace ein
