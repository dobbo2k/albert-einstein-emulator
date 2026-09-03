#include "ay38910.h"

#include <algorithm>
#include <cmath>

#include "state.h"

namespace ein {
namespace {

// The AY's DAC is logarithmic, roughly 1.5 dB a step. These are the measured
// normalised levels the emulator community settled on for the AY-3-8910; the
// chip's own datasheet only gives the shape.
constexpr float kVolume[16] = {
    0.0000f, 0.0137f, 0.0205f, 0.0291f, 0.0423f, 0.0618f, 0.0847f, 0.1369f,
    0.1691f, 0.2647f, 0.3527f, 0.4499f, 0.5704f, 0.6873f, 0.8482f, 1.0000f,
};

// Three channels summed would clip, so leave headroom.
constexpr float kMixScale = 1.0f / 3.0f;

}  // namespace

void Ay38910::reset() {
    soft_reset();
    tstate_accum_ = 0;
    update_lowpass();
    lp1_ = lp2_ = 0.0f;
    sum_ = 0.0f;
    sum_count_ = 0;
    sample_accum_ = 0;
    ring_head_ = ring_tail_ = 0;
    samples_generated_ = samples_dropped_ = 0;
}

void Ay38910::soft_reset() {
    for (auto& r : regs_) r = 0;
    selected_ = 0;
    row_select_ = 0;
    for (int i = 0; i < 3; ++i) {
        tone_count_[i] = 0;
        tone_state_[i] = 0;
    }
    noise_count_ = 0;
    noise_lfsr_ = 1;
    noise_state_ = 0;
    env_count_ = 0;
    env_step_ = 0;
    env_level_ = 0;
    env_attack_ = 0;
    env_holding_ = false;
    half_ = false;
}

// The register file and the generators. The ring buffer and the sample
// counters are left out: audio regenerates within a millisecond of resuming.
void Ay38910::save_state(StateWriter& w) const {
    w.bytes(regs_, sizeof regs_);
    w.put(selected_);
    w.put(row_select_);
    w.bytes(tone_count_, sizeof tone_count_);
    w.bytes(tone_state_, sizeof tone_state_);
    w.put(noise_count_);
    w.put(noise_lfsr_);
    w.put(noise_state_);
    w.put(env_count_);
    w.put(env_step_);
    w.put(env_level_);
    w.put(env_attack_);
    w.put(env_holding_);
    w.put(half_);
    w.put(tstate_accum_);
    w.put(sample_accum_);
}

void Ay38910::load_state(StateReader& r) {
    r.bytes(regs_, sizeof regs_);
    r.get(selected_);
    r.get(row_select_);
    r.bytes(tone_count_, sizeof tone_count_);
    r.bytes(tone_state_, sizeof tone_state_);
    r.get(noise_count_);
    r.get(noise_lfsr_);
    r.get(noise_state_);
    r.get(env_count_);
    r.get(env_step_);
    r.get(env_level_);
    r.get(env_attack_);
    r.get(env_holding_);
    r.get(half_);
    r.get(tstate_accum_);
    r.get(sample_accum_);
    clear_samples();
}

void Ay38910::set_lowpass(float hz) {
    lowpass_hz_ = hz > 0.0f ? hz : 0.0f;
    update_lowpass();
    // Deliberately not clearing lp1_/lp2_: changing the cutoff from the menu
    // while a game is playing should slide the tone, not click.
}

// alpha for y += alpha * (x - y), the one-pole section whose corner is at the
// requested cutoff when sampled at kStepHz. Two of them in series means the
// cascade is -6 dB there rather than -3, so the audible knee sits a little
// under the number in the menu -- named for the section, not the pair, because
// that is the frequency the rolloff is anchored to. Clamped to a quarter of
// the step rate, well inside the range where the mapping still means anything.
void Ay38910::update_lowpass() {
    if (lowpass_hz_ <= 0.0f) {
        lp_alpha_ = 0.0f;
        return;
    }
    const float fc = std::min(lowpass_hz_, static_cast<float>(kStepHz) / 4.0f);
    constexpr float kTwoPi = 6.283185307f;
    lp_alpha_ = 1.0f - std::exp(-kTwoPi * fc / static_cast<float>(kStepHz));
}

int Ay38910::tone_period(int ch) const {
    const int i = (ch & 3) * 2;
    return (regs_[i] | ((regs_[i + 1] & 0x0f) << 8));
}

int Ay38910::channel_level(int ch) const {
    const std::uint8_t amp = regs_[8 + (ch & 3)];
    return (amp & 0x10) ? env_level_ : (amp & 0x0f);
}

void Ay38910::write(std::uint8_t v) {
    regs_[selected_] = v;
    if (selected_ == 14) {
        // Port A drives the keyboard rows active low; MOS writes ~row.
        row_select_ = static_cast<std::uint8_t>(~v);
    } else if (selected_ == 13) {
        // Writing the shape register always restarts the envelope, even if the
        // value is unchanged -- that is how music drivers retrigger it.
        restart_envelope();
    }
}

std::uint8_t Ay38910::read() const {
    if (selected_ == 15) {
        // Port B returns the columns for the selected rows, active low.
        const std::uint8_t cols = keyboard ? keyboard->columns(row_select_) : 0;
        return static_cast<std::uint8_t>(~cols);
    }
    return regs_[selected_];
}

void Ay38910::restart_envelope() {
    const std::uint8_t shape = regs_[13] & 0x0f;
    // The step counter always runs 0..15; ATT (bit 2) decides whether the level
    // follows it or its complement, so a mask of 0F counts down and 00 counts up.
    env_attack_ = (shape & 0x04) ? 0x00 : 0x0f;
    env_step_ = 0;
    env_holding_ = false;
    env_count_ = 0;
    env_level_ = 0 ^ env_attack_;
}

// One of the sixteen envelope steps. CONT (bit 3), ATT (bit 2), ALT (bit 1) and
// HOLD (bit 0) between them give the eight distinct shapes the datasheet draws;
// the eight with CONT clear all behave as a single pass followed by silence.
void Ay38910::envelope_step() {
    if (env_holding_) return;

    ++env_step_;
    if (env_step_ <= 15) {
        env_level_ = env_step_ ^ env_attack_;
        return;
    }

    const std::uint8_t shape = regs_[13] & 0x0f;
    if (!(shape & 0x08)) {
        // CONT = 0: one ramp, then hold at zero.
        env_level_ = 0;
        env_holding_ = true;
        return;
    }
    if (shape & 0x02) env_attack_ ^= 0x0f;  // ALT: mirror for the next pass
    if (shape & 0x01) {
        // HOLD: freeze at the far end of the (possibly mirrored) ramp. That
        // gives the datasheet's four held shapes -- 09 and 0F settle at 0,
        // 0B and 0D at 15.
        env_level_ = 15 ^ env_attack_;
        env_holding_ = true;
        return;
    }
    env_step_ = 0;
    env_level_ = 0 ^ env_attack_;
}

float Ay38910::mix() const {
    const std::uint8_t mixer = regs_[7];
    float out = 0.0f;
    for (int ch = 0; ch < 3; ++ch) {
        // Both enables are active low. A channel is on when its tone is high
        // (or disabled) and the noise is high (or disabled).
        const bool tone_on = tone_state_[ch] != 0 || (mixer & (1u << ch)) != 0;
        const bool noise_on = noise_state_ != 0 || (mixer & (1u << (ch + 3))) != 0;
        if (!(tone_on && noise_on)) continue;
        out += kVolume[channel_level(ch) & 0x0f];
    }
    return out * kMixScale;
}

void Ay38910::step() {
    // Tone counters run at clock/8 and toggle on reaching the period, giving
    // f = clock / (16 * period). A period of 0 behaves as 1.
    for (int ch = 0; ch < 3; ++ch) {
        int period = tone_period(ch);
        if (period == 0) period = 1;
        if (++tone_count_[ch] >= period) {
            tone_count_[ch] = 0;
            tone_state_[ch] ^= 1;
        }
    }

    // Noise and envelope are clocked at clock/16, half the tone rate.
    half_ = !half_;
    if (half_) {
        int np = noise_period();
        if (np == 0) np = 1;
        if (++noise_count_ >= np) {
            noise_count_ = 0;
            // 17-bit LFSR, taps at bits 0 and 3.
            const std::uint32_t bit = (noise_lfsr_ ^ (noise_lfsr_ >> 3)) & 1u;
            noise_lfsr_ = (noise_lfsr_ >> 1) | (bit << 16);
            noise_state_ = static_cast<int>(noise_lfsr_ & 1u);
        }

        int ep = envelope_period();
        if (ep == 0) ep = 1;
        if (++env_count_ >= ep) {
            env_count_ = 0;
            envelope_step();
        }
    }
}

void Ay38910::push(float s) {
    const int next = (ring_head_ + 1) % kRingSize;
    if (next == ring_tail_) {
        // Nobody is draining. Drop the oldest sample rather than the newest so
        // a paused frontend resumes on current audio instead of stale audio.
        ring_tail_ = (ring_tail_ + 1) % kRingSize;
        ++samples_dropped_;
    }
    ring_[ring_head_] = s;
    ring_head_ = next;
    ++samples_generated_;
}

void Ay38910::tick(int tstates) {
    if (!audio_enabled_) return;

    tstate_accum_ += tstates;
    while (tstate_accum_ >= kTStatesPerStep) {
        tstate_accum_ -= kTStatesPerStep;
        step();

        // The speaker/anti-alias low-pass runs here, at the step rate, so
        // anything it removes never reaches the decimator to fold down.
        float s = mix();
        if (lp_alpha_ > 0.0f) {
            lp1_ += lp_alpha_ * (s - lp1_);
            lp2_ += lp_alpha_ * (lp1_ - lp2_);
            s = lp2_;
        }

        // Box filter down from the 250 kHz step rate to the output rate.
        sum_ += s;
        ++sum_count_;
        sample_accum_ += kSampleRate;
        if (sample_accum_ >= kStepHz) {
            sample_accum_ -= kStepHz;
            push(sum_count_ > 0 ? sum_ / static_cast<float>(sum_count_) : 0.0f);
            sum_ = 0.0f;
            sum_count_ = 0;
        }
    }
}

int Ay38910::samples_available() const {
    return (ring_head_ - ring_tail_ + kRingSize) % kRingSize;
}

int Ay38910::read_samples(float* dst, int count) {
    const int have = samples_available();
    const int n = std::min(count, have);
    for (int i = 0; i < n; ++i) {
        dst[i] = ring_[ring_tail_];
        ring_tail_ = (ring_tail_ + 1) % kRingSize;
    }
    return n;
}

void Ay38910::clear_samples() {
    ring_tail_ = ring_head_;
    sum_ = 0.0f;
    sum_count_ = 0;
}

}  // namespace ein
