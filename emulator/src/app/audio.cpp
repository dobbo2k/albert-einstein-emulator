// Albert -- audio output, which is also what paces the emulation.
//
// Split out of main.cpp at M10 phase 4.
#include "app.h"

namespace albert {

// -------------------------------------------------------------------- Audio

// Emulation speed is slaved to how fast the audio device consumes samples, not
// to a sleep or a fixed T-state budget. The PSG produces exactly 48000 samples
// per emulated second, so "keep the device's queue topped up" and "run at the
// right speed" are the same instruction -- and it self-corrects, because a slow
// frame leaves the queue short and the next frame runs longer to refill it.
void drain_psg_to_device(AppState& app) {
    float buf[2048];
    int n;
    while ((n = app.machine.psg.read_samples(buf, 2048)) > 0) {
        if (app.audio_ok && app.sound_on) {
            SDL_PutAudioStreamData(app.audio, buf, n * static_cast<int>(sizeof(float)));
        }
    }
}

int queued_samples(AppState& app) {
    if (!app.audio_ok) return 0;
    const int bytes = SDL_GetAudioStreamQueued(app.audio);
    return bytes < 0 ? 0 : bytes / static_cast<int>(sizeof(float));
}

// Runs the CPU until the audio queue is back up to the target depth. Returns
// the number of instructions executed.
long long run_for_audio(AppState& app) {
    const int rate = ein::Ay38910::kSampleRate;
    const int target = rate * kAudioTargetMs / 1000;
    const int ceiling = rate * kAudioMaxMs / 1000;

    int have = queued_samples(app) + app.machine.psg.samples_available();
    app.queued_ms = have * 1000 / rate;
    if (have >= ceiling) return 0;

    // Cap the work per frame so a stall cannot wedge the UI.
    const long long kMaxInstructions = 400000;
    long long done = 0;
    while (have < target && done < kMaxInstructions) {
        const auto r = app.machine.step();
        ++done;
        if (!r.ok) { app.running = false; break; }
        if ((done & 0x3ff) == 0) {
            drain_psg_to_device(app);
            have = queued_samples(app) + app.machine.psg.samples_available();
        }
    }
    drain_psg_to_device(app);
    return done;
}

}  // namespace albert
