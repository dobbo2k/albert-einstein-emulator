#include "z80.h"

#include "machine.h"

namespace ein {
namespace {

// Flag lookup tables. sz53 = S, Z, F5, F3; sz53p adds parity.
struct Tables {
    std::uint8_t sz53[256]{};
    std::uint8_t sz53p[256]{};
    std::uint8_t parity[256]{};
    constexpr Tables() {
        for (int i = 0; i < 256; ++i) {
            int bits = 0;
            for (int b = 0; b < 8; ++b)
                if (i & (1 << b)) ++bits;
            parity[i] = (bits & 1) ? 0 : Z80::PF;
            auto f = static_cast<std::uint8_t>(i & (Z80::SF | Z80::YF | Z80::XF));
            if (i == 0) f |= Z80::ZF;
            sz53[i] = f;
            sz53p[i] = static_cast<std::uint8_t>(f | parity[i]);
        }
    }
};
constexpr Tables T{};

}  // namespace

// ---------------------------------------------------------------- bus glue

// Every internal delay and bus access funnels through here, so this is where
// the instruction's own T-state total is accumulated as well as being handed
// to the devices. step() and irq() return ts_; without the += they returned 0,
// which stalled the app's frame budget and hid every interrupt from
// Machine::interrupts_taken.
void Z80::tick(int n) {
    ts_ += n;
    m_->tick(n);
}

std::uint8_t Z80::rd(std::uint16_t a) {
    tick(3);
    return m_->read(a);
}

void Z80::wr(std::uint16_t a, std::uint8_t v) {
    tick(3);
    m_->write(a, v);
}

std::uint8_t Z80::io_in(std::uint16_t p) {
    tick(4);
    return m_->io_read(p);
}

void Z80::io_out(std::uint16_t p, std::uint8_t v) {
    tick(4);
    m_->io_write(p, v);
}

std::uint8_t Z80::fetch_op() {
    tick(4);
    refresh();
    return m_->read(pc.w++);
}

std::uint8_t Z80::fetch() {
    tick(3);
    return m_->read(pc.w++);
}

std::uint16_t Z80::fetch16() {
    const std::uint8_t lo = fetch();
    const std::uint8_t hi = fetch();
    return static_cast<std::uint16_t>(lo | (hi << 8));
}

void Z80::push16(std::uint16_t v) {
    wr(--sp.w, static_cast<std::uint8_t>(v >> 8));
    wr(--sp.w, static_cast<std::uint8_t>(v));
}

std::uint16_t Z80::pop16() {
    const std::uint8_t lo = rd(sp.w++);
    const std::uint8_t hi = rd(sp.w++);
    return static_cast<std::uint16_t>(lo | (hi << 8));
}

// -------------------------------------------------------------------- ALU

void Z80::add8(std::uint8_t v, std::uint8_t carry) {
    const std::uint8_t a = A();
    const unsigned res = a + v + carry;
    const auto r8 = static_cast<std::uint8_t>(res);
    set_f(static_cast<std::uint8_t>((r8 & (SF | YF | XF)) | (r8 ? 0 : ZF) |
                                    ((a ^ v ^ r8) & HF) |
                                    ((((a ^ v ^ 0x80) & (a ^ r8)) >> 5) & PF) |
                                    ((res >> 8) & CF)));
    A() = r8;
}

void Z80::sub8(std::uint8_t v, std::uint8_t carry, bool store) {
    const std::uint8_t a = A();
    const unsigned res = a - v - carry;
    const auto r8 = static_cast<std::uint8_t>(res);
    // CP takes its undocumented bits from the operand, not the result.
    const std::uint8_t yx = store ? (r8 & (YF | XF)) : (v & (YF | XF));
    set_f(static_cast<std::uint8_t>((r8 & SF) | (r8 ? 0 : ZF) | NF | yx |
                                    ((a ^ v ^ r8) & HF) |
                                    ((((a ^ v) & (a ^ r8)) >> 5) & PF) |
                                    ((res >> 8) & CF)));
    if (store) A() = r8;
}

void Z80::and8(std::uint8_t v) {
    A() &= v;
    set_f(static_cast<std::uint8_t>(T.sz53p[A()] | HF));
}

void Z80::xor8(std::uint8_t v) {
    A() ^= v;
    set_f(T.sz53p[A()]);
}

void Z80::or8(std::uint8_t v) {
    A() |= v;
    set_f(T.sz53p[A()]);
}

std::uint8_t Z80::inc8(std::uint8_t v) {
    const auto r8 = static_cast<std::uint8_t>(v + 1);
    set_f(static_cast<std::uint8_t>((F() & CF) | (r8 & (SF | YF | XF)) | (r8 ? 0 : ZF) |
                                    ((r8 & 0x0f) ? 0 : HF) | ((r8 == 0x80) ? PF : 0)));
    return r8;
}

std::uint8_t Z80::dec8(std::uint8_t v) {
    const auto r8 = static_cast<std::uint8_t>(v - 1);
    set_f(static_cast<std::uint8_t>((F() & CF) | NF | (r8 & (SF | YF | XF)) | (r8 ? 0 : ZF) |
                                    (((r8 & 0x0f) == 0x0f) ? HF : 0) |
                                    ((r8 == 0x7f) ? PF : 0)));
    return r8;
}

void Z80::add16(RegPair& dst, std::uint16_t v) {
    const std::uint16_t a = dst.w;
    const unsigned res = a + v;
    wz.w = static_cast<std::uint16_t>(a + 1);
    set_f(static_cast<std::uint8_t>((F() & (SF | ZF | PF)) |
                                    (((a ^ v ^ res) >> 8) & HF) |
                                    ((res >> 16) & CF) |
                                    ((res >> 8) & (YF | XF))));
    dst.w = static_cast<std::uint16_t>(res);
}

void Z80::adc16(std::uint16_t v) {
    const std::uint16_t a = hl.w;
    const unsigned res = a + v + (F() & CF);
    wz.w = static_cast<std::uint16_t>(a + 1);
    const auto r16 = static_cast<std::uint16_t>(res);
    set_f(static_cast<std::uint8_t>(((res >> 8) & (SF | YF | XF)) | (r16 ? 0 : ZF) |
                                    (((a ^ v ^ res) >> 8) & HF) |
                                    ((((a ^ v ^ 0x8000) & (a ^ res)) >> 13) & PF) |
                                    ((res >> 16) & CF)));
    hl.w = r16;
}

void Z80::sbc16(std::uint16_t v) {
    const std::uint16_t a = hl.w;
    const unsigned res = a - v - (F() & CF);
    wz.w = static_cast<std::uint16_t>(a + 1);
    const auto r16 = static_cast<std::uint16_t>(res);
    set_f(static_cast<std::uint8_t>(((res >> 8) & (SF | YF | XF)) | (r16 ? 0 : ZF) | NF |
                                    (((a ^ v ^ res) >> 8) & HF) |
                                    ((((a ^ v) & (a ^ res)) >> 13) & PF) |
                                    ((res >> 16) & CF)));
    hl.w = r16;
}

void Z80::daa() {
    const std::uint8_t a = A();
    const std::uint8_t f = F();
    std::uint8_t corr = 0;
    if ((a & 0x0f) > 9 || (f & HF)) corr |= 0x06;
    const bool carry = (a > 0x99) || (f & CF);
    if (carry) corr |= 0x60;

    std::uint8_t nf = (f & NF);
    std::uint8_t res;
    if (f & NF) {
        res = static_cast<std::uint8_t>(a - corr);
        if ((f & HF) && ((a & 0x0f) < 6)) nf |= HF;
    } else {
        res = static_cast<std::uint8_t>(a + corr);
        if ((a & 0x0f) > 9) nf |= HF;
    }
    if (carry) nf |= CF;
    A() = res;
    set_f(static_cast<std::uint8_t>(nf | T.sz53p[res]));
}

// CB rotate/shift group: 0 RLC, 1 RRC, 2 RL, 3 RR, 4 SLA, 5 SRA, 6 SLL, 7 SRL.
std::uint8_t Z80::rot(int op, std::uint8_t v) {
    std::uint8_t c = 0, res = 0;
    switch (op) {
        case 0: c = static_cast<std::uint8_t>(v >> 7); res = static_cast<std::uint8_t>((v << 1) | c); break;
        case 1: c = static_cast<std::uint8_t>(v & 1); res = static_cast<std::uint8_t>((v >> 1) | (c << 7)); break;
        case 2: c = static_cast<std::uint8_t>(v >> 7); res = static_cast<std::uint8_t>((v << 1) | (F() & CF)); break;
        case 3: c = static_cast<std::uint8_t>(v & 1); res = static_cast<std::uint8_t>((v >> 1) | ((F() & CF) << 7)); break;
        case 4: c = static_cast<std::uint8_t>(v >> 7); res = static_cast<std::uint8_t>(v << 1); break;
        case 5: c = static_cast<std::uint8_t>(v & 1); res = static_cast<std::uint8_t>((v >> 1) | (v & 0x80)); break;
        case 6: c = static_cast<std::uint8_t>(v >> 7); res = static_cast<std::uint8_t>((v << 1) | 1); break;  // undocumented
        case 7: c = static_cast<std::uint8_t>(v & 1); res = static_cast<std::uint8_t>(v >> 1); break;
        default: break;
    }
    set_f(static_cast<std::uint8_t>(T.sz53p[res] | c));
    return res;
}

// ---------------------------------------------------------------- control

void Z80::reset() {
    pc.w = 0;
    i = 0;
    r = 0;
    iff1 = iff2 = false;
    im = 0;
    halted = false;
    wz.w = 0;
    q = 0;
    ei_pending = false;
}

int Z80::step(Machine& bus) {
    m_ = &bus;
    ts_ = 0;
    q_new_ = 0;
    ei_pending = false;

    if (halted) {
        // The CPU keeps fetching the same NOP until an interrupt arrives.
        tick(4);
        refresh();
        q = 0;
        return ts_;
    }

    // Chained DD/FD prefixes: only the last one counts, each costs its own M1.
    RegPair* idx = nullptr;
    std::uint8_t op;
    for (;;) {
        op = fetch_op();
        if (op == 0xdd) { idx = &ix; continue; }
        if (op == 0xfd) { idx = &iy; continue; }
        break;
    }

    if (op == 0xcb) {
        if (idx) exec_ddcb(idx);
        else exec_cb();
    } else if (op == 0xed) {
        exec_ed();
    } else {
        exec_main(op, idx);
    }

    q = q_new_;
    return ts_;
}

int Z80::irq(Machine& bus, std::uint8_t data) {
    if (!iff1 || ei_pending) return 0;
    m_ = &bus;
    ts_ = 0;
    // HALT parks PC on its own opcode so the CPU re-fetches it while halted.
    // Accepting an interrupt has to step past it, or the address pushed is the
    // HALT itself and the program halts again the moment the handler returns --
    // which would trap anything using HALT to wait for a frame interrupt.
    if (halted) {
        halted = false;
        ++pc.w;
    }
    iff1 = iff2 = false;
    refresh();

    switch (im) {
        case 0:
            // The device supplies an opcode; in practice always an RST.
            tick(6);
            if ((data & 0xc7) == 0xc7) {
                push16(pc.w);
                pc.w = static_cast<std::uint16_t>(data & 0x38);
                wz.w = pc.w;
            }
            break;
        case 1:
            tick(7);
            push16(pc.w);
            pc.w = 0x0038;
            wz.w = pc.w;
            break;
        default: {
            tick(7);
            push16(pc.w);
            const auto vec = static_cast<std::uint16_t>((i << 8) | data);
            const std::uint8_t lo = rd(vec);
            const std::uint8_t hi = rd(static_cast<std::uint16_t>(vec + 1));
            pc.w = static_cast<std::uint16_t>(lo | (hi << 8));
            wz.w = pc.w;
            break;
        }
    }
    return ts_;
}

int Z80::nmi(Machine& bus) {
    m_ = &bus;
    ts_ = 0;
    if (halted) {
        halted = false;
        ++pc.w;
    }
    iff2 = iff1;
    iff1 = false;
    refresh();
    tick(5);
    push16(pc.w);
    pc.w = 0x0066;
    wz.w = pc.w;
    return ts_;
}

// ------------------------------------------------------------- main decode

void Z80::exec_main(std::uint8_t op, RegPair* idx) {
    const int x = op >> 6;
    const int y = (op >> 3) & 7;
    const int z = op & 7;
    const int p = y >> 1;
    const int qb = y & 1;

    // Does this opcode use (HL) as an operand? If so and we are prefixed, the
    // operand becomes (IX+d) and H/L elsewhere stay the *real* H/L.
    bool mem_operand = false;
    if (x == 0 && (z == 4 || z == 5 || z == 6) && y == 6) mem_operand = true;
    if (x == 1 && ((y == 6) != (z == 6))) mem_operand = true;
    if (x == 2 && z == 6) mem_operand = true;

    std::uint16_t ea = 0;
    if (idx && mem_operand) {
        const auto d = static_cast<std::int8_t>(fetch());
        ea = static_cast<std::uint16_t>(idx->w + d);
        wz.w = ea;
        tick(5);
    } else if (mem_operand) {
        ea = hl.w;
    }
    const bool sub = (idx != nullptr) && !mem_operand;

    auto reg_ptr = [&](int c) -> std::uint8_t* {
        switch (c) {
            case 0: return &bc.b.h;
            case 1: return &bc.b.l;
            case 2: return &de.b.h;
            case 3: return &de.b.l;
            case 4: return sub ? &idx->b.h : &hl.b.h;
            case 5: return sub ? &idx->b.l : &hl.b.l;
            case 7: return &af.b.h;
            default: return nullptr;
        }
    };
    auto rd_r = [&](int c) -> std::uint8_t {
        std::uint8_t* ptr = reg_ptr(c);
        return ptr ? *ptr : rd(ea);
    };
    auto wr_r = [&](int c, std::uint8_t v) {
        std::uint8_t* ptr = reg_ptr(c);
        if (ptr) *ptr = v;
        else wr(ea, v);
    };
    auto rp = [&](int n) -> RegPair& {
        switch (n) {
            case 0: return bc;
            case 1: return de;
            case 2: return idx ? *idx : hl;
            default: return sp;
        }
    };
    auto rp2 = [&](int n) -> RegPair& {
        switch (n) {
            case 0: return bc;
            case 1: return de;
            case 2: return idx ? *idx : hl;
            default: return af;
        }
    };
    auto cond = [&](int c) -> bool {
        switch (c) {
            case 0: return !(F() & ZF);
            case 1: return (F() & ZF) != 0;
            case 2: return !(F() & CF);
            case 3: return (F() & CF) != 0;
            case 4: return !(F() & PF);
            case 5: return (F() & PF) != 0;
            case 6: return !(F() & SF);
            default: return (F() & SF) != 0;
        }
    };
    auto alu = [&](int o, std::uint8_t v) {
        switch (o) {
            case 0: add8(v, 0); break;
            case 1: add8(v, static_cast<std::uint8_t>(F() & CF)); break;
            case 2: sub8(v, 0, true); break;
            case 3: sub8(v, static_cast<std::uint8_t>(F() & CF), true); break;
            case 4: and8(v); break;
            case 5: xor8(v); break;
            case 6: or8(v); break;
            default: sub8(v, 0, false); break;
        }
    };

    switch (x) {
        case 0:
            switch (z) {
                case 0:
                    if (y == 0) {
                        // NOP
                    } else if (y == 1) {
                        std::swap(af.w, af2.w);
                    } else if (y == 2) {  // DJNZ
                        tick(1);
                        const auto d = static_cast<std::int8_t>(fetch());
                        if (--bc.b.h != 0) {
                            tick(5);
                            pc.w = static_cast<std::uint16_t>(pc.w + d);
                            wz.w = pc.w;
                        }
                    } else if (y == 3) {  // JR d
                        const auto d = static_cast<std::int8_t>(fetch());
                        tick(5);
                        pc.w = static_cast<std::uint16_t>(pc.w + d);
                        wz.w = pc.w;
                    } else {  // JR cc,d
                        const auto d = static_cast<std::int8_t>(fetch());
                        if (cond(y - 4)) {
                            tick(5);
                            pc.w = static_cast<std::uint16_t>(pc.w + d);
                            wz.w = pc.w;
                        }
                    }
                    break;

                case 1:
                    if (qb == 0) {
                        rp(p).w = fetch16();
                    } else {
                        tick(7);
                        add16(idx ? *idx : hl, rp(p).w);
                    }
                    break;

                case 2:
                    if (qb == 0) {
                        switch (p) {
                            case 0:
                                wr(bc.w, A());
                                wz.b.l = static_cast<std::uint8_t>(bc.w + 1);
                                wz.b.h = A();
                                break;
                            case 1:
                                wr(de.w, A());
                                wz.b.l = static_cast<std::uint8_t>(de.w + 1);
                                wz.b.h = A();
                                break;
                            case 2: {
                                const std::uint16_t nn = fetch16();
                                const RegPair& s = idx ? *idx : hl;
                                wr(nn, s.b.l);
                                wr(static_cast<std::uint16_t>(nn + 1), s.b.h);
                                wz.w = static_cast<std::uint16_t>(nn + 1);
                                break;
                            }
                            default: {
                                const std::uint16_t nn = fetch16();
                                wr(nn, A());
                                wz.b.l = static_cast<std::uint8_t>(nn + 1);
                                wz.b.h = A();
                                break;
                            }
                        }
                    } else {
                        switch (p) {
                            case 0:
                                A() = rd(bc.w);
                                wz.w = static_cast<std::uint16_t>(bc.w + 1);
                                break;
                            case 1:
                                A() = rd(de.w);
                                wz.w = static_cast<std::uint16_t>(de.w + 1);
                                break;
                            case 2: {
                                const std::uint16_t nn = fetch16();
                                RegPair& d = idx ? *idx : hl;
                                d.b.l = rd(nn);
                                d.b.h = rd(static_cast<std::uint16_t>(nn + 1));
                                wz.w = static_cast<std::uint16_t>(nn + 1);
                                break;
                            }
                            default: {
                                const std::uint16_t nn = fetch16();
                                A() = rd(nn);
                                wz.w = static_cast<std::uint16_t>(nn + 1);
                                break;
                            }
                        }
                    }
                    break;

                case 3:
                    tick(2);
                    if (qb == 0) ++rp(p).w;
                    else --rp(p).w;
                    break;

                case 4: {
                    const std::uint8_t v = rd_r(y);
                    if (y == 6) tick(1);
                    wr_r(y, inc8(v));
                    break;
                }
                case 5: {
                    const std::uint8_t v = rd_r(y);
                    if (y == 6) tick(1);
                    wr_r(y, dec8(v));
                    break;
                }
                case 6: {
                    const std::uint8_t n = fetch();
                    wr_r(y, n);
                    break;
                }
                default:
                    switch (y) {
                        case 0: {  // RLCA
                            const std::uint8_t c = static_cast<std::uint8_t>(A() >> 7);
                            A() = static_cast<std::uint8_t>((A() << 1) | c);
                            set_f(static_cast<std::uint8_t>((F() & (SF | ZF | PF)) |
                                                            (A() & (YF | XF)) | c));
                            break;
                        }
                        case 1: {  // RRCA
                            const std::uint8_t c = static_cast<std::uint8_t>(A() & 1);
                            A() = static_cast<std::uint8_t>((A() >> 1) | (c << 7));
                            set_f(static_cast<std::uint8_t>((F() & (SF | ZF | PF)) |
                                                            (A() & (YF | XF)) | c));
                            break;
                        }
                        case 2: {  // RLA
                            const std::uint8_t c = static_cast<std::uint8_t>(A() >> 7);
                            A() = static_cast<std::uint8_t>((A() << 1) | (F() & CF));
                            set_f(static_cast<std::uint8_t>((F() & (SF | ZF | PF)) |
                                                            (A() & (YF | XF)) | c));
                            break;
                        }
                        case 3: {  // RRA
                            const std::uint8_t c = static_cast<std::uint8_t>(A() & 1);
                            A() = static_cast<std::uint8_t>((A() >> 1) | ((F() & CF) << 7));
                            set_f(static_cast<std::uint8_t>((F() & (SF | ZF | PF)) |
                                                            (A() & (YF | XF)) | c));
                            break;
                        }
                        case 4:
                            daa();
                            break;
                        case 5:  // CPL
                            A() = static_cast<std::uint8_t>(~A());
                            set_f(static_cast<std::uint8_t>((F() & (SF | ZF | PF | CF)) | HF | NF |
                                                            (A() & (YF | XF))));
                            break;
                        case 6: {  // SCF
                            const std::uint8_t yx =
                                static_cast<std::uint8_t>(((q ^ F()) | A()) & (YF | XF));
                            set_f(static_cast<std::uint8_t>((F() & (SF | ZF | PF)) | CF | yx));
                            break;
                        }
                        default: {  // CCF
                            const std::uint8_t f = F();
                            const std::uint8_t yx =
                                static_cast<std::uint8_t>(((q ^ f) | A()) & (YF | XF));
                            set_f(static_cast<std::uint8_t>((f & (SF | ZF | PF)) |
                                                            ((f & CF) ? HF : 0) |
                                                            ((f & CF) ^ CF) | yx));
                            break;
                        }
                    }
                    break;
            }
            break;

        case 1:
            if (y == 6 && z == 6) {
                halted = true;
                --pc.w;
            } else {
                wr_r(y, rd_r(z));
            }
            break;

        case 2:
            alu(y, rd_r(z));
            break;

        default:
            switch (z) {
                case 0:
                    tick(1);
                    if (cond(y)) {
                        pc.w = pop16();
                        wz.w = pc.w;
                    }
                    break;

                case 1:
                    if (qb == 0) {
                        rp2(p).w = pop16();
                    } else {
                        switch (p) {
                            case 0:
                                pc.w = pop16();
                                wz.w = pc.w;
                                break;
                            case 1:
                                std::swap(bc.w, bc2.w);
                                std::swap(de.w, de2.w);
                                std::swap(hl.w, hl2.w);
                                break;
                            case 2:
                                pc.w = idx ? idx->w : hl.w;
                                break;
                            default:
                                tick(2);
                                sp.w = idx ? idx->w : hl.w;
                                break;
                        }
                    }
                    break;

                case 2: {
                    const std::uint16_t nn = fetch16();
                    wz.w = nn;
                    if (cond(y)) pc.w = nn;
                    break;
                }

                case 3:
                    switch (y) {
                        case 0: {
                            const std::uint16_t nn = fetch16();
                            wz.w = nn;
                            pc.w = nn;
                            break;
                        }
                        case 2: {  // OUT (n),A
                            const std::uint8_t n = fetch();
                            const auto port = static_cast<std::uint16_t>((A() << 8) | n);
                            io_out(port, A());
                            wz.b.l = static_cast<std::uint8_t>(n + 1);
                            wz.b.h = A();
                            break;
                        }
                        case 3: {  // IN A,(n)
                            const std::uint8_t n = fetch();
                            const auto port = static_cast<std::uint16_t>((A() << 8) | n);
                            A() = io_in(port);
                            wz.w = static_cast<std::uint16_t>(port + 1);
                            break;
                        }
                        case 4: {  // EX (SP),HL/IX/IY
                            RegPair& t = idx ? *idx : hl;
                            const std::uint8_t lo = rd(sp.w);
                            const std::uint8_t hi = rd(static_cast<std::uint16_t>(sp.w + 1));
                            tick(1);
                            wr(static_cast<std::uint16_t>(sp.w + 1), t.b.h);
                            wr(sp.w, t.b.l);
                            tick(2);
                            t.b.l = lo;
                            t.b.h = hi;
                            wz.w = t.w;
                            break;
                        }
                        case 5:
                            std::swap(de.w, hl.w);
                            break;
                        case 6:
                            iff1 = iff2 = false;
                            break;
                        default:
                            iff1 = iff2 = true;
                            ei_pending = true;
                            break;
                    }
                    break;

                case 4: {
                    const std::uint16_t nn = fetch16();
                    wz.w = nn;
                    if (cond(y)) {
                        tick(1);
                        push16(pc.w);
                        pc.w = nn;
                    }
                    break;
                }

                case 5:
                    if (qb == 0) {
                        tick(1);
                        push16(rp2(p).w);
                    } else if (p == 0) {
                        const std::uint16_t nn = fetch16();
                        wz.w = nn;
                        tick(1);
                        push16(pc.w);
                        pc.w = nn;
                    }
                    break;

                case 6:
                    alu(y, fetch());
                    break;

                default:
                    tick(1);
                    push16(pc.w);
                    pc.w = static_cast<std::uint16_t>(y * 8);
                    wz.w = pc.w;
                    break;
            }
            break;
    }
}

// --------------------------------------------------------------- CB prefix

void Z80::exec_cb() {
    const std::uint8_t op = fetch_op();
    const int x = op >> 6;
    const int y = (op >> 3) & 7;
    const int z = op & 7;

    std::uint8_t* reg = nullptr;
    switch (z) {
        case 0: reg = &bc.b.h; break;
        case 1: reg = &bc.b.l; break;
        case 2: reg = &de.b.h; break;
        case 3: reg = &de.b.l; break;
        case 4: reg = &hl.b.h; break;
        case 5: reg = &hl.b.l; break;
        case 7: reg = &af.b.h; break;
        default: break;
    }

    const std::uint8_t v = reg ? *reg : rd(hl.w);

    if (x == 1) {  // BIT
        const auto res = static_cast<std::uint8_t>(v & (1 << y));
        std::uint8_t f = static_cast<std::uint8_t>((F() & CF) | HF | (res & SF));
        if (!res) f |= static_cast<std::uint8_t>(ZF | PF);
        // For (HL) the undocumented bits come from MEMPTR, not the operand.
        f |= static_cast<std::uint8_t>(reg ? (v & (YF | XF)) : (wz.b.h & (YF | XF)));
        set_f(f);
        if (!reg) tick(1);
        return;
    }

    std::uint8_t res;
    if (x == 0) {
        res = rot(y, v);
    } else if (x == 2) {
        res = static_cast<std::uint8_t>(v & ~(1 << y));
    } else {
        res = static_cast<std::uint8_t>(v | (1 << y));
    }

    if (reg) {
        *reg = res;
    } else {
        tick(1);
        wr(hl.w, res);
    }
}

// ------------------------------------------------------------ DDCB / FDCB

void Z80::exec_ddcb(RegPair* idx) {
    const auto d = static_cast<std::int8_t>(fetch());
    const std::uint8_t op = fetch();  // not an M1 cycle: R does not increment
    tick(2);

    const auto ea = static_cast<std::uint16_t>(idx->w + d);
    wz.w = ea;

    const int x = op >> 6;
    const int y = (op >> 3) & 7;
    const int z = op & 7;

    const std::uint8_t v = rd(ea);

    if (x == 1) {  // BIT b,(IX+d) -- undocumented bits from the address high byte
        const auto res = static_cast<std::uint8_t>(v & (1 << y));
        std::uint8_t f = static_cast<std::uint8_t>((F() & CF) | HF | (res & SF));
        if (!res) f |= static_cast<std::uint8_t>(ZF | PF);
        f |= static_cast<std::uint8_t>(wz.b.h & (YF | XF));
        set_f(f);
        tick(1);
        return;
    }

    std::uint8_t res;
    if (x == 0) {
        res = rot(y, v);
    } else if (x == 2) {
        res = static_cast<std::uint8_t>(v & ~(1 << y));
    } else {
        res = static_cast<std::uint8_t>(v | (1 << y));
    }

    tick(1);
    wr(ea, res);

    // Undocumented: the result is also copied into the named register.
    switch (z) {
        case 0: bc.b.h = res; break;
        case 1: bc.b.l = res; break;
        case 2: de.b.h = res; break;
        case 3: de.b.l = res; break;
        case 4: hl.b.h = res; break;
        case 5: hl.b.l = res; break;
        case 7: af.b.h = res; break;
        default: break;
    }
}

// --------------------------------------------------------------- ED prefix

void Z80::exec_ed() {
    const std::uint8_t op = fetch_op();
    const int x = op >> 6;
    const int y = (op >> 3) & 7;
    const int z = op & 7;
    const int p = y >> 1;
    const int qb = y & 1;

    auto rp = [&](int n) -> RegPair& {
        switch (n) {
            case 0: return bc;
            case 1: return de;
            case 2: return hl;
            default: return sp;
        }
    };
    auto reg_ptr = [&](int c) -> std::uint8_t* {
        switch (c) {
            case 0: return &bc.b.h;
            case 1: return &bc.b.l;
            case 2: return &de.b.h;
            case 3: return &de.b.l;
            case 4: return &hl.b.h;
            case 5: return &hl.b.l;
            case 7: return &af.b.h;
            default: return nullptr;
        }
    };

    if (x == 1) {
        switch (z) {
            case 0: {  // IN r,(C)
                const std::uint8_t v = io_in(bc.w);
                wz.w = static_cast<std::uint16_t>(bc.w + 1);
                if (std::uint8_t* ptr = reg_ptr(y)) *ptr = v;
                set_f(static_cast<std::uint8_t>((F() & CF) | T.sz53p[v]));
                break;
            }
            case 1: {  // OUT (C),r
                std::uint8_t* ptr = reg_ptr(y);
                io_out(bc.w, ptr ? *ptr : 0);
                wz.w = static_cast<std::uint16_t>(bc.w + 1);
                break;
            }
            case 2:
                tick(7);
                if (qb == 0) sbc16(rp(p).w);
                else adc16(rp(p).w);
                break;
            case 3: {
                const std::uint16_t nn = fetch16();
                wz.w = static_cast<std::uint16_t>(nn + 1);
                if (qb == 0) {
                    wr(nn, rp(p).b.l);
                    wr(static_cast<std::uint16_t>(nn + 1), rp(p).b.h);
                } else {
                    rp(p).b.l = rd(nn);
                    rp(p).b.h = rd(static_cast<std::uint16_t>(nn + 1));
                }
                break;
            }
            case 4: {  // NEG
                const std::uint8_t v = A();
                A() = 0;
                sub8(v, 0, true);
                break;
            }
            case 5:  // RETN / RETI
                iff1 = iff2;
                pc.w = pop16();
                wz.w = pc.w;
                // RETI (ED 4D) is watched by peripherals on the daisy chain.
                if (y == 1) m_->on_reti();
                break;
            case 6:
                im = (y == 0 || y == 1 || y == 4 || y == 5) ? 0 : ((y == 2 || y == 6) ? 1 : 2);
                break;
            default:
                switch (y) {
                    case 0: tick(1); i = A(); break;
                    case 1: tick(1); r = A(); break;
                    case 2:
                        tick(1);
                        A() = i;
                        set_f(static_cast<std::uint8_t>((F() & CF) | T.sz53[A()] |
                                                        (iff2 ? PF : 0)));
                        break;
                    case 3:
                        tick(1);
                        A() = r;
                        set_f(static_cast<std::uint8_t>((F() & CF) | T.sz53[A()] |
                                                        (iff2 ? PF : 0)));
                        break;
                    case 4: {  // RRD
                        const std::uint8_t v = rd(hl.w);
                        tick(4);
                        wr(hl.w, static_cast<std::uint8_t>((v >> 4) | (A() << 4)));
                        A() = static_cast<std::uint8_t>((A() & 0xf0) | (v & 0x0f));
                        wz.w = static_cast<std::uint16_t>(hl.w + 1);
                        set_f(static_cast<std::uint8_t>((F() & CF) | T.sz53p[A()]));
                        break;
                    }
                    case 5: {  // RLD
                        const std::uint8_t v = rd(hl.w);
                        tick(4);
                        wr(hl.w, static_cast<std::uint8_t>((v << 4) | (A() & 0x0f)));
                        A() = static_cast<std::uint8_t>((A() & 0xf0) | (v >> 4));
                        wz.w = static_cast<std::uint16_t>(hl.w + 1);
                        set_f(static_cast<std::uint8_t>((F() & CF) | T.sz53p[A()]));
                        break;
                    }
                    default:
                        break;  // NOP
                }
                break;
        }
        return;
    }

    if (x == 2 && z <= 3 && y >= 4) {
        const int inc = (y & 1) ? -1 : 1;
        const bool repeat = (y & 2) != 0;
        switch (z) {
            case 0: block_ld(inc, repeat); break;
            case 1: block_cp(inc, repeat); break;
            case 2: block_in(inc, repeat); break;
            default: block_out(inc, repeat); break;
        }
        return;
    }
    // Everything else on the ED page behaves as NOP.
}

// ------------------------------------------------------------- block group

void Z80::block_ld(int inc, bool repeat) {
    const std::uint8_t v = rd(hl.w);
    wr(de.w, v);
    tick(2);
    hl.w = static_cast<std::uint16_t>(hl.w + inc);
    de.w = static_cast<std::uint16_t>(de.w + inc);
    --bc.w;

    const auto n = static_cast<std::uint8_t>(v + A());
    set_f(static_cast<std::uint8_t>((F() & (SF | ZF | CF)) | ((n & 0x02) ? YF : 0) |
                                    ((n & 0x08) ? XF : 0) | (bc.w ? PF : 0)));

    if (repeat && bc.w) {
        tick(5);
        pc.w = static_cast<std::uint16_t>(pc.w - 2);
        wz.w = static_cast<std::uint16_t>(pc.w + 1);
    }
}

void Z80::block_cp(int inc, bool repeat) {
    const std::uint8_t v = rd(hl.w);
    tick(5);
    const std::uint8_t a = A();
    const auto res = static_cast<std::uint8_t>(a - v);
    hl.w = static_cast<std::uint16_t>(hl.w + inc);
    --bc.w;
    wz.w = static_cast<std::uint16_t>(wz.w + inc);

    const auto hf = static_cast<std::uint8_t>((a ^ v ^ res) & HF);
    const auto n = static_cast<std::uint8_t>(res - (hf ? 1 : 0));
    set_f(static_cast<std::uint8_t>((F() & CF) | NF | hf | (res & SF) | (res ? 0 : ZF) |
                                    ((n & 0x02) ? YF : 0) | ((n & 0x08) ? XF : 0) |
                                    (bc.w ? PF : 0)));

    if (repeat && bc.w && res != 0) {
        tick(5);
        pc.w = static_cast<std::uint16_t>(pc.w - 2);
        wz.w = static_cast<std::uint16_t>(pc.w + 1);
    }
}

void Z80::block_in(int inc, bool repeat) {
    tick(1);
    const std::uint8_t v = io_in(bc.w);
    wz.w = static_cast<std::uint16_t>(bc.w + inc);
    wr(hl.w, v);
    --bc.b.h;
    hl.w = static_cast<std::uint16_t>(hl.w + inc);

    const unsigned k = v + ((bc.b.l + inc) & 0xff);
    set_f(static_cast<std::uint8_t>(T.sz53[bc.b.h] | ((v & 0x80) ? NF : 0) |
                                    ((k > 255) ? (HF | CF) : 0) |
                                    T.parity[(k & 7) ^ bc.b.h]));

    if (repeat && bc.b.h) {
        tick(5);
        pc.w = static_cast<std::uint16_t>(pc.w - 2);
    }
}

void Z80::block_out(int inc, bool repeat) {
    tick(1);
    const std::uint8_t v = rd(hl.w);
    --bc.b.h;
    hl.w = static_cast<std::uint16_t>(hl.w + inc);
    io_out(bc.w, v);
    wz.w = static_cast<std::uint16_t>(bc.w + inc);

    const unsigned k = v + bc.b.l;
    set_f(static_cast<std::uint8_t>(T.sz53[bc.b.h] | ((v & 0x80) ? NF : 0) |
                                    ((k > 255) ? (HF | CF) : 0) |
                                    T.parity[(k & 7) ^ bc.b.h]));

    if (repeat && bc.b.h) {
        tick(5);
        pc.w = static_cast<std::uint16_t>(pc.w - 2);
    }
}

}  // namespace ein
