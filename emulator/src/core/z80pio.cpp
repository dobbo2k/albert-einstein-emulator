#include "z80pio.h"

namespace ein {

void Z80Pio::reset() {
    for (int p = 0; p < 2; ++p) {
        vector_[p] = 0;
        mode_[p] = kInput;
        io_mask_[p] = 0xff;
        output_[p] = 0;
        input_[p] = 0xff;
        int_enabled_[p] = false;
        int_request_[p] = false;
        in_service_[p] = false;
        pending_written_[p] = false;
        expect_io_mask_[p] = false;
        expect_int_mask_[p] = false;
        strobes_[p] = 0;
    }
}

std::uint8_t Z80Pio::read(int reg) {
    const int port = (reg >> 1) & 1;
    if (reg & 1) {
        // The control registers are write-only on a real PIO.
        return 0xff;
    }
    if (mode_[port] == kOutput) return output_[port];
    return input_[port];
}

void Z80Pio::write(int reg, std::uint8_t value) {
    const int port = (reg >> 1) & 1;
    if (!(reg & 1)) {
        output_[port] = value;
        // In mode 0 writing the data register asserts READY; the peripheral
        // answers with /STB, which is what raises the interrupt.
        pending_written_[port] = true;
        return;
    }

    // Control port. A byte expected as a follow-up takes precedence.
    if (expect_io_mask_[port]) {
        expect_io_mask_[port] = false;
        io_mask_[port] = value;
        return;
    }
    if (expect_int_mask_[port]) {
        expect_int_mask_[port] = false;
        return;  // bit-control masking is not used by anything on this machine
    }

    if ((value & 0x01) == 0) {
        // Interrupt vector. Bit 0 of the vector itself is forced to 0 by the
        // chip when it puts it on the bus.
        vector_[port] = static_cast<std::uint8_t>(value & 0xfe);
        return;
    }
    switch (value & 0x0f) {
        case 0x0f:  // mode set
            mode_[port] = static_cast<std::uint8_t>((value >> 6) & 3);
            expect_io_mask_[port] = (mode_[port] == kBitControl);
            // Leaving output mode drops any half-finished handshake.
            pending_written_[port] = false;
            return;
        case 0x07:  // interrupt control
            int_enabled_[port] = (value & 0x80) != 0;
            expect_int_mask_[port] = (value & 0x10) != 0;
            if (!int_enabled_[port]) int_request_[port] = false;
            return;
        case 0x03:  // interrupt enable / disable
            int_enabled_[port] = (value & 0x80) != 0;
            if (!int_enabled_[port]) int_request_[port] = false;
            return;
        default:
            return;
    }
}

bool Z80Pio::take_output(int port, std::uint8_t& out) {
    const int p = port & 1;
    if (!pending_written_[p]) return false;
    pending_written_[p] = false;
    out = output_[p];
    return true;
}

void Z80Pio::strobe(int port) {
    const int p = port & 1;
    ++strobes_[p];
    if (int_enabled_[p]) int_request_[p] = true;
}

bool Z80Pio::int_pending() const {
    for (int p = 0; p < 2; ++p) {
        if (in_service_[p]) return false;  // a higher port is still being served
        if (int_request_[p]) return true;
    }
    return false;
}

bool Z80Pio::int_in_service() const {
    return in_service_[0] || in_service_[1];
}

std::uint8_t Z80Pio::int_ack() {
    for (int p = 0; p < 2; ++p) {
        if (in_service_[p]) break;
        if (int_request_[p]) {
            int_request_[p] = false;
            in_service_[p] = true;
            return vector_[p];
        }
    }
    return vector_[0];
}

void Z80Pio::reti() {
    // Port A outranks port B inside the chip.
    for (int p = 0; p < 2; ++p) {
        if (in_service_[p]) {
            in_service_[p] = false;
            return;
        }
    }
}

}  // namespace ein
