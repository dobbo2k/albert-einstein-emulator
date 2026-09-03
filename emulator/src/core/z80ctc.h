#pragma once

#include <cstdint>

namespace ein {

// Z80 CTC -- four counter/timer channels at ports 28-2B (Einstein.zsm:35).
//
// Channel 2's ZC/TO output is wired to channel 3's CLK/TRG input, which is how
// MOS builds its one-second real-time clock: ZZTIME (Mos12.zsm:2717) programs
// channel 2 as a timer with prescaler 256 and time constant 125, giving
// 4000000/256/125 = 125 Hz, then channel 3 as a counter with time constant 125,
// dividing that to exactly 1 Hz.
class Z80Ctc {
public:
    static constexpr int kChannels = 4;

    // The CLK/TRG inputs of channels 0-2 are driven at 2 MHz, i.e. one edge
    // every two T-states. Derived from the baud table at Mos12.zsm:686-697:
    // the 9600 entry is counter mode with time constant 13, and MOS puts the
    // 8251 in x16 mode (mode byte CE at Mos12.zsm:293), so
    // 9615.38 = F / (16 * 13) gives F = 2 MHz. Every other counter-mode entry
    // agrees: 1200 -> 2e6/104/16 = 1201.92, 2400 -> 2403.85, 4800 -> 4807.69.
    //
    // This is what starts channel 2, whose control word 3F asks for an
    // external start edge -- it arrives within two T-states of the time
    // constant being loaded.
    static constexpr int kExtClockDivisor = 2;

    void reset();

    void write(int channel, std::uint8_t v);
    std::uint8_t read(int channel) const;

    void tick(int tstates);
    void trigger(int channel);  // external CLK/TRG edge

    // Daisy-chain interface.
    bool int_pending() const;
    // True while any channel is being serviced. A device in service holds IEO
    // low, which is what blocks every device below it in the chain.
    bool int_in_service() const;
    std::uint8_t int_ack();
    void reti();

    std::uint8_t vector() const { return vector_; }
    std::uint8_t control(int c) const { return ch_[c & 3].control; }
    std::uint8_t time_constant(int c) const { return ch_[c & 3].tc; }
    std::uint8_t counter(int c) const { return ch_[c & 3].counter; }
    std::uint64_t zero_counts(int c) const { return ch_[c & 3].zero_counts; }

private:
    struct Channel {
        std::uint8_t control = 0;
        std::uint8_t tc = 0;
        std::uint8_t counter = 0;
        int prescale = 0;
        bool expecting_tc = false;
        bool running = false;
        bool int_request = false;
        bool in_service = false;
        std::uint64_t zero_counts = 0;

        bool counter_mode() const { return (control & 0x40) != 0; }
        int divisor() const { return (control & 0x20) ? 256 : 16; }
        bool int_enabled() const { return (control & 0x80) != 0; }
        bool needs_trigger() const { return (control & 0x08) != 0; }
    };

    Channel ch_[kChannels];
    std::uint8_t vector_ = 0;
    int ext_accum_ = 0;
    bool ext_needed_ = false;

    void load(Channel& c, std::uint8_t tc);
    void down(int index);
    void refresh_ext_need();
};

}  // namespace ein
