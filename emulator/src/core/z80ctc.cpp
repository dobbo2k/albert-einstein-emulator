#include "z80ctc.h"

namespace ein {

void Z80Ctc::reset() {
    for (auto& c : ch_) c = Channel{};
    vector_ = 0;
    ext_accum_ = 0;
    ext_needed_ = false;
}

void Z80Ctc::load(Channel& c, std::uint8_t tc) {
    c.tc = tc;
    c.counter = tc;  // 0 means 256
    c.prescale = 0;
    // In timer mode with bit 3 set the channel waits for a CLK/TRG edge before
    // it starts; otherwise loading the time constant starts it immediately.
    c.running = c.counter_mode() || !c.needs_trigger();
}

void Z80Ctc::write(int channel, std::uint8_t v) {
    Channel& c = ch_[channel & 3];

    if (c.expecting_tc) {
        c.expecting_tc = false;
        load(c, v);
        refresh_ext_need();
        return;
    }

    if (v & 0x01) {
        // Control word.
        c.control = v;
        if (v & 0x02) {  // software reset
            c.running = false;
            c.int_request = false;
        }
        c.expecting_tc = (v & 0x04) != 0;
        if (!c.expecting_tc && !(v & 0x02)) {
            c.running = c.counter_mode() || !c.needs_trigger();
        }
        refresh_ext_need();
        return;
    }

    // Interrupt vector. Only channel 0 carries it; channel n uses base | n*2.
    if ((channel & 3) == 0) vector_ = static_cast<std::uint8_t>(v & 0xf8);
}

std::uint8_t Z80Ctc::read(int channel) const {
    return ch_[channel & 3].counter;
}

void Z80Ctc::down(int index) {
    Channel& c = ch_[index];
    if (--c.counter != 0) return;

    c.counter = c.tc;  // 0 reloads as 256 via natural wrap
    ++c.zero_counts;
    if (c.int_enabled()) c.int_request = true;

    // ZC/TO of channel 2 drives the CLK/TRG of channel 3.
    if (index == 2) trigger(3);
}

void Z80Ctc::trigger(int channel) {
    Channel& c = ch_[channel & 3];
    if (c.counter_mode()) {
        if (!c.running) return;
        down(channel & 3);
        return;
    }
    // Timer mode: an edge starts a channel that was waiting for one.
    if (!c.running) {
        c.running = true;
        c.prescale = 0;
    }
}

void Z80Ctc::refresh_ext_need() {
    // Channels 0-2 take the external 2 MHz CLK/TRG. They only need it when
    // counting, or when a timer is still waiting for its start edge.
    ext_needed_ = false;
    for (int i = 0; i < 3; ++i) {
        const Channel& c = ch_[i];
        if (c.counter_mode() || (!c.running && c.needs_trigger())) ext_needed_ = true;
    }
}

void Z80Ctc::tick(int tstates) {
    // External CLK/TRG edges. Skipped entirely when nothing is listening, which
    // is the common case -- this runs on every bus access.
    if (ext_needed_) {
        ext_accum_ += tstates;
        while (ext_accum_ >= kExtClockDivisor) {
            ext_accum_ -= kExtClockDivisor;
            for (int i = 0; i < 3; ++i) trigger(i);
        }
        refresh_ext_need();
    }

    for (int i = 0; i < kChannels; ++i) {
        Channel& c = ch_[i];
        if (!c.running || c.counter_mode()) continue;
        c.prescale += tstates;
        const int div = c.divisor();
        while (c.prescale >= div) {
            c.prescale -= div;
            down(i);
        }
    }
}

bool Z80Ctc::int_pending() const {
    for (const auto& c : ch_) {
        if (c.in_service) return false;  // higher priority still being served
        if (c.int_request) return true;
    }
    return false;
}

bool Z80Ctc::int_in_service() const {
    for (const auto& c : ch_) {
        if (c.in_service) return true;
    }
    return false;
}

std::uint8_t Z80Ctc::int_ack() {
    for (int i = 0; i < kChannels; ++i) {
        Channel& c = ch_[i];
        if (c.in_service) break;
        if (c.int_request) {
            c.int_request = false;
            c.in_service = true;
            return static_cast<std::uint8_t>(vector_ | (i << 1));
        }
    }
    return vector_;
}

void Z80Ctc::reti() {
    // Clear the highest-priority channel currently in service.
    for (auto& c : ch_) {
        if (c.in_service) {
            c.in_service = false;
            return;
        }
    }
}

}  // namespace ein
