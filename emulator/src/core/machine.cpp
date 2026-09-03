#include "machine.h"

#include <type_traits>

namespace ein {

void Machine::set_rom(RomImage rom) {
    rom_ = std::move(rom);
}

void Machine::reset() {
    // Warm reset: CPU and the ROM overlay only. RAM is deliberately not
    // cleared -- MOS fills it with 0xFF itself as a memory test
    // (Mos12.zsm:255), and preserving it keeps warm-reset behaviour honest.
    cpu.reset();
    vdp.reset();
    ctc.reset();
    psg.reset();
    psg.keyboard = &keyboard;
    keyboard.reset();
    // The FDC is reset but the discs in the drives stay where they are, the
    // same way a real machine does not eject on Ctrl-BREAK.
    fdc.reset();
    drive_select = 0;
    pio.reset();
    uart.reset();
    adc.reset();
    card80.reset();
    adc_mask = 0;
    fire_mask = 0;
    printer.clear();
    printer_ack_ = 0;
    bp_hit_ = false;
    bp_hit_kind_ = 0;
    bp_step_over_ = false;
    uart_tx_seen_ = 0;
    uart_rx_seen_ = 0;
    interrupts_taken = 0;
    rom_enabled = true;
    tstates = 0;
    instructions = 0;
    saw_ram_rom_write = false;
    last_ram_rom_write = 0;
    rom_toggles = 0;
}

std::uint8_t Machine::peek(std::uint16_t addr) const {
    if (rom_enabled && addr < kRomWindowEnd && !rom_.data.empty()) {
        return rom_.data[addr];
    }
    return ram_[addr];
}

std::uint8_t Machine::read(std::uint16_t addr) const {
    if (bp_any_ && (bp_mem_[addr] & kBpRead)) note_bp(addr, kBpRead, false);
    return peek(addr);
}

void Machine::note_bp(std::uint16_t addr, std::uint8_t kind, bool port) const {
    bp_hit_ = true;
    bp_hit_addr_ = addr;
    bp_hit_kind_ = kind;
    bp_hit_port_ = port;
}

void Machine::set_breakpoint(std::uint16_t addr, std::uint8_t kinds) {
    if (bp_mem_[addr] && !kinds) --bp_count_;
    if (!bp_mem_[addr] && kinds) ++bp_count_;
    bp_mem_[addr] = kinds;
    bp_any_ = bp_count_ > 0;
}

void Machine::set_port_breakpoint(std::uint8_t port, std::uint8_t kinds) {
    if (bp_port_[port] && !kinds) --bp_count_;
    if (!bp_port_[port] && kinds) ++bp_count_;
    bp_port_[port] = kinds;
    bp_any_ = bp_count_ > 0;
}

void Machine::clear_all_breakpoints() {
    bp_mem_.fill(0);
    bp_port_.fill(0);
    bp_count_ = 0;
    bp_any_ = false;
    bp_hit_ = false;
    bp_hit_kind_ = 0;
}

void Machine::resume_from_breakpoint() {
    bp_hit_ = false;
    bp_hit_kind_ = 0;
    bp_step_over_ = true;
}

void Machine::write(std::uint16_t addr, std::uint8_t value) {
    if (bp_any_ && (bp_mem_[addr] & kBpWrite)) note_bp(addr, kBpWrite, false);
    if (msx_rom_shadow && addr >= msx_rom_lo && addr < msx_rom_hi) {
        note_stray_write(addr, value);
        return;  // on an MSX this range is cartridge ROM and the write vanishes
    }
    // Writes always land in RAM; the ROM overlay is read-only, so RAM
    // underneath it stays writable even while ROM is paged in.
    ram_[addr] = value;
}

// Records a write the MSX would have discarded, folded by the pc and address it
// came from so that a loop writing thousands of times shows up as one site.
void Machine::note_stray_write(std::uint16_t addr, std::uint8_t value) {
    ++stray_writes_total;
    const std::uint16_t pc = cpu.pc.w;
    for (int i = 0; i < stray_write_sites; ++i) {
        if (stray_writes[i].pc == pc && stray_writes[i].addr == addr) {
            ++stray_writes[i].count;
            return;
        }
    }
    if (stray_write_sites < kMaxStrayWrites) {
        stray_writes[stray_write_sites++] = {pc, addr, value, 1};
    }
}

std::uint8_t Machine::io_read(std::uint16_t port) {
    if (bp_any_ && (bp_port_[port & 0xff] & kBpIn)) {
        note_bp(static_cast<std::uint16_t>(port & 0xff), kBpIn, true);
    }
    // The Einstein decodes I/O in blocks of 8; the VDP occupies 08-0F, with
    // bit 0 selecting data (08) or control/status (09).
    const std::uint8_t p = static_cast<std::uint8_t>(port & 0xff);
    if ((p & 0xf8) == 0x08) {
        return (p & 1) ? vdp.read_status() : vdp.read_data();
    }
    if ((p & 0xf8) == 0x00) {
        // PSG: reading port 02 returns the latched register.
        if (p == kPortPsgSel) return psg.read();
        return 0xff;
    }
    if ((p & 0xf8) == kPortUart) return uart.read(p & 1);
    if ((p & 0xf8) == kPortFdc) return fdc.read(p & 3);
    if ((p & 0xf8) == kPortPio) return pio.read(p & 3);
    if ((p & 0xf8) == kPortAdc) return adc.read();
    // The 80-column card's RAM offset rides on A8-A15, so it needs the whole
    // 16-bit port, not the low byte (Mos12.zsm:L0C13).
    if (card80.decodes(p)) return card80.read(port);
    if (p == kPortDriveSel) return drive_select;
    if (p == kPortKbdMask) {
        // Reading the keyboard mask port returns the modifier keys, active low
        // in bits 5-7 (Mos12.zsm:1350 does IN A,(20) / CPL / AND 0E0H).
        return keyboard.modifier_port();
    }
    if (p == kPortAdcMask) return adc_mask;
    if (p == kPortFireMask) return fire_mask;
    if (p == kPortAlphaLed) return alpha_led;
    if ((p & 0xf8) == kPortCtc) return ctc.read(p & 3);
    // Unclaimed reads see the floating bus, which settles high.
    return 0xff;
}

void Machine::io_write(std::uint16_t port, std::uint8_t value) {
    if (bp_any_ && (bp_port_[port & 0xff] & kBpOut)) {
        note_bp(static_cast<std::uint16_t>(port & 0xff), kBpOut, true);
    }
    const std::uint8_t p = static_cast<std::uint8_t>(port & 0xff);
    if ((p & 0xf8) == 0x08) {
        if (p & 1) vdp.write_ctrl(value);
        else vdp.write_data(value);
        return;
    }
    if ((p & 0xf8) == 0x00) {
        // Port 00 is the soft reset strobe for the FDC and the PSG
        // (Mos12.zsm:8; ZFDRST pulses it 32 times at Mos12.zsm:4239).
        if (p == kPortSoftReset) {
            fdc.reset();
            psg.soft_reset();
            fdc.select(drive_select);
            return;
        }
        if (p == kPortPsgSel) psg.select(value);
        else if (p == kPortPsgWrite) psg.write(value);
        return;
    }
    if ((p & 0xf8) == kPortUart) {
        uart.write(p & 1, value);
        return;
    }
    if ((p & 0xf8) == kPortFdc) {
        fdc.write(p & 3, value);
        return;
    }
    if ((p & 0xf8) == kPortPio) {
        pio.write(p & 3, value);
        return;
    }
    if ((p & 0xf8) == kPortAdc) {
        adc.write(value);
        return;
    }
    if (card80.decodes(p)) {
        card80.write(port, value);
        return;
    }
    if (p == kPortDriveSel) {
        drive_select = value;
        fdc.select(value);
        return;
    }
    if ((p & 0xf8) == kPortCtc) {
        ctc.write(p & 3, value);
        return;
    }
    if (p == kPortKbdMask) {
        kbd_mask = value;
        return;
    }
    if (p == kPortAdcMask) {
        adc_mask = value;
        return;
    }
    if (p == kPortFireMask) {
        fire_mask = value;
        return;
    }
    if (p == kPortAlphaLed) {
        alpha_led = static_cast<std::uint8_t>(alpha_led ^ 0xff);
        return;
    }
    if ((port & 0xff) == kPortRamRom) {
        // Port 24H ignores the data bus: any write toggles the ROM overlay.
        //
        // Established from the RAM trampolines MOS copies to FB14-FB2B
        // (UPPERMEM.ZSM:33-52). Each is "switch, do work, switch back", and
        // LFB14 proves the value is irrelevant:
        //
        //     LFB14:  OUT (24H),A     ; switch
        //             LD  A,(HL)      ; A is overwritten with an arbitrary byte
        //     LFB17:  OUT (24H),A     ; switch back, writing that arbitrary byte
        //
        // The second OUT sends whatever happened to be at (HL) yet must still
        // restore the previous state, so only the write strobe matters. The
        // same shape appears in the LDIR/LDDR/CALL helpers and in the RST 08
        // redirect at LFC3B/LFC40, which brackets CALL 0FCAH with two OUTs
        // holding the same MCAL number in A.
        rom_enabled = !rom_enabled;
        last_ram_rom_write = value;
        saw_ram_rom_write = true;
        ++rom_toggles;
    }
}

// Devices that are still being serviced hold IEO low and shut out everything
// below them, so the chain is walked in order and stops at the first device
// that is either asking or busy. The CTC sits ahead of the PIO: MOS hands the
// CTC vectors 00-06 and the PIO 10 and 12 (Mos12.zsm:291, 301), and the vector
// table at FB00 is laid out in that order (Einstein.zsm:40-49).
//
// The keyboard, ADC, fire button and user vectors (08-0E) come from separate
// logic, not from either of these chips. Where that logic sits in the chain is
// not determined by anything in the archive, and every one of its MOS handlers
// is the EI/RETI stub at LFC4E, so it is left out rather than guessed at.
void Machine::on_reti() {
    if (ctc.int_in_service()) {
        ctc.reti();
        return;
    }
    if (pio.int_in_service()) pio.reti();
}

void Machine::tick_peripherals(int n) {
    // CTC channels 0 and 1 are the 8251's transmit and receive clocks; BAUD
    // (Mos12.zsm:636) programmes them from the table at Mos12.zsm:686, so the
    // serial bit rate is whatever the CTC actually produces.
    const std::uint64_t tx = ctc.zero_counts(0);
    while (uart_tx_seen_ < tx) {
        ++uart_tx_seen_;
        uart.tx_clock();
    }
    const std::uint64_t rx = ctc.zero_counts(1);
    while (uart_rx_seen_ < rx) {
        ++uart_rx_seen_;
        uart.rx_clock();
    }

    // The printer hangs off PIO port A. ZPOUT writes a byte and waits on IFLAG
    // bit 0; the printer's acknowledge strobe raises the port A interrupt and
    // the handler at LFC84 clears that bit.
    std::uint8_t byte = 0;
    if (pio.take_output(0, byte)) {
        printer.push_back(byte);
        printer_ack_ = kPrinterAckTStates;
    }
    if (printer_ack_ > 0) {
        printer_ack_ -= n;
        if (printer_ack_ <= 0) {
            printer_ack_ = 0;
            pio.strobe(0);
        }
    }

    // A finished conversion would raise the ADC interrupt if the mask allowed
    // it. Nothing generates that vector yet -- see on_reti -- so the flag is
    // consumed to keep the converter from latching it forever.
    if (adc.take_conversion_done()) {
        // adc_mask gates it on real hardware; recorded for the debugger.
    }
}

// Devices with no owning members are written whole, which is checked at
// compile time so that adding a std::string to one of them fails loudly here
// rather than corrupting states quietly.
std::vector<std::uint8_t> Machine::save_state() const {
    static_assert(std::is_trivially_copyable_v<Z80>, "Z80 must stay POD");
    static_assert(std::is_trivially_copyable_v<Tms9129>, "Tms9129 must stay POD");
    static_assert(std::is_trivially_copyable_v<Z80Ctc>, "Z80Ctc must stay POD");
    static_assert(std::is_trivially_copyable_v<Z80Pio>, "Z80Pio must stay POD");
    static_assert(std::is_trivially_copyable_v<Adc0844>, "Adc0844 must stay POD");
    static_assert(std::is_trivially_copyable_v<Keyboard>, "Keyboard must stay POD");

    std::vector<std::uint8_t> out;
    out.reserve(kRamSize + 32768);
    StateWriter w(out);
    w.put(kStateMagic);
    w.put(kStateVersion);

    w.bytes(ram_.data(), ram_.size());
    w.bytes(&cpu, sizeof cpu);
    w.bytes(&vdp, sizeof vdp);
    w.bytes(&ctc, sizeof ctc);
    w.bytes(&pio, sizeof pio);
    w.bytes(&adc, sizeof adc);
    w.bytes(&keyboard, sizeof keyboard);
    psg.save_state(w);
    uart.save_state(w);
    fdc.save_state(w);

    w.put(rom_enabled);
    w.put(kbd_mask);
    w.put(alpha_led);
    w.put(drive_select);
    w.put(adc_mask);
    w.put(fire_mask);
    w.put(tstates);
    w.put(instructions);
    w.put(interrupts_taken);
    w.put(rom_toggles);
    w.put(printer_ack_);
    w.put(uart_tx_seen_);
    w.put(uart_rx_seen_);
    return out;
}

bool Machine::load_state(const std::vector<std::uint8_t>& data) {
    if (data.size() < 8) return false;
    StateReader r(data.data(), data.data() + data.size());
    std::uint32_t magic = 0, version = 0;
    r.get(magic);
    r.get(version);
    if (magic != kStateMagic || version != kStateVersion) return false;

    r.bytes(ram_.data(), ram_.size());
    r.bytes(&cpu, sizeof cpu);
    r.bytes(&vdp, sizeof vdp);
    r.bytes(&ctc, sizeof ctc);
    r.bytes(&pio, sizeof pio);
    r.bytes(&adc, sizeof adc);
    r.bytes(&keyboard, sizeof keyboard);
    psg.load_state(r);
    uart.load_state(r);
    fdc.load_state(r);

    r.get(rom_enabled);
    r.get(kbd_mask);
    r.get(alpha_led);
    r.get(drive_select);
    r.get(adc_mask);
    r.get(fire_mask);
    r.get(tstates);
    r.get(instructions);
    r.get(interrupts_taken);
    r.get(rom_toggles);
    r.get(printer_ack_);
    r.get(uart_tx_seen_);
    r.get(uart_rx_seen_);

    // The PSG keeps a raw pointer to the keyboard, which the bulk copy above
    // has just overwritten with the saved machine's address.
    psg.keyboard = &keyboard;
    return r.ok();
}

Machine::StepResult Machine::step() {
    // An execute breakpoint stops *before* the instruction, so the debugger
    // shows the machine as it was when it arrived. resume_from_breakpoint()
    // steps past it once, otherwise continuing would never get anywhere.
    if (bp_any_ && !bp_step_over_ && (bp_mem_[cpu.pc.w] & kBpExec)) {
        note_bp(cpu.pc.w, kBpExec, false);
        return {0, true, "breakpoint", true};
    }
    bp_step_over_ = false;

    // Interrupt acknowledge happens between instructions.
    //
    // The guard here must match Z80::irq's own precondition exactly: int_ack()
    // clears the request and latches it in service, so acknowledging an
    // interrupt the CPU then refuses would drop it silently. A halted CPU is
    // not excluded -- irq() clears the halt itself, which is how the CPU wakes.
    if (cpu.iff1 && !cpu.ei_pending) {
        int vec = -1;
        if (ctc.int_pending()) {
            vec = ctc.int_ack();
        } else if (!ctc.int_in_service() && pio.int_pending()) {
            vec = pio.int_ack();
        }
        if (vec >= 0) {
            const int t = cpu.irq(*this, static_cast<std::uint8_t>(vec));
            ++interrupts_taken;
            ++instructions;
            return {t, true, ""};
        }
    }
    const int t = cpu.step(*this);
    ++instructions;
    return {t, true, ""};
}

}  // namespace ein
