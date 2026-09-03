#pragma once

#include <cstdint>

namespace ein {

// ADC0844 8-bit analogue to digital converter at ports 38-3F
// (Einstein.zsm:37), used for the analogue joysticks. Its interrupt mask is
// port 21 and its vector is 0A (Einstein.zsm:30, 149).
//
// Writing selects a multiplexer channel and starts a conversion; reading
// returns the last result. The part quotes about 40 us a conversion, which is
// modelled because a game that polls the status will otherwise see the answer
// arrive impossibly fast.
//
// Nothing in MOS reads it -- the reset code only sets the interrupt mask
// (Mos12.zsm:2388) -- so the register-level behaviour here follows the
// datasheet rather than firmware, and it is documented as such. Games drive it
// directly.
class Adc0844 {
public:
    static constexpr int kChannels = 4;
    // 40 us at 4 MHz.
    static constexpr int kConversionTStates = 160;

    void reset();
    void tick(int tstates);

    std::uint8_t read();
    void write(std::uint8_t value);

    // Axis values as the host sets them, 0-255 with 128 at rest.
    void set_axis(int channel, std::uint8_t value) { axis_[channel & 3] = value; }
    std::uint8_t axis(int channel) const { return axis_[channel & 3]; }

    bool converting() const { return timer_ > 0; }
    int selected_channel() const { return channel_; }
    std::uint8_t result() const { return result_; }

    // Set when a conversion completes; the machine turns this into an
    // interrupt if port 21 has unmasked it.
    bool take_conversion_done();

private:
    std::uint8_t axis_[kChannels]{128, 128, 128, 128};
    std::uint8_t result_ = 0;
    int channel_ = 0;
    int timer_ = 0;
    bool done_ = false;
};

}  // namespace ein
