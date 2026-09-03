#include "i8251.h"

#include "state.h"

namespace ein {

void I8251::reset() {
    expect_mode_ = true;
    mode_ = 0;
    command_ = 0;
    tx_holding_ = 0;
    tx_holding_full_ = false;
    tx_shifting_ = false;
    tx_shift_ = 0;
    tx_count_ = 0;
    rx_data_ = 0;
    rx_ready_ = false;
    rx_overrun_ = false;
    rx_count_ = 0;
    rx_shifting_ = false;
    rx_shift_ = 0;
    sent_.clear();
    rx_queue_.clear();
    tx_clocks_ = rx_clocks_ = 0;
}

// Bytes still on the wire are not saved: a state restored mid-character
// simply starts the next one cleanly.
void I8251::save_state(StateWriter& w) const {
    w.put(expect_mode_);
    w.put(mode_);
    w.put(command_);
    w.put(tx_holding_);
    w.put(tx_holding_full_);
    w.put(tx_shifting_);
    w.put(tx_shift_);
    w.put(tx_count_);
    w.put(rx_data_);
    w.put(rx_ready_);
    w.put(rx_overrun_);
    w.put(rx_count_);
    w.put(rx_shifting_);
    w.put(rx_shift_);
}

void I8251::load_state(StateReader& r) {
    r.get(expect_mode_);
    r.get(mode_);
    r.get(command_);
    r.get(tx_holding_);
    r.get(tx_holding_full_);
    r.get(tx_shifting_);
    r.get(tx_shift_);
    r.get(tx_count_);
    r.get(rx_data_);
    r.get(rx_ready_);
    r.get(rx_overrun_);
    r.get(rx_count_);
    r.get(rx_shifting_);
    r.get(rx_shift_);
    sent_.clear();
    rx_queue_.clear();
}

// Mode word bits 0-1: 00 synchronous, 01 x1, 10 x16, 11 x64.
int I8251::clock_divisor() const {
    switch (mode_ & 0x03) {
        case 0: return 1;  // synchronous; the clock is the bit rate
        case 1: return 1;
        case 2: return 16;
        default: return 64;
    }
}

// Mode word bits 2-3: 00 5 bits ... 11 8 bits.
int I8251::data_bits() const {
    return 5 + ((mode_ >> 2) & 0x03);
}

// A whole character on the wire: start bit, data, optional parity, stop bits,
// all counted in clock edges.
int I8251::character_edges() const {
    int bits = 1 + data_bits();
    if (mode_ & 0x10) ++bits;  // parity enabled
    const int stop = (mode_ >> 6) & 3;
    // 00 is invalid, 01 = 1 stop bit, 10 = 1.5, 11 = 2. Round 1.5 up; nothing
    // on this machine measures the half bit.
    bits += (stop == 0) ? 1 : (stop == 3 ? 2 : 1);
    return bits * clock_divisor();
}

std::uint8_t I8251::status() const {
    std::uint8_t s = kDsr;  // nothing models a modem, so DSR is always asserted
    if (!tx_holding_full_) s |= kTxRdy;
    if (!tx_holding_full_ && !tx_shifting_) s |= kTxEmpty;
    if (rx_ready_) s |= kRxRdy;
    if (rx_overrun_) s |= kOverrunError;
    return s;
}

std::uint8_t I8251::read(int reg) {
    if (reg & 1) return status();
    rx_ready_ = false;
    return rx_data_;
}

void I8251::write(int reg, std::uint8_t v) {
    if (!(reg & 1)) {
        tx_holding_ = v;
        tx_holding_full_ = true;
        return;
    }

    if (expect_mode_) {
        mode_ = v;
        expect_mode_ = false;
        return;
    }

    command_ = v;
    if (v & 0x10) {  // ER: clear the error flags
        rx_overrun_ = false;
    }
    if (v & 0x40) {  // IR: internal reset, back to expecting a mode word
        const bool keep_loopback = loopback;
        reset();
        loopback = keep_loopback;
        return;
    }
    if (!(v & 0x04)) {  // receive disabled
        rx_ready_ = false;
    }
}

void I8251::deliver(std::uint8_t byte) {
    sent_.push_back(byte);
    if (loopback) rx_queue_.push_back(byte);
}

void I8251::tx_clock() {
    ++tx_clocks_;
    if (!(command_ & 0x01)) return;  // TxEN clear

    if (tx_shifting_) {
        if (--tx_count_ > 0) return;
        tx_shifting_ = false;
        deliver(tx_shift_);
    }
    if (!tx_shifting_ && tx_holding_full_) {
        tx_shift_ = tx_holding_;
        tx_holding_full_ = false;
        tx_shifting_ = true;
        tx_count_ = character_edges();
    }
}

void I8251::rx_clock() {
    ++rx_clocks_;
    if (!(command_ & 0x04)) return;  // RxE clear

    if (rx_shifting_) {
        if (--rx_count_ > 0) return;
        rx_shifting_ = false;
        if (rx_ready_) {
            rx_overrun_ = true;  // the CPU never collected the last one
        }
        rx_data_ = rx_shift_;
        rx_ready_ = true;
    }
    if (!rx_shifting_ && !rx_queue_.empty()) {
        rx_shift_ = rx_queue_.front();
        rx_queue_.pop_front();
        rx_shifting_ = true;
        rx_count_ = character_edges();
    }
}

}  // namespace ein
