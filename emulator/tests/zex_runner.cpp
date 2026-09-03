// Runs the zexdoc / zexall Z80 exercisers against the core.
//
// Usage: zex_runner <path-to-zexdoc-or-zexall.com>
//
// The exercisers are CP/M .COM programs: loaded at 0100, they call BDOS at
// 0005 for console output and jump to 0000 to exit. We give them plain 64K RAM
// (the ROM overlay off) and trap those two addresses by watching PC, so no bus
// abstraction is needed.
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "machine.h"

namespace {

std::string g_output;

void emit(char ch) {
    g_output.push_back(ch);
    std::fputc(ch, stdout);
}

// CP/M BDOS: function 2 prints E, function 9 prints a $-terminated string.
void bdos(ein::Machine& m) {
    const std::uint8_t fn = m.cpu.bc.b.l;
    if (fn == 2) {
        emit(static_cast<char>(m.cpu.de.b.l));
    } else if (fn == 9) {
        std::uint16_t a = m.cpu.de.w;
        for (int guard = 0; guard < 0x10000; ++guard) {
            const std::uint8_t ch = m.read(a++);
            if (ch == '$') break;
            emit(static_cast<char>(ch));
        }
    }
    // Simulate the RET that would end the BDOS call.
    const std::uint8_t lo = m.read(m.cpu.sp.w++);
    const std::uint8_t hi = m.read(m.cpu.sp.w++);
    m.cpu.pc.w = static_cast<std::uint16_t>(lo | (hi << 8));
}

}  // namespace

int main(int argc, char** argv) {
    // Progress must be visible when stdout is redirected to a file.
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    if (argc < 2) {
        std::fprintf(stderr, "usage: zex_runner <file.com>\n");
        return 2;
    }

    std::ifstream f(argv[1], std::ios::binary);
    if (!f) {
        std::fprintf(stderr, "cannot open %s\n", argv[1]);
        return 2;
    }
    const std::vector<std::uint8_t> prog{std::istreambuf_iterator<char>(f),
                                         std::istreambuf_iterator<char>()};
    if (prog.empty()) {
        std::fprintf(stderr, "empty file %s\n", argv[1]);
        return 2;
    }

    ein::Machine m;
    m.rom_enabled = false;  // plain 64K RAM
    // Nothing listens to the audio here, and synthesising billions of samples
    // costs real time on a run that already takes minutes.
    m.psg.set_audio_enabled(false);
    for (std::size_t i = 0; i < prog.size(); ++i) {
        m.write(static_cast<std::uint16_t>(0x0100 + i), prog[i]);
    }
    m.write(0x0000, 0x76);  // HALT at the warm-boot vector, as a backstop
    m.write(0x0005, 0xc9);  // RET at BDOS, ditto

    m.cpu.pc.w = 0x0100;
    m.cpu.sp.w = 0xf000;

    std::printf("running %s\n\n", argv[1]);
    const auto t0 = std::chrono::steady_clock::now();

    // Generous ceiling: zexall is roughly 5e9 T-states.
    constexpr std::uint64_t kMaxTStates = 100'000'000'000ull;
    bool completed = false;
    while (m.tstates < kMaxTStates) {
        if (m.cpu.pc.w == 0x0005) {
            bdos(m);
            continue;
        }
        if (m.cpu.pc.w == 0x0000) {
            completed = true;
            break;
        }
        m.step();
        if (m.cpu.halted) {
            completed = true;
            break;
        }
    }

    const auto t1 = std::chrono::steady_clock::now();
    const double secs = std::chrono::duration<double>(t1 - t0).count();
    std::fflush(stdout);

    const bool had_error = g_output.find("ERROR") != std::string::npos;

    std::printf("\n---\n");
    std::printf("instructions %llu\n", static_cast<unsigned long long>(m.instructions));
    std::printf("T-states     %llu\n", static_cast<unsigned long long>(m.tstates));
    std::printf("wall time    %.1f s", secs);
    if (secs > 0.01) {
        std::printf("  (%.1f MHz effective)",
                    static_cast<double>(m.tstates) / secs / 1e6);
    }
    std::printf("\n");

    if (!completed) {
        std::printf("RESULT: FAILED -- exceeded the T-state ceiling without finishing\n");
        return 1;
    }
    if (had_error) {
        std::printf("RESULT: FAILED -- the exerciser reported at least one ERROR\n");
        return 1;
    }
    std::printf("RESULT: PASSED\n");
    return 0;
}
