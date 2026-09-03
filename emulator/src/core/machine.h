#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "adc0844.h"
#include "card80.h"
#include "ay38910.h"
#include "fd1770.h"
#include "i8251.h"
#include "keyboard.h"
#include "rom.h"
#include "state.h"
#include "tms9129.h"
#include "z80.h"
#include "z80ctc.h"
#include "z80pio.h"

namespace ein {

// Tatung Einstein TC-01.
//
// M7 completes the machine: CPU, memory map, ROM/RAM banking, VDP, CTC, the
// mode-2 interrupt daisy chain, keyboard, FD1770, AY-3-8910 sound, Z80 PIO
// (printer), 8251 serial and the ADC0844 joystick converter. The 80-column
// card is the only documented peripheral still unmodelled.
class Machine {
public:
    static constexpr std::size_t kRamSize = 0x10000;
    static constexpr std::uint16_t kRomWindowEnd = 0x2000;  // ROM overlays 0000-1FFF
    static constexpr std::uint8_t kPortRamRom = 0x24;
    static constexpr std::uint8_t kPortVdpData = 0x08;   // 08-0F, decoded on bit 0
    static constexpr std::uint8_t kPortVdpCtrl = 0x09;
    static constexpr std::uint8_t kPortPsgSel = 0x02;   // latch register number
    static constexpr std::uint8_t kPortPsgWrite = 0x03;
    static constexpr std::uint8_t kPortKbdMask = 0x20;  // write: mask; read: modifiers
    static constexpr std::uint8_t kPortAlphaLed = 0x22;
    static constexpr std::uint8_t kPortSoftReset = 0x00;  // resets FDC and PSG
    static constexpr std::uint8_t kPortUart = 0x10;     // 8251, 10-17
    static constexpr std::uint8_t kPortAdcMask = 0x21;  // ADC interrupt mask
    static constexpr std::uint8_t kPortFireMask = 0x25;  // fire button int mask
    static constexpr std::uint8_t kPortPio = 0x30;      // Z80 PIO, 30-37
    static constexpr std::uint8_t kPortAdc = 0x38;      // ADC0844, 38-3F
    static constexpr std::uint8_t kPortCard80 = 0x40;   // 80-column card, 40-4F
    static constexpr std::uint8_t kPortFdc = 0x18;      // 18-1F, decoded on bits 0-1
    static constexpr std::uint8_t kPortDriveSel = 0x23;  // drive one-hot + side
    static constexpr std::uint8_t kPortCtc = 0x28;      // 28-2B
    static constexpr int kCpuClockHz = 4'000'000;  // confirmed: Mos12.zsm:686

    static constexpr bool kCpuImplemented = true;
    static constexpr const char* kCpuStatusNote = "Z80 core active";

    Machine() { reset(); }

    void set_rom(RomImage rom);
    const RomImage& rom() const { return rom_; }
    bool has_rom() const { return !rom_.data.empty(); }

    void reset();

    std::uint8_t read(std::uint16_t addr) const;
    void write(std::uint16_t addr, std::uint8_t value);

    // Reads that must not trip a breakpoint: the debugger's own hex view, the
    // disassembler, the tests.
    std::uint8_t peek(std::uint16_t addr) const;

    std::uint8_t io_read(std::uint16_t port);
    void io_write(std::uint16_t port, std::uint8_t value);

    // Advances every device by n T-states.
    void tick(int n) {
        tstates += static_cast<std::uint64_t>(n);
        vdp.tick(n);
        ctc.tick(n);
        fdc.tick(n);
        psg.tick(n);
        adc.tick(n);
        card80.tick(n);
        tick_peripherals(n);
    }

    // Called by the Z80 when it executes RETI. The chain is released from the
    // top: whichever device is actually in service is the one that answers.
    void on_reti();

    struct StepResult {
        int tstates = 0;
        bool ok = false;
        const char* note = "";
        // Set when execution stopped at a breakpoint instead of running the
        // instruction, so the frontend knows to pause rather than spin.
        bool stopped = false;
    };
    StepResult step();

    // ------------------------------------------------------------ breakpoints
    enum BpKind : std::uint8_t {
        kBpExec = 1,
        kBpRead = 2,
        kBpWrite = 4,
    };
    enum PortBpKind : std::uint8_t {
        kBpIn = 1,
        kBpOut = 2,
    };

    void set_breakpoint(std::uint16_t addr, std::uint8_t kinds);
    void clear_breakpoint(std::uint16_t addr) { set_breakpoint(addr, 0); }
    std::uint8_t breakpoint(std::uint16_t addr) const { return bp_mem_[addr]; }
    void set_port_breakpoint(std::uint8_t port, std::uint8_t kinds);
    std::uint8_t port_breakpoint(std::uint8_t port) const { return bp_port_[port]; }
    void clear_all_breakpoints();
    int breakpoint_count() const { return bp_count_; }

    // The last breakpoint that fired. kind is one of the enums above, or 0 for
    // none; port_hit says whether addr is a port number or an address.
    bool bp_hit() const { return bp_hit_; }
    std::uint16_t bp_hit_addr() const { return bp_hit_addr_; }
    std::uint8_t bp_hit_kind() const { return bp_hit_kind_; }
    bool bp_hit_is_port() const { return bp_hit_port_; }
    // Clears the hit and lets execution past an exec breakpoint once, which is
    // what "continue from here" has to do.
    void resume_from_breakpoint();

    // ----------------------------------------------------------- save states
    // A snapshot of the machine, but not of the discs in the drives, the audio
    // still queued, or serial bytes mid-character. See state.h.
    static constexpr std::uint32_t kStateMagic = 0x384e4945;  // "EIN8"
    static constexpr std::uint32_t kStateVersion = 1;
    std::vector<std::uint8_t> save_state() const;
    bool load_state(const std::vector<std::uint8_t>& data);

    Z80 cpu;
    Tms9129 vdp;
    Z80Ctc ctc;
    Ay38910 psg;
    Keyboard keyboard;
    Fd1770 fdc;
    Z80Pio pio;
    I8251 uart;
    Adc0844 adc;
    Card80 card80;
    bool rom_enabled = true;
    std::uint8_t kbd_mask = 0;
    std::uint8_t alpha_led = 0;
    std::uint8_t drive_select = 0;
    std::uint8_t adc_mask = 0;
    std::uint8_t fire_mask = 0;

    // Everything MOS has sent to the printer through PIO port A.
    std::vector<std::uint8_t> printer;
    // How long the printer takes to acknowledge a byte. Real ones are slower;
    // this only has to be long enough that the handshake is a handshake.
    static constexpr int kPrinterAckTStates = 2000;
    std::uint64_t interrupts_taken = 0;
    std::uint64_t tstates = 0;
    std::uint64_t instructions = 0;

    // Port 24H is a write strobe that toggles the ROM overlay; the data bus is
    // ignored. See the derivation in Machine::io_write. The value is kept only
    // for the debugger.
    std::uint8_t last_ram_rom_write = 0;
    bool saw_ram_rom_write = false;
    std::uint64_t rom_toggles = 0;

    // --- MSX ROM shadow -----------------------------------------------------
    //
    // Scaffolding for running an MSX cartridge image out of Einstein RAM. On
    // the MSX, 4000-BFFF is ROM and a write to it vanishes; here it is ordinary
    // RAM, so a write the original discarded lands instead and corrupts the
    // program. With this on, writes to that range are dropped and logged rather
    // than stored, reproducing the MSX guarantee while the offending
    // instructions are found.
    //
    // Off by default, and it must stay off in anything shipped -- real hardware
    // offers no such protection. Turn it on only once a loader has finished
    // filling that range, because MOS writes the program there through this
    // very function.
    // The guarded range is 4000-BFFF for a plain 32 KB cartridge, which is
    // every one of them until a cartridge switches part of its own address
    // space to RAM and then uses it. Montezuma's Revenge does exactly that:
    // it copies itself down to 0000, switches page 1 to the RAM slot and keeps
    // its variables at 7000-73FF, so on that machine 4000-7FFF was RAM and
    // guarding it would drop every write the game depends on. Hence bounds
    // rather than constants; the defaults are the ordinary case and nothing
    // that does not set them can tell the difference.
    static constexpr std::uint16_t kMsxRomLoDefault = 0x4000;
    static constexpr std::uint16_t kMsxRomHiDefault = 0xC000;
    std::uint16_t msx_rom_lo = kMsxRomLoDefault;
    std::uint16_t msx_rom_hi = kMsxRomHiDefault;
    static constexpr int kMaxStrayWrites = 64;

    // pc is the program counter as the write happens, so it is past the
    // instruction's own operands rather than at its first byte.
    struct StrayWrite {
        std::uint16_t pc = 0;
        std::uint16_t addr = 0;
        std::uint8_t value = 0;
        std::uint64_t count = 0;
    };

    bool msx_rom_shadow = false;
    std::array<StrayWrite, kMaxStrayWrites> stray_writes{};
    int stray_write_sites = 0;  // distinct pc/addr pairs recorded
    std::uint64_t stray_writes_total = 0;

private:
    void note_stray_write(std::uint16_t addr, std::uint8_t value);
    void tick_peripherals(int n);
    void note_bp(std::uint16_t addr, std::uint8_t kind, bool port) const;

    std::array<std::uint8_t, kRamSize> bp_mem_{};
    std::array<std::uint8_t, 256> bp_port_{};
    int bp_count_ = 0;
    bool bp_any_ = false;      // one test guards the common case
    bool bp_step_over_ = false;
    mutable bool bp_hit_ = false;
    mutable std::uint16_t bp_hit_addr_ = 0;
    mutable std::uint8_t bp_hit_kind_ = 0;
    mutable bool bp_hit_port_ = false;


    int printer_ack_ = 0;
    std::uint64_t uart_tx_seen_ = 0;
    std::uint64_t uart_rx_seen_ = 0;

    RomImage rom_;
    std::array<std::uint8_t, kRamSize> ram_{};
};

}  // namespace ein
