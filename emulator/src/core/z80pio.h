#pragma once

#include <cstdint>

namespace ein {

// Z80 PIO at ports 30-37 (Einstein.zsm:36). The Einstein wires A0 to the
// control/data select and A1 to the port select, which is the opposite of the
// usual Z80 PIO layout and is settled by how MOS addresses it (Mos12.zsm:301):
//
//   30  port A data     31  port A control
//   32  port B data     33  port B control
//
// Port A is the printer. MOS sets it up as a mode 0 output with vector 10 and
// interrupts enabled, so the printer's acknowledge strobe raises the port A
// interrupt; the handler at LFC84 clears IFLAG bit 0 and ZPOUT (Mos12.zsm:2661)
// waits on that bit before sending the next byte. Port B gets vector 12 and is
// left alone.
class Z80Pio {
public:
    enum Mode { kOutput = 0, kInput = 1, kBidirectional = 2, kBitControl = 3 };

    void reset();

    // reg: bit 0 selects control (1) over data (0), bit 1 selects port B.
    std::uint8_t read(int reg);
    void write(int reg, std::uint8_t value);

    // The attached peripheral pulsing /STB. In mode 0 this is the acknowledge
    // that says the byte was taken, and it is what raises the interrupt.
    void strobe(int port);

    // True once, when the CPU has written a new byte to that port's output
    // register. The byte is handed back in out.
    bool take_output(int port, std::uint8_t& out);

    // What an input port reads back.
    void set_input(int port, std::uint8_t value) { input_[port & 1] = value; }
    std::uint8_t input(int port) const { return input_[port & 1]; }

    // Daisy-chain interface, same shape as Z80Ctc.
    bool int_pending() const;
    bool int_in_service() const;
    std::uint8_t int_ack();
    void reti();

    std::uint8_t vector(int port) const { return vector_[port & 1]; }
    std::uint8_t mode(int port) const { return mode_[port & 1]; }
    std::uint8_t output(int port) const { return output_[port & 1]; }
    bool int_enabled(int port) const { return int_enabled_[port & 1]; }
    std::uint64_t strobes(int port) const { return strobes_[port & 1]; }

private:
    std::uint8_t vector_[2]{};
    std::uint8_t mode_[2]{kInput, kInput};  // a reset PIO is in input mode
    std::uint8_t io_mask_[2]{0xff, 0xff};
    std::uint8_t output_[2]{};
    std::uint8_t input_[2]{0xff, 0xff};
    bool int_enabled_[2]{};
    bool int_request_[2]{};
    bool in_service_[2]{};
    bool pending_written_[2]{};
    bool expect_io_mask_[2]{};
    bool expect_int_mask_[2]{};
    std::uint64_t strobes_[2]{};
};

}  // namespace ein
