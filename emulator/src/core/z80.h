#pragma once

#include <bit>
#include <cstdint>

namespace ein {

class Machine;  // the bus

static_assert(std::endian::native == std::endian::little,
              "RegPair aliases 16-bit registers onto byte pairs and assumes little-endian");

union RegPair {
    std::uint16_t w;
    struct {
        std::uint8_t l, h;
    } b;
};

// Z80 CPU.
//
// Execution is per-instruction, but every bus access and internal delay calls
// Machine::tick(), so devices advance at the correct T-state offsets *within*
// an instruction. That is what the VDP access timing and the CTC cascade need;
// the CPU never needs to be suspended mid-instruction on this machine.
class Z80 {
public:
    enum Flag : std::uint8_t {
        CF = 0x01,  // carry
        NF = 0x02,  // add/subtract
        PF = 0x04,  // parity / overflow
        XF = 0x08,  // undocumented, bit 3
        HF = 0x10,  // half carry
        YF = 0x20,  // undocumented, bit 5
        ZF = 0x40,  // zero
        SF = 0x80,  // sign
    };

    RegPair af{}, bc{}, de{}, hl{};
    RegPair af2{}, bc2{}, de2{}, hl2{};
    RegPair ix{}, iy{}, sp{}, pc{}, wz{};

    std::uint8_t i = 0;
    std::uint8_t r = 0;
    bool iff1 = false;
    bool iff2 = false;
    std::uint8_t im = 0;
    bool halted = false;

    // Q: the value written to F by the last instruction that touched F, or 0.
    // SCF and CCF derive their undocumented bits 3 and 5 from it.
    std::uint8_t q = 0;

    // EI defers interrupt acceptance until after the following instruction.
    bool ei_pending = false;

    void reset();

    // Executes one instruction (including any prefixes). Returns T-states.
    int step(Machine& bus);

    // Accepts a maskable interrupt. `data` is the byte the device places on the
    // bus during the acknowledge cycle. Returns T-states, or 0 if not taken.
    int irq(Machine& bus, std::uint8_t data);
    int nmi(Machine& bus);

private:
    Machine* m_ = nullptr;
    int ts_ = 0;
    std::uint8_t q_new_ = 0;

    // --- bus helpers (defined in z80.cpp, where Machine is complete) ---
    void tick(int n);
    std::uint8_t rd(std::uint16_t a);
    void wr(std::uint16_t a, std::uint8_t v);
    std::uint8_t io_in(std::uint16_t p);
    void io_out(std::uint16_t p, std::uint8_t v);

    std::uint8_t fetch_op();
    std::uint8_t fetch();
    std::uint16_t fetch16();

    void set_f(std::uint8_t v) {
        af.b.l = v;
        q_new_ = v;
    }
    std::uint8_t F() const { return af.b.l; }
    std::uint8_t& A() { return af.b.h; }

    void push16(std::uint16_t v);
    std::uint16_t pop16();

    void refresh() { r = static_cast<std::uint8_t>((r & 0x80) | ((r + 1) & 0x7f)); }

    // --- ALU ---
    void add8(std::uint8_t v, std::uint8_t carry);
    void sub8(std::uint8_t v, std::uint8_t carry, bool store);
    void and8(std::uint8_t v);
    void xor8(std::uint8_t v);
    void or8(std::uint8_t v);
    std::uint8_t inc8(std::uint8_t v);
    std::uint8_t dec8(std::uint8_t v);
    void add16(RegPair& dst, std::uint16_t v);
    void adc16(std::uint16_t v);
    void sbc16(std::uint16_t v);
    void daa();
    std::uint8_t rot(int op, std::uint8_t v);

    // --- decode groups ---
    void exec_main(std::uint8_t op, RegPair* idx);
    void exec_cb();
    void exec_ddcb(RegPair* idx);
    void exec_ed();
    void block_ld(int inc, bool repeat);
    void block_cp(int inc, bool repeat);
    void block_in(int inc, bool repeat);
    void block_out(int inc, bool repeat);
};

}  // namespace ein
