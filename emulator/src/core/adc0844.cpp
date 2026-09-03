#include "adc0844.h"

namespace ein {

void Adc0844::reset() {
    for (auto& a : axis_) a = 128;
    result_ = 0;
    channel_ = 0;
    timer_ = 0;
    done_ = false;
}

void Adc0844::write(std::uint8_t value) {
    // The low nibble is the multiplexer address; only the four single-ended
    // channels are wired to anything here.
    channel_ = value & 0x03;
    timer_ = kConversionTStates;
}

std::uint8_t Adc0844::read() {
    return result_;
}

void Adc0844::tick(int tstates) {
    if (timer_ <= 0) return;
    timer_ -= tstates;
    if (timer_ > 0) return;
    timer_ = 0;
    result_ = axis_[channel_];
    done_ = true;
}

bool Adc0844::take_conversion_done() {
    const bool d = done_;
    done_ = false;
    return d;
}

}  // namespace ein
