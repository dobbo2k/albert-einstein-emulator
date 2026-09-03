#pragma once

#include <cstdint>
#include <deque>

namespace ein {

// Intel 8251A programmable communication interface at ports 10-17
// (Einstein.zsm:20): port 10 is data, port 11 is command (write) and status
// (read). Bit layouts are at Einstein.zsm:471-487.
//
// MOS initialises it at reset (Mos12.zsm:293) to mode CE, command 27:
//
//   mode CE = 1100 1110 -> x16 baud factor, 8 data bits, no parity, 2 stop bits
//   cmd  27 = 0010 0111 -> transmit enable, DTR, receive enable, RTS
//
// The transmit and receive clocks come from CTC channels 0 and 1, which BAUD
// (Mos12.zsm:636) programmes from the table at Mos12.zsm:686. That table's own
// arithmetic is worth reading, because it pins down two clocks at once:
//
//   75-600 baud   timer mode    BAUD = 1/(COUNT * 256 * 250ns)
//   1200-9600     counter mode  BAUD = 1/(2 * COUNT * 16 * 250ns)
//
// 250 ns is the 4 MHz CPU period. In timer mode the 256 is the CTC's own /16
// prescaler times this chip's x16 factor. In counter mode there is no
// prescaler, so the 2 is the divider between the 4 MHz CPU clock and the 2 MHz
// the CTC's CLK/TRG inputs actually run at -- the same 2 MHz derived
// independently in z80ctc.h.
//
// So the baud rate is not configured here at all: it falls out of the CTC, and
// this chip simply divides its clock input by the mode word's factor.
class I8251 {
public:
    // Status bits, Einstein.zsm:471-487.
    enum Status : std::uint8_t {
        kTxRdy = 0x01,
        kRxRdy = 0x02,
        kTxEmpty = 0x04,
        kParityError = 0x08,
        kOverrunError = 0x10,
        kFramingError = 0x20,
        kSynDet = 0x40,
        kDsr = 0x80,
    };

    void reset();

    void save_state(class StateWriter& w) const;
    void load_state(class StateReader& r);

    std::uint8_t read(int reg);          // reg 0 data, 1 status
    void write(int reg, std::uint8_t v);  // reg 0 data, 1 mode/command

    // One edge of the transmit or receive clock, from the CTC.
    void tx_clock();
    void rx_clock();

    // Loopback ties TxD to RxD, which is how the serial path is tested without
    // anything plugged in.
    bool loopback = false;

    // Bytes the port has sent, and bytes waiting to be received.
    const std::deque<std::uint8_t>& sent() const { return sent_; }
    void clear_sent() { sent_.clear(); }
    void receive(std::uint8_t byte) { rx_queue_.push_back(byte); }

    std::uint8_t status() const;
    std::uint8_t mode() const { return mode_; }
    std::uint8_t command() const { return command_; }
    int clock_divisor() const;
    int data_bits() const;
    std::uint64_t tx_clocks() const { return tx_clocks_; }
    std::uint64_t rx_clocks() const { return rx_clocks_; }

private:
    // A reset 8251 wants a mode word first, then command words after that.
    bool expect_mode_ = true;
    std::uint8_t mode_ = 0;
    std::uint8_t command_ = 0;

    std::uint8_t tx_holding_ = 0;
    bool tx_holding_full_ = false;
    bool tx_shifting_ = false;
    std::uint8_t tx_shift_ = 0;
    int tx_count_ = 0;  // clock edges left in the character being shifted out

    std::uint8_t rx_data_ = 0;
    bool rx_ready_ = false;
    bool rx_overrun_ = false;
    int rx_count_ = 0;
    bool rx_shifting_ = false;
    std::uint8_t rx_shift_ = 0;

    std::deque<std::uint8_t> sent_;
    std::deque<std::uint8_t> rx_queue_;

    std::uint64_t tx_clocks_ = 0;
    std::uint64_t rx_clocks_ = 0;

    int character_edges() const;
    void deliver(std::uint8_t byte);
};

}  // namespace ein
