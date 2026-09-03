// M6 acceptance: ten emulated minutes of a real game's music with the audio
// drained the way the frontend drains it, checking that the sample rate stays
// exact and that nothing is ever dropped.
//
// Underruns on real hardware also depend on the host audio device, which a
// headless run cannot exercise. What this does prove is the half that belongs
// to the emulator: the PSG produces exactly kSampleRate samples per emulated
// second, indefinitely, with no drift.
//
// Usage: soak_audio <rom> <disk> [minutes]
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

#include "ay38910.h"
#include "machine.h"
#include "rom.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: soak_audio <rom> <disk> [minutes]\n");
        return 2;
    }
    const std::filesystem::path rom_path = argv[1];
    const std::filesystem::path disk_path = argv[2];
    const int minutes = (argc > 3) ? std::atoi(argv[3]) : 10;

    if (!std::filesystem::exists(disk_path)) {
        std::printf("SKIP: %s not present\n", disk_path.string().c_str());
        return 0;
    }

    ein::Machine m;
    try {
        m.set_rom(ein::load_rom(rom_path));
        m.reset();
        m.fdc.disk(0).load(disk_path);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "setup failed: %s\n", e.what());
        return 1;
    }

    const std::uint64_t seconds = static_cast<std::uint64_t>(minutes) * 60;
    const std::uint64_t limit = seconds * ein::Machine::kCpuClockHz;

    std::uint64_t drained = 0;
    std::uint64_t silent_runs = 0;
    std::uint64_t longest_silence = 0;
    float peak = 0.0f;
    float buf[2048];

    // Worst per-second shortfall: the frontend asks for a slice of audio each
    // frame, so a rate that sagged even briefly would show up here.
    long long worst_second = 0;
    std::uint64_t next_mark = ein::Machine::kCpuClockHz;
    std::uint64_t at_mark = 0;
    std::uint64_t elapsed_seconds = 0;

    while (m.tstates < limit && !m.cpu.halted) {
        m.step();
        int n;
        while ((n = m.psg.read_samples(buf, 2048)) > 0) {
            for (int i = 0; i < n; ++i) {
                const float a = std::abs(buf[i]);
                if (a > peak) peak = a;
                if (a < 1e-4f) {
                    ++silent_runs;
                    if (silent_runs > longest_silence) longest_silence = silent_runs;
                } else {
                    silent_runs = 0;
                }
            }
            drained += n;
        }
        if (m.tstates >= next_mark) {
            const long long got = static_cast<long long>(drained - at_mark);
            const long long err = got - ein::Ay38910::kSampleRate;
            if (std::llabs(err) > std::llabs(worst_second)) worst_second = err;
            at_mark = drained;
            next_mark += ein::Machine::kCpuClockHz;
            ++elapsed_seconds;
        }
    }

    const std::uint64_t want = seconds * ein::Ay38910::kSampleRate;
    const long long err = static_cast<long long>(drained) - static_cast<long long>(want);
    std::printf("emulated     %llu s (%d minutes)\n", (unsigned long long)elapsed_seconds, minutes);
    std::printf("samples      %llu, expected %llu (%+lld)\n", (unsigned long long)drained,
                (unsigned long long)want, err);
    std::printf("worst second %+lld samples\n", worst_second);
    std::printf("dropped      %llu\n", (unsigned long long)m.psg.samples_dropped());
    std::printf("peak         %.3f\n", peak);
    std::printf("longest run of silence %.2f s\n",
                double(longest_silence) / ein::Ay38910::kSampleRate);

    bool ok = true;
    // One sample of slack per second for the loop boundary.
    if (std::llabs(err) > static_cast<long long>(seconds)) {
        std::printf("FAIL: sample rate drifted\n");
        ok = false;
    }
    if (std::llabs(worst_second) > 2) {
        std::printf("FAIL: a one-second window was short\n");
        ok = false;
    }
    if (m.psg.samples_dropped() != 0) {
        std::printf("FAIL: samples were dropped despite continuous draining\n");
        ok = false;
    }
    if (peak <= 0.01f) {
        std::printf("FAIL: nothing audible\n");
        ok = false;
    }
    std::printf("RESULT: %s\n", ok ? "PASSED" : "FAILED");
    return ok ? 0 : 1;
}
