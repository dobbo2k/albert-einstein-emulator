#pragma once

#include <cstdint>
#include <vector>

#include "keyboard.h"

namespace ein {

// AY-3-8910 programmable sound generator at ports 00-07 (Einstein.zsm:10-13):
//
//   OUT (02),n  latch register number
//   OUT (03),v  write the latched register
//   IN  (02)    read the latched register
//
// Three tone channels, a noise generator, one envelope generator and two I/O
// ports. Port A is the keyboard row select and port B returns the columns, both
// active low; ZPINIT sets R7 to 7F, making port A output and port B input
// (Mos12.zsm:2698).
//
// **Clock: 2 MHz.** Derived from the firmware rather than assumed. CTRLG, the
// BEL handler at Mos12.zsm:2679, plays one tone with everything else muted:
// tone period 142 (`LD BC,8E00H` then `LD BC,0001H`), amplitude 10, mixer 7E.
// A tone is f = clock / (16 * period), so:
//
//   1.7897725 MHz (Spectrum/CPC family)  ->  787.75 Hz, 0.48% flat of G5
//   1 MHz                                ->  440.14 Hz, A4 to within 0.03%
//   2 MHz                                ->  880.28 Hz, A5 to within 0.03%
//   4 MHz                                -> 1760.56 Hz, A6 to within 0.03%
//
// Landing that exactly on a note means the period was computed for one, which
// rules out the 1.7897725 MHz crystal this chip usually runs on and says the
// clock is a whole number of megahertz. 4 MHz is past the part's rating, so it
// is 1 or 2 MHz -- and those are an octave apart, which the beep alone cannot
// separate. 2 MHz is the choice because this machine is already known to
// distribute 2 MHz: the CTC's CLK/TRG inputs run at it, derived independently
// from the baud table at Mos12.zsm:686. Feeding the PSG from the same line
// needs no extra divider. If it ever proves to be 1 MHz, everything transposes
// down an octave and only kClockHz changes.
class Ay38910 {
public:
    static constexpr int kClockHz = 2'000'000;
    static constexpr int kCpuClockHz = 4'000'000;

    // The tone counters are clocked at clock/8, and toggle their output on
    // reaching the period -- two toggles to a cycle, so f = clock/(16*period),
    // the datasheet's formula. That is one step per 16 CPU T-states.
    static constexpr int kTStatesPerStep = kCpuClockHz / (kClockHz / 8);  // 16
    static constexpr int kStepHz = kCpuClockHz / kTStatesPerStep;         // 250 kHz

    static constexpr int kSampleRate = 48000;
    static constexpr int kRingSize = 1 << 15;  // ~0.68 s of slack

    // Output low-pass. A bare AY is a stack of square waves with no filtering
    // at all, and games that park a tone channel at a short period -- or lean
    // on the noise generator -- put energy well above anything the machine
    // could actually reproduce. The Einstein drives a small internal speaker
    // through an amplifier, which rolls that off; without it the emulator
    // whistles in a way the hardware does not.
    //
    // The filter sits at the 250 kHz step rate, *before* the box filter
    // decimates to kSampleRate, so it does double duty: it stands in for the
    // speaker's response, and it is a proper anti-alias stage. After the
    // decimation it could only attenuate a whistle that had already folded
    // down to the wrong frequency; here it removes it before the fold.
    //
    // 16 kHz is the default: the light setting, and mostly the anti-alias
    // half of the job. It leaves the whole audible band alone -- every
    // fundamental the AY can play, and the harmonics that give a square wave
    // its edge -- and takes out what was folding down from above. A whistle
    // that really is at 15 kHz is inside that and survives; the shorter
    // cutoffs in the menu are there for those, at the cost of some of the
    // brightness. The AY's musical range tops out around 2-4 kHz of
    // fundamental (a tone period of 32 is 3.9 kHz), so there is a lot of room
    // between the notes and this corner.
    static constexpr float kDefaultLowpassHz = 16000.0f;

    // 0 or less turns the filter off. Survives reset(), because it is a
    // preference and not chip state.
    void set_lowpass(float hz);
    float lowpass_hz() const { return lowpass_hz_; }

    // Power-on: the chip, the resampler phase, the queued audio and the
    // statistics.
    void reset();

    void save_state(class StateWriter& w) const;
    void load_state(class StateReader& r);

    // The port 00 strobe (Mos12.zsm:8) resets the FDC and the PSG. That clears
    // the chip, but it must NOT discard audio that has already been generated
    // or restart the resampler mid-sample -- ZFDRST pulses the strobe 32 times
    // in a row (Mos12.zsm:4239), and throwing a fraction of a sample away each
    // time punches an audible hole in the output.
    void soft_reset();

    void select(std::uint8_t reg) { selected_ = static_cast<std::uint8_t>(reg & 0x0f); }
    void write(std::uint8_t v);
    std::uint8_t read() const;

    // Advances the chip by n CPU T-states, generating samples as it goes.
    void tick(int tstates);

    // Sound generation costs time that a headless CPU exerciser has no use for.
    void set_audio_enabled(bool on) { audio_enabled_ = on; }
    bool audio_enabled() const { return audio_enabled_; }

    // Drains up to count samples into dst; returns how many were written.
    // Mono, -1.0 to 1.0.
    int read_samples(float* dst, int count);
    int samples_available() const;
    void clear_samples();

    std::uint8_t reg(int i) const { return regs_[i & 0x0f]; }
    std::uint8_t selected() const { return selected_; }
    std::uint8_t row_select() const { return row_select_; }

    // Debugger/test accessors.
    int tone_period(int ch) const;
    int noise_period() const { return regs_[6] & 0x1f; }
    int envelope_period() const { return regs_[11] | (regs_[12] << 8); }
    int envelope_level() const { return env_level_; }
    int channel_level(int ch) const;
    std::uint64_t samples_generated() const { return samples_generated_; }
    std::uint64_t samples_dropped() const { return samples_dropped_; }

    Keyboard* keyboard = nullptr;

private:
    std::uint8_t regs_[16]{};
    std::uint8_t selected_ = 0;
    std::uint8_t row_select_ = 0;

    bool audio_enabled_ = true;

    // Tone generators.
    int tone_count_[3]{};
    int tone_state_[3]{};

    // Noise: a 17-bit LFSR clocked at clock/16.
    int noise_count_ = 0;
    std::uint32_t noise_lfsr_ = 1;
    int noise_state_ = 0;

    // Envelope: clock/16 as well, sixteen levels.
    int env_count_ = 0;
    int env_step_ = 0;
    int env_level_ = 0;
    std::uint8_t env_attack_ = 0;  // XOR mask: 00 counts up, 0F counts down
    bool env_holding_ = false;

    bool half_ = false;  // every other step gives the clock/16 tick

    int tstate_accum_ = 0;

    // Two cascaded one-pole sections, 12 dB/octave, run at the step rate. One
    // pole gentle enough to leave the music alone barely dents a 15 kHz
    // whistle; two take it down 24 dB an octave and still cost two
    // multiply-adds a step.
    float lowpass_hz_ = kDefaultLowpassHz;
    float lp_alpha_ = 0.0f;  // derived from lowpass_hz_; 0 bypasses
    float lp1_ = 0.0f;
    float lp2_ = 0.0f;

    // Box filter from the 250 kHz step rate down to kSampleRate.
    float sum_ = 0.0f;
    int sum_count_ = 0;
    int sample_accum_ = 0;

    std::vector<float> ring_ = std::vector<float>(kRingSize, 0.0f);
    int ring_head_ = 0;  // next write
    int ring_tail_ = 0;  // next read
    std::uint64_t samples_generated_ = 0;
    std::uint64_t samples_dropped_ = 0;

    void step();
    void update_lowpass();
    void envelope_step();
    void restart_envelope();
    float mix() const;
    void push(float s);
};

}  // namespace ein
