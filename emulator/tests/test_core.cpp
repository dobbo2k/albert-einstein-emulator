// M0 acceptance test. Headless: links ein_core only, no SDL, no ImGui.
//
// Usage: test_core <path-to-roms-dir>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include <initializer_list>

#include "adc0844.h"
#include "disasm.h"
#include "dskimage.h"
#include "fd1770.h"
#include "machine.h"
#include "z80pio.h"
#include "md5.h"
#include "rom.h"
#include "symbols.h"

namespace {

int g_failures = 0;
void print_gap() { std::printf("\n"); }
std::filesystem::path g_disks = "tests/disks";

void check(bool ok, const std::string& what) {
    std::printf("%s  %s\n", ok ? "[ PASS ]" : "[ FAIL ]", what.c_str());
    if (!ok) ++g_failures;
}

void check_eq(const std::string& got, const std::string& want, const std::string& what) {
    const bool ok = got == want;
    if (ok) {
        std::printf("[ PASS ]  %s\n", what.c_str());
    } else {
        std::printf("[ FAIL ]  %s\n           got:  %s\n           want: %s\n",
                    what.c_str(), got.c_str(), want.c_str());
        ++g_failures;
    }
}

// RFC 1321 test vectors -- proves the MD5 is correct before we trust it to
// identify ROM images.
void test_md5_vectors() {
    struct V { const char* in; const char* out; };
    static const V kVectors[] = {
        {"", "d41d8cd98f00b204e9800998ecf8427e"},
        {"a", "0cc175b9c0f1b6a831c399e269772661"},
        {"abc", "900150983cd24fb0d6963f7d28e17f72"},
        {"message digest", "f96b697d7cb7938d525a2f31aaf161d0"},
        {"abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b"},
        {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
         "d174ab98d277d9f5a5611c2c9f419d9f"},
        {"1234567890123456789012345678901234567890"
         "1234567890123456789012345678901234567890",
         "57edf4a22be3c955ac49da2e2107b67a"},
    };
    for (const auto& v : kVectors) {
        check_eq(ein::md5_hex(v.in, std::strlen(v.in)), v.out,
                 std::string("md5(\"") + v.in + "\")");
    }
    // Exercises the two-block padding path (len % 64 lands in 56..63).
    const std::string s(56, 'x');
    check(ein::md5_hex(s.data(), s.size()).size() == 32, "md5 56-byte padding path");
}

void test_banking(const ein::RomImage& rom);
void test_mos_reset(const ein::RomImage& rom);
void test_vdp(const ein::RomImage& rom);
int find_font(const ein::RomImage& rom);
void test_ctc_cascade();
void test_disk_geometry(const std::filesystem::path& roms);
void test_boot(const ein::RomImage& rom, const std::filesystem::path& roms,
               const char* dos, std::uint16_t want_entry);
void test_ctrl_break(const ein::RomImage& rom, const std::filesystem::path& roms);
void test_dos_session(const ein::RomImage& rom, const std::filesystem::path& roms);
void test_real_disk(const ein::RomImage& rom, const std::filesystem::path& disks);
void test_sound();
void test_envelope();
void test_game_sound(const ein::RomImage& rom, const std::filesystem::path& disks);
void test_printer(const ein::RomImage& rom);
void test_serial(const ein::RomImage& rom);
void test_adc();
void test_daisy_chain();
void test_disasm();
void test_symbols(const std::filesystem::path& roms, const ein::RomImage& rom);
void test_breakpoints(const ein::RomImage& rom);
void test_halt_wakeup();
void test_save_state(const ein::RomImage& rom, const std::filesystem::path& disks);
void test_sprites();
void test_mos121(const std::filesystem::path& roms);
void test_mos121_disk(const std::filesystem::path& roms);
void test_card80(const ein::RomImage& rom);
void test_vdp_timing(const ein::RomImage& rom);
void test_screenshot(const ein::RomImage& rom);
void test_rtc(const ein::RomImage& rom);
void test_keyboard(const ein::RomImage& rom);

void test_rom(const std::filesystem::path& roms) {
    const auto mos12 = roms / "Tatung X-TAL MOS v1.2 (1983)(Tatung).rom";
    if (!std::filesystem::exists(mos12)) {
        std::printf("[ SKIP ]  ROM not found: %s\n", mos12.string().c_str());
        ++g_failures;
        return;
    }

    const ein::RomImage rom = ein::load_rom(mos12);
    check_eq(rom.md5, ein::kMos12Md5, "MOS 1.2 ROM md5");
    check_eq(rom.identity, "MOS 1.2", "MOS 1.2 identified");
    check(rom.data.size() == ein::kRomSize, "ROM is 8192 bytes");
    check(!rom.truncated, "MOS 1.2 .rom needs no truncation");

    // roms/rom.bin is 8193 bytes: the same image plus one stray trailing byte.
    const auto rombin = roms / "rom.bin";
    if (std::filesystem::exists(rombin)) {
        const ein::RomImage r = ein::load_rom(rombin);
        check(r.file_size == 8193, "rom.bin is 8193 bytes on disk");
        check(r.truncated, "rom.bin flagged as truncated");
        check_eq(r.md5, ein::kMos12Md5, "rom.bin truncated to 8192 matches MOS 1.2");
    }

    const auto mos121 = roms / "einstein1.21.bin";
    if (std::filesystem::exists(mos121)) {
        const ein::RomImage r = ein::load_rom(mos121);
        check_eq(r.identity, "MOS 1.21", "einstein1.21.bin identified as MOS 1.21");
    }

    // The M0 acceptance criterion: the reset vector as seen through the bus.
    ein::Machine m;
    m.set_rom(rom);
    m.reset();

    static const std::uint8_t kExpect[8] = {0x21, 0x00, 0x80, 0x11, 0x01, 0x80, 0x18, 0x38};
    bool match = true;
    for (std::uint16_t i = 0; i < 8; ++i) match = match && (m.read(i) == kExpect[i]);
    check(match, "bus read at 0000 is 21 00 80 11 01 80 18 38 (LD HL,8000 / LD DE,8001 / JR +38)");

    check(m.cpu.pc.w == 0x0000, "reset PC = 0000");
    check(m.cpu.im == 0 && !m.cpu.iff1 && !m.cpu.iff2, "reset leaves IM 0, interrupts disabled");
    check(m.rom_enabled, "ROM overlay enabled after reset");

    // RAM under the ROM overlay stays writable.
    m.write(0x0000, 0xaa);
    check(m.read(0x0000) == 0x21, "ROM overlay shadows RAM writes at 0000");
    m.rom_enabled = false;
    check(m.read(0x0000) == 0xaa, "RAM under the overlay retained the write");

    std::printf("\n");
    test_banking(rom);
    std::printf("\n");
    test_mos_reset(rom);
    std::printf("\n");
    test_vdp(rom);
    std::printf("\n");
    test_ctc_cascade();
    std::printf("\n");
    test_rtc(rom);
    std::printf("\n");
    test_keyboard(rom);
    std::printf("\n");
    test_disk_geometry(roms);
    std::printf("\n");
    test_boot(rom, roms, "__XTALDOS202/SYSTEM202.CPM", 0xf800);
    test_boot(rom, roms, "__XTALDOS131/SYSTEM131.CPM", 0xfa00);
    std::printf("\n");
    test_ctrl_break(rom, roms);
    std::printf("\n");
    test_dos_session(rom, roms);
    test_real_disk(rom, g_disks);
    test_game_sound(rom, g_disks);
    print_gap();
    test_printer(rom);
    print_gap();
    test_serial(rom);
    print_gap();
    test_symbols(roms, rom);
    print_gap();
    test_breakpoints(rom);
    print_gap();
    test_screenshot(rom);
    print_gap();
    test_save_state(rom, g_disks);
    print_gap();
    test_mos121(roms);
    print_gap();
    test_mos121_disk(roms);
    print_gap();
    test_card80(rom);
    print_gap();
    test_vdp_timing(rom);
}

// Fast CPU smoke test. zexdoc/zexall are the real proof but take minutes each;
// this catches gross regressions in a millisecond.
void test_cpu() {
    ein::Machine m;
    m.rom_enabled = false;  // plain RAM

    static const std::uint8_t kProg[] = {
        0x3e, 0x05,        // LD A,5
        0x06, 0x03,        // LD B,3
        0x80,              // ADD A,B
        0x21, 0x34, 0x12,  // LD HL,1234
        0x23,              // INC HL
        0x76,              // HALT
    };
    for (std::size_t i = 0; i < sizeof kProg; ++i) {
        m.write(static_cast<std::uint16_t>(0x0100 + i), kProg[i]);
    }
    m.cpu.pc.w = 0x0100;

    // step() must report the T-states it consumed, not just tick the devices
    // with them. The app's frame budget is spent against this number, so a
    // zero return spins the render loop forever.
    int reported = 0;
    for (int guard = 0; guard < 100 && !m.cpu.halted; ++guard) reported += m.step().tstates;

    check(m.cpu.halted, "CPU reached HALT");
    check(reported == static_cast<int>(m.tstates),
          "step() reports the same T-states it charged to the machine");
    check(m.cpu.af.b.h == 8, "LD/ADD produced A=8");
    check(m.cpu.hl.w == 0x1235, "LD HL,1234 then INC HL gives 1235");
    // 7 + 7 + 4 + 10 + 6 + 4
    check(m.tstates == 38, "T-state accounting for the sequence is 38");

    // Flags: 5+3 = 8, so no zero, no carry, no sign; half-carry from 5+3.
    check((m.cpu.af.b.l & ein::Z80::ZF) == 0, "ADD result is not zero");
    check((m.cpu.af.b.l & ein::Z80::CF) == 0, "ADD produced no carry");
    check((m.cpu.af.b.l & ein::Z80::NF) == 0, "ADD clears the N flag");

    // Undocumented F3/F5 come straight from the result byte (8 -> bit 3 set).
    check((m.cpu.af.b.l & ein::Z80::XF) != 0, "F3 mirrors bit 3 of the result");
}

// Port 24H is a write strobe: any write toggles the ROM overlay and the data
// bus is ignored. Encoded here because MOS depends on it (UPPERMEM.ZSM:33-52).
void test_banking(const ein::RomImage& rom) {
    ein::Machine m;
    m.set_rom(rom);
    m.reset();

    check(m.rom_enabled, "overlay starts enabled");
    check(m.read(0x0000) == 0x21, "reads ROM at 0000 with the overlay in");

    m.write(0x0000, 0x5a);  // lands in RAM underneath

    m.io_write(0x24, 0x00);
    check(!m.rom_enabled, "a write of 00 toggles the overlay out");
    check(m.read(0x0000) == 0x5a, "now reads RAM underneath");

    // The LFB14 case: the second OUT carries an unrelated data byte and must
    // still switch back. Different value, same effect.
    m.io_write(0x24, 0xff);
    check(m.rom_enabled, "a write of FF toggles it back in -- data is ignored");
    check(m.read(0x0000) == 0x21, "ROM visible again");

    m.io_write(0x24, 0x9a);  // an MCAL number, as at LFC3B
    check(!m.rom_enabled, "a write of 9A toggles again");
    check(m.rom_toggles == 3, "three toggles counted");

    // Only port 24 does this.
    const bool before = m.rom_enabled;
    m.io_write(0x25, 0xff);
    check(m.rom_enabled == before, "port 25 does not affect the overlay");
    // The Z80 puts the full 16-bit address on the bus; only the low byte decodes.
    m.io_write(0xab24, 0x00);
    check(m.rom_enabled != before, "port 24 decodes on the low address byte");
}

// Runs the real MOS 1.2 reset sequence (Mos12.zsm:255-300) to the IM 2 at
// 0090 and checks every documented effect.
void test_mos_reset(const ein::RomImage& rom) {
    ein::Machine m;
    m.set_rom(rom);
    m.reset();

    int steps = 0;
    constexpr int kCap = 400000;
    while (m.cpu.pc.w != 0x0090 && steps < kCap && !m.cpu.halted) {
        m.step();
        ++steps;
    }
    check(m.cpu.pc.w == 0x0090, "reset sequence reaches IM 2 at 0090");
    if (m.cpu.pc.w != 0x0090) return;

    const auto& r = rom.data;

    // 1. RAM filled with FF, including underneath the ROM overlay.
    m.rom_enabled = false;
    bool fill_ok = true;
    for (std::uint32_t a = 0x000b; a < 0x2000; ++a) fill_ok = fill_ok && (m.read(std::uint16_t(a)) == 0xff);
    for (std::uint32_t a = 0x2000; a < 0xfb00; ++a) fill_ok = fill_ok && (m.read(std::uint16_t(a)) == 0xff);
    m.rom_enabled = true;
    check(fill_ok, "RAM 000B-FAFF filled with FF, overlay included");

    // 2. FB00-FFFF cleared, apart from the copied regions, the RTC, and the
    // three scratchpad variables the tail of the reset writes (Mos12.zsm:285-288).
    auto reset_written = [](std::uint32_t a) {
        return (a >= 0xfb8c && a <= 0xfb91) ||  // RTC
               a == 0xfb60 ||                   // USERI
               a == 0xfb65 || a == 0xfb66 ||    // USERSP
               a == 0xfbb0;                     // DFLAG
    };
    bool clear_ok = true;
    for (std::uint32_t a = 0xfb46; a < 0xfc09; ++a) {
        if (reset_written(a)) continue;
        clear_ok = clear_ok && (m.read(std::uint16_t(a)) == 0x00);
    }
    for (std::uint32_t a = 0xfc8d; a <= 0xfcfe; ++a) clear_ok = clear_ok && (m.read(std::uint16_t(a)) == 0x00);
    check(clear_ok, "FB00-FFFF cleared outside the copied and written regions");

    check(m.read(0xfb60) == 0xfb, "USERI (FB60) = FB");
    check(m.read(0xfb65) == 0xff && m.read(0xfb66) == 0xfc, "USERSP (FB65) = FCFF");
    check(m.read(0xfbb0) == 0x01, "DFLAG (FBB0) = 01, the 12ms step rate");

    // 3. The three ROM-to-RAM block copies, byte for byte. 0000-000A lives
    // under the overlay, so it must be read with the ROM paged out.
    bool c1 = true, c2 = true, c3 = true;
    for (int i = 0; i < 0x46; ++i) c1 = c1 && (m.read(std::uint16_t(0xfb00 + i)) == r[0x0ff0 + i]);
    m.rom_enabled = false;
    for (int i = 0; i < 11; ++i) c2 = c2 && (m.read(std::uint16_t(0x0000 + i)) == r[0x1036 + i]);
    m.rom_enabled = true;
    for (int i = 0; i < 0x84; ++i) c3 = c3 && (m.read(std::uint16_t(0xfc09 + i)) == r[0x1041 + i]);
    check(c1, "copy 0FF0-1035 -> FB00-FB45 (interrupt vectors) matches ROM");
    check(c2, "copy 1036-1040 -> 0000-000A matches ROM (read under the overlay)");
    check(c3, "copy 1041-10C4 -> FC09-FC8C (banking trampolines) matches ROM");

    // The copied trampoline must really be OUT (24H),A / RET at FC1F.
    check(m.read(0xfc1f) == 0xd3 && m.read(0xfc20) == 0x24 && m.read(0xfc21) == 0xc9,
          "FC1F holds OUT (24H),A / RET");

    // 4. RTC reset to six ASCII zeroes at FB8C.
    bool rtc = true;
    for (int i = 0; i < 6; ++i) rtc = rtc && (m.read(std::uint16_t(0xfb8c + i)) == 0x30);
    check(rtc, "RTC at FB8C set to six 30H bytes");

    // 5. CPU state.
    check(m.cpu.sp.w == 0xfcff, "SP = FCFF");
    check(m.cpu.i == 0xfb, "I = FB (interrupt vector page)");
    check(m.cpu.im == 0, "IM still 0 -- the IM 2 has not executed yet");
    m.step();
    check(m.cpu.im == 2, "IM 2 after executing the instruction at 0090");

    std::printf("           (reset sequence: %d instructions, %llu T-states)\n", steps,
                static_cast<unsigned long long>(m.tstates));

    // Carry on into MOS proper. With no devices the machine cannot get far,
    // but init should complete and interrupts should end up enabled.
    for (int i = 0; i < 3'000'000 && !m.cpu.halted; ++i) m.step();
    check(m.cpu.iff1 && m.cpu.im == 2, "MOS completes init and enables IM 2 interrupts");
    std::printf("           (after 3M instructions: PC=%04X, port-24 writes=%llu)\n", m.cpu.pc.w,
                static_cast<unsigned long long>(m.rom_toggles));
}

// M3: run MOS until it has drawn, then inspect what the VDP was told to do and
// read the banner back out of the name table.
void test_vdp(const ein::RomImage& rom) {
    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();

    const auto& v = m.vdp;
    static const char* kModeName[] = {"Graphics I", "Graphics II", "Multicolour", "Text", "?"};
    std::printf("           VDP: mode=%s display=%s  regs=", kModeName[v.mode()],
                v.display_enabled() ? "on" : "off");
    for (int i = 0; i < 8; ++i) std::printf("%02X ", v.reg(i));
    std::printf("\n           name=%04X pattern=%04X colour=%04X  writes: reg=%llu vram=%llu\n",
                v.name_base(), v.pattern_base(), v.colour_base(),
                static_cast<unsigned long long>(v.reg_writes()),
                static_cast<unsigned long long>(v.vram_writes()));

    check(v.reg_writes() > 0, "MOS wrote VDP registers");
    check(v.vram_writes() > 0, "MOS wrote VRAM");
    check(v.display_enabled(), "MOS enabled the display (R1 bit 6)");

    // MOS drives a Graphics II bitmap screen and renders text into the pattern
    // table as pixels -- the name table just holds the sequential bitmap index,
    // so there is no ASCII to read back. The table layout must match the
    // documented Einstein map (Einstein.zsm:342-348).
    check(v.mode() == ein::Tms9129::kGraphics2, "MOS selects Graphics II");
    check(v.name_base() == 0x3800, "name table at 3800 (VPNTBL)");
    check((v.reg(4) & 0x04) == 0, "G2 pattern generator bank base 0000 (VPGTBL)");
    check((v.reg(3) & 0x80) != 0, "G2 colour table bank base 2000 (VPCTBL)");
    check(v.sprite_pat_base() == 0x1800, "sprite pattern table at 1800 (VSPTBL)");
    check((v.reg(7) & 0x0f) == 4, "backdrop is colour 4, dark blue");
    check((v.reg(7) >> 4) == 15, "text colour is 15, white");

    // The name table should be the incrementing bitmap index, not characters.
    bool seq = true;
    for (int i = 0; i < 256; ++i) {
        seq = seq && (v.vram(std::uint16_t(v.name_base() + i)) == (i & 0xff));
    }
    check(seq, "name table holds the sequential Graphics II bitmap index");

    // Render and confirm the screen is not simply blank backdrop.
    std::vector<std::uint32_t> fb(ein::Tms9129::kWidth * ein::Tms9129::kHeight);
    v.render(fb.data());
    const std::uint32_t backdrop = ein::Tms9129::palette(v.reg(7) & 0x0f);
    std::size_t lit = 0;
    for (std::uint32_t px : fb) if (px != backdrop) ++lit;
    std::printf("           rendered %zu / %zu non-backdrop pixels\n", lit, fb.size());
    check(lit > 500, "something is actually drawn on the rendered screen");

    // Coarse ASCII view of the top of the screen, as a smoke check.
    for (int y = 0; y < 64; y += 4) {
        std::string line;
        for (int x = 0; x < ein::Tms9129::kWidth; x += 2) {
            line.push_back(fb[y * ein::Tms9129::kWidth + x] != backdrop ? '#' : '.');
        }
        std::printf("           %s\n", line.c_str());
    }
}

// M4: the CTC cascade, the mode-2 interrupt path, and the keyboard.
//
// ZZTIME (Mos12.zsm:2717) programs channel 2 as a 125 Hz timer and channel 3 as
// a counter dividing it by 125, giving exactly 1 Hz into the RTC at FB8C.
void test_ctc_cascade() {
    ein::Machine m;
    m.rom_enabled = false;

    // Reproduce exactly what ZZTIME writes.
    m.io_write(0x28, 0x00);  // CTC0: interrupt vector base 00
    m.io_write(0x2a, 0x3f);  // CTC2: timer, prescale 256, TC follows
    m.io_write(0x2a, 0x7d);  // TC = 125
    m.io_write(0x2b, 0xdf);  // CTC3: counter, TC follows, interrupt enabled
    m.io_write(0x2b, 0x7d);  // TC = 125

    check(m.ctc.vector() == 0x00, "CTC interrupt vector base is 00");

    // One second of CPU time.
    for (int i = 0; i < ein::Machine::kCpuClockHz; i += 4) m.ctc.tick(4);

    const auto c2 = m.ctc.zero_counts(2);
    const auto c3 = m.ctc.zero_counts(3);
    std::printf("           after 1 s: channel 2 fired %llu times, channel 3 %llu\n",
                static_cast<unsigned long long>(c2), static_cast<unsigned long long>(c3));
    check(c2 == 125, "channel 2 produces 125 Hz (4000000 / 256 / 125)");
    check(c3 == 1, "channel 3 divides that to exactly 1 Hz");
    check(m.ctc.int_pending(), "channel 3 raises an interrupt");

    // Channel 3's vector is base | (3 << 1) = 06, which with I=FB selects the
    // RTC handler at FB06 (UPPERMEM.ZSM ICTC3).
    check(m.ctc.int_ack() == 0x06, "channel 3 acknowledges with vector 06 -> FB06");
    check(!m.ctc.int_pending(), "in-service latch blocks further interrupts until RETI");
    m.ctc.reti();
    check(!m.ctc.int_pending(), "nothing pending after RETI");
}

// The real thing: run MOS and watch its BCD clock at FB8C advance.
void test_rtc(const ein::RomImage& rom) {
    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();

    auto rtc = [&] {
        std::string s;
        for (int i = 0; i < 6; ++i) s.push_back(static_cast<char>(m.read(std::uint16_t(0xfb8c + i))));
        return s;
    };
    const std::string before = rtc();
    const std::uint64_t t0 = m.tstates;

    // Advance five emulated seconds.
    const std::uint64_t target = t0 + 5ull * ein::Machine::kCpuClockHz;
    while (m.tstates < target && !m.cpu.halted) m.step();

    const std::string after = rtc();
    std::printf("           RTC %s -> %s after 5 s, %llu interrupts taken\n", before.c_str(),
                after.c_str(), static_cast<unsigned long long>(m.interrupts_taken));

    check(m.interrupts_taken > 0, "CTC interrupts reach the CPU");
    check(before != after, "the RTC at FB8C advances");
    // The clock is stored as BCD digits in ASCII; five seconds should show.
    check(after[5] != before[5] || after[4] != before[4], "seconds digits changed");
}

// Typing at the MOS prompt must echo. This drives the real 8x8 matrix through
// the PSG exactly the way the firmware reads it, and checks the result against
// two tables in the ROM itself: the key codes at L10C5 and the font at L116D.
void test_keyboard(const ein::RomImage& rom) {
    // 1. The matrix table, byte for byte against L10C5 (Mos12.zsm:2827).
    //    24 bytes per matrix row -- normal, shifted, control -- for rows 02..80.
    int bad = -1;
    for (int d = 0; d < ein::Keyboard::kRomTableRows && bad < 0; ++d) {
        for (int p = 0; p < 3 && bad < 0; ++p) {
            for (int c = 0; c < 8; ++c) {
                const std::uint8_t want = rom.data[ein::Keyboard::kRomTableAddr + d * 24 + p * 8 + c];
                const std::uint8_t got =
                    ein::Keyboard::code(d + 1, c, static_cast<ein::Keyboard::Plane>(p));
                if (got != want) {
                    std::printf("           row %02X plane %d col %d: have %02X, ROM has %02X\n",
                                2 << d, p, c, got, want);
                    bad = 0;
                    break;
                }
            }
        }
    }
    check(bad < 0, "all 168 key codes match the ROM table at 10C5");

    int row = 0, col = 0;
    bool shift = false;
    check(ein::Keyboard::map_ascii('a', row, col, shift) && row == 6 && col == 6 && !shift,
          "'a' maps to row 40, column 40");
    check(ein::Keyboard::map_ascii('A', row, col, shift) && row == 6 && col == 6 && shift,
          "'A' is the same key with shift");
    check(ein::Keyboard::map_ascii('1', row, col, shift) && row == 4 && col == 6 && !shift,
          "'1' maps to row 10, column 40");

    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();

    // 2. What the firmware sees on the bus: L0F99 selects rows by writing ~E to
    //    PSG register 14, L0FA5 reads the columns back from register 15 and
    //    complements them. E=FF selects every row, which is what L0763 does.
    auto psg_scan = [&](std::uint8_t rows_active_high) {
        m.io_write(0x02, 0x0e);
        m.io_write(0x03, static_cast<std::uint8_t>(~rows_active_high));
        m.io_write(0x02, 0x0f);
        return static_cast<std::uint8_t>(~m.io_read(0x02));
    };
    m.keyboard.set_key(6, 6, true);  // the 'a' key, row 40 column 40
    const std::uint8_t all_rows = psg_scan(0xff);
    const std::uint8_t one_row = psg_scan(0x40);
    const std::uint8_t other_row = psg_scan(0x80);
    m.keyboard.set_key(6, 6, false);
    check(all_rows == 0x40, "a key held shows on its column with every row selected");
    check(one_row == 0x40, "and on its own row alone");
    check(other_row == 0x00, "but not on any other row");
    check((m.psg.reg(7) & 0xc0) == 0x40,
          "ZPINIT left PSG R7 = 7F: port A output (rows), port B input (columns)");

    // 3. LKEY (FB46) holds the code MOS accepted. It has to be sampled while the
    //    key is down: L0763 clears it the moment nothing is pressed, and the
    //    auto-repeat path at L0708 clears it periodically during the hold too.
    auto press = [&](int r, int c, bool sh, bool ctrl) {
        m.keyboard.set_modifier(ein::Keyboard::kShift, sh);
        m.keyboard.set_modifier(ein::Keyboard::kControl, ctrl);
        m.keyboard.set_key(r, c, true);
        std::uint8_t seen = 0;
        for (int i = 0; i < 300000; ++i) {
            m.step();
            const std::uint8_t k = m.read(0xfb46);
            if (k) seen = k;
        }
        m.keyboard.set_key(r, c, false);
        m.keyboard.set_modifier(ein::Keyboard::kShift, false);
        m.keyboard.set_modifier(ein::Keyboard::kControl, false);
        for (int i = 0; i < 200000; ++i) m.step();
        return seen;
    };

    // 4. The screen. MOS boots at WIDTH = 40 (FB4F), and ZVOUT's 40-column path
    //    (Mos12.zsm:1888) packs four characters into three bytes -- six pixels
    //    per character, masked to the top six bits of the font byte with
    //    AND 0FCH. Characters therefore never line up with VRAM pattern cells,
    //    so the echo has to be checked in rendered pixels: character n occupies
    //    x = n*6 .. n*6+5 of the row it is on.
    const std::uint8_t width = m.read(0xfb4f);
    std::printf("           WIDTH (FB4F) = %u, CUSCOL/CUSROW = %u,%u\n", width, m.read(0xfb4a),
                m.read(0xfb4b));
    check(width == 40, "MOS boots the 40-column text screen");

    std::vector<std::uint32_t> fb(ein::Tms9129::kWidth * ein::Tms9129::kHeight);
    const int font_base = find_font(rom);
    auto count_glyph = [&](std::uint8_t ch) {
        m.vdp.render(fb.data());
        const std::uint32_t bg = ein::Tms9129::palette(m.vdp.reg(7) & 0x0f);
        const std::uint16_t g = static_cast<std::uint16_t>(font_base + (ch - 0x20) * 8);
        int n = 0;
        for (int cy = 0; cy < 24; ++cy) {
            for (int cx = 0; cx < 40; ++cx) {
                bool same = true;
                for (int r = 0; r < 8 && same; ++r) {
                    const std::uint8_t bits = rom.data[g + r];
                    for (int k = 0; k < 6; ++k) {
                        const bool want = (bits & (0x80 >> k)) != 0;
                        const bool lit =
                            fb[(cy * 8 + r) * ein::Tms9129::kWidth + cx * 6 + k] != bg;
                        if (want != lit) { same = false; break; }
                    }
                }
                if (same) ++n;
            }
        }
        return n;
    };

    // MOS boots with caps on -- UPPERMEM.ZSM presets KFLAG to 80H and ZKSCAN
    // keeps bit 7 across every scan (Mos12.zsm:1326). With caps on, a lowercase
    // table entry is looked up again in the shifted plane (Mos12.zsm:1412-1428),
    // so the unshifted 'a' key produces 'A'.
    const int a_before = count_glyph('A');
    const std::uint8_t k_a = press(6, 6, false, false);
    const int a_after = count_glyph('A');
    std::printf("           unshifted 'a' key -> %02X, 'A' glyphs on screen %d -> %d\n",
                k_a, a_before, a_after);
    check(k_a == 'A', "MOS accepted the key and stored 'A' in LKEY (caps on at boot)");
    // Holding a key long enough makes MOS auto-repeat it (KDEL then KSCAN,
    // Mos12.zsm:1256-1268), so the count grows by at least one, not exactly one.
    check(a_after > a_before, "the character was echoed as the ROM's 'A' glyph");

    const int z_before = count_glyph('Z');
    const std::uint8_t k_z = press(7, 6, true, false);  // shift + the 'z' key
    const int z_after = count_glyph('Z');
    std::printf("           shift + 'z' key -> %02X, 'Z' glyphs on screen %d -> %d\n",
                k_z, z_before, z_after);
    check(k_z == 'Z', "shift reaches MOS through port 20 and selects the shifted plane");
    check(z_after > z_before, "'Z' was echoed too");

    // Control uses the third plane of the same table: the 'a' key gives 01H.
    const std::uint8_t k_ctrl = press(6, 6, false, true);
    std::printf("           ctrl + 'a' key -> %02X\n", k_ctrl);
    check(k_ctrl == 0x01, "control selects the third plane of the table (ctrl-A = 01)");
}

// Reads the 40x24 text screen back as ASCII.
//
// MOS draws text as Graphics II pixels, six per character, using the top six
// bits of each font byte (ZVOUT's 40-column path, Mos12.zsm:1888). All 96
// glyphs of the ROM font at L116D stay distinct under that mask, so matching a
// rendered 6x8 block against the font recovers the character exactly.
// Finds the ROM's 8x8 character set rather than hardcoding where it sits: MOS
// 1.2 keeps it at 116D and MOS 1.21 at 117A, byte for byte the same table
// thirteen bytes further on. The signature is the first three glyphs -- space,
// then exclamation mark, then double quote -- which is specific enough to have
// exactly one match.
int find_font(const ein::RomImage& rom) {
    static const std::uint8_t kSig[24] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00,
        0x00, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00,
    };
    if (rom.data.size() < sizeof kSig) return -1;
    for (std::size_t i = 0; i + sizeof kSig <= rom.data.size(); ++i) {
        if (std::memcmp(rom.data.data() + i, kSig, sizeof kSig) == 0) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::vector<std::string> read_screen(ein::Machine& m, const ein::RomImage& rom) {
    std::vector<std::uint32_t> fb(ein::Tms9129::kWidth * ein::Tms9129::kHeight);
    m.vdp.render(fb.data());
    const std::uint32_t bg = ein::Tms9129::palette(m.vdp.reg(7) & 0x0f);

    const int font = find_font(rom);
    std::vector<std::string> out;
    if (font < 0) return out;
    for (int cy = 0; cy < 24; ++cy) {
        std::string line;
        for (int cx = 0; cx < 40; ++cx) {
            char found = ' ';
            for (int ch = 0x20; ch < 0x80; ++ch) {
                const std::uint16_t g = static_cast<std::uint16_t>(font + (ch - 0x20) * 8);
                bool same = true;
                for (int r = 0; r < 8 && same; ++r) {
                    const std::uint8_t bits = rom.data[g + r];
                    for (int k = 0; k < 6; ++k) {
                        const bool want = (bits & (0x80 >> k)) != 0;
                        const bool lit = fb[(cy * 8 + r) * ein::Tms9129::kWidth + cx * 6 + k] != bg;
                        if (want != lit) { same = false; break; }
                    }
                }
                if (same) { found = static_cast<char>(ch); break; }
            }
            line.push_back(found);
        }
        while (!line.empty() && line.back() == ' ') line.pop_back();
        out.push_back(line);
    }
    return out;
}

void print_screen(const std::vector<std::string>& lines) {
    for (const std::string& l : lines) {
        if (!l.empty()) std::printf("           | %s\n", l.c_str());
    }
}

bool screen_has(const std::vector<std::string>& lines, const std::string& what) {
    for (const std::string& l : lines) {
        if (l.find(what) != std::string::npos) return true;
    }
    return false;
}

// M5: the FD1770 and disk images.
//
// The archive has no .dsk, but it does have the two XtalDOS system tracks as
// SYSTEM131.CPM / SYSTEM202.CPM -- 10240 bytes each, exactly two tracks of ten
// 512-byte sectors. DiskImage pads one out into a blank bootable disc, which is
// all FBOOT needs.
void test_disk_geometry(const std::filesystem::path& roms) {
    ein::DiskImage d;
    d.format(1);
    auto off = [](const ein::DiskImage& img, int t, int side, int id) {
        const ein::DiskImage::Sector* s = img.find(t, side, id);
        return s ? static_cast<long long>(s->offset) : -1LL;
    };
    check(d.data().size() == 40 * 10 * 512, "a single-sided disc is 40 x 10 x 512 bytes");
    check(off(d, 0, 0, 0) == 0, "track 0 sector 0 is at offset 0");
    check(off(d, 0, 0, 9) == 9 * 512, "sectors run 0-9 within a track");
    check(off(d, 1, 0, 0) == 10 * 512, "track 1 follows track 0");
    check(off(d, 0, 0, 10) < 0, "sector 10 does not exist on a single-sided disc");
    check(off(d, 40, 0, 0) < 0, "there is no track 40");
    check(off(d, 0, 1, 10) < 0, "and no side 1");

    ein::DiskImage two;
    two.format(2);
    check(two.data().size() == 40 * 2 * 10 * 512, "a double-sided disc is twice that");
    // CALDSC sets the side bit when HSTSEC >= 10 and still writes the raw
    // number to the sector register, so ids 10-19 are side 1 (Mos12.zsm:4079).
    check(off(two, 0, 1, 10) == 10 * 512, "side 1 of track 0 follows side 0");
    check(off(two, 0, 1, 9) < 0, "id 9 is not on side 1");
    check(off(two, 0, 0, 10) < 0, "and id 10 is not on side 0");
    check(off(two, 1, 0, 0) == 20 * 512, "sides interleave within a track");

    // The system tracks load and carry the boot header FBOOT expects
    // (Mos12.zsm:484): start address, end address, entry point, drive types.
    ein::DiskImage sys;
    sys.load(roms / "disassembled" / "Einstein" / "__XTALDOS202" / "SYSTEM202.CPM");
    check(sys.loaded() && sys.data().size() == 40 * 10 * 512,
          "SYSTEM202.CPM pads out to a blank bootable disc");
    const std::uint8_t* boot = sys.sector(0, 0, 0);
    const std::uint16_t start = std::uint16_t(boot[0] | (boot[1] << 8));
    const std::uint16_t end = std::uint16_t(boot[2] | (boot[3] << 8));
    const std::uint16_t entry = std::uint16_t(boot[4] | (boot[5] << 8));
    std::printf("           boot header: start %04X end %04X entry %04X dtype %02X\n",
                start, end, entry, boot[6]);
    check(start == 0xe100, "XtalDOS 2.02 loads from E100 (CCP202)");
    check(entry == 0xf800, "and enters at F800 (BIOS202)");
    check(end == 0xfb00, "loading 1A00 bytes, 13 sectors");
}

// Boot the real thing. With a disc in drive 0, MOS boots unasked: the reset
// tail selects drive 0, calls ZHMDSC and jumps straight to FBOOT if the drive
// comes ready (Mos12.zsm:331-333).
void test_boot(const ein::RomImage& rom, const std::filesystem::path& roms, const char* dos,
               std::uint16_t want_entry) {
    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    m.fdc.disk(0).load(roms / "disassembled" / "Einstein" / dos);

    // Run until the CPU reaches the entry point the boot header named.
    const std::uint64_t limit = 30ull * ein::Machine::kCpuClockHz;
    bool entered = false;
    bool rom_out_at_entry = false;
    bool image_matches = false;
    std::uint16_t first_bad = 0;
    double entry_secs = 0;

    while (m.tstates < limit && !m.cpu.halted) {
        if (!entered && m.cpu.pc.w == want_entry) {
            entered = true;
            entry_secs = double(m.tstates) / ein::Machine::kCpuClockHz;
            // FBOOT ends with JP LFB17, which strobes port 24 and returns to
            // the entry point, so all RAM must be paged in right now.
            rom_out_at_entry = !m.rom_enabled;
            // And the loaded image must match the disc byte for byte, checked
            // before DOS has had a chance to touch its own data areas.
            const ein::DiskImage& d = m.fdc.disk(0);
            image_matches = true;
            for (int i = 0; i < 0x1a00; ++i) {
                const int sec = i / 512;
                const std::uint8_t* s = d.sector(sec / 10, 0, sec % 10);
                if (m.read(std::uint16_t(0xe100 + i)) != s[i % 512]) {
                    image_matches = false;
                    first_bad = std::uint16_t(0xe100 + i);
                    break;
                }
            }
        }
        m.step();
    }

    const std::vector<std::string> screen = read_screen(m, rom);
    std::printf("           %s: entered %04X at %.2f s, %llu sectors read, %llu RNF\n", dos,
                want_entry, entry_secs, static_cast<unsigned long long>(m.fdc.sectors_read()),
                static_cast<unsigned long long>(m.fdc.not_found()));
    print_screen(screen);

    check(entered, "the CPU reached the entry point named in the boot sector");
    check(m.fdc.not_found() == 0, "no sector went missing");
    check(m.fdc.sectors_read() >= 14, "the boot sector plus the thirteen system sectors");
    check(rom_out_at_entry, "the ROM overlay is paged out when DOS is entered");
    if (!image_matches) std::printf("           first mismatch at %04X\n", first_bad);
    check(image_matches, "E100-FB00 held the system tracks byte for byte at entry");
    check(m.read(want_entry) == 0xc3, "the BIOS entry point holds a JP");
    check(screen_has(screen, "XtalDOS"), "XtalDOS signed on");
    // Not "A>": PRNPMT prints the drive as a digit then a colon
    // (CCP202.ZSM:275-284), so an Einstein CCP prompt is "0:".
    check(screen_has(screen, "0:"), "and reached the CCP prompt");
}

// Ctrl-BREAK: the other way in. Boot with an empty drive, reach the MOS prompt,
// then insert a disc and press it. ZKSCAN waits for BREAK to be *released*
// before it looks at the modifiers (Mos12.zsm:1336-1348), so control has to
// still be held at that point -- which is what "Ctrl-BREAK" means in practice.
void test_ctrl_break(const ein::RomImage& rom, const std::filesystem::path& roms) {
    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();

    const std::vector<std::string> before = read_screen(m, rom);
    check(m.rom_enabled && m.cpu.pc.w < 0x2000,
          "with an empty drive MOS stays in the ROM");
    check(screen_has(before, "Insert disc in drive 0"),
          "and asks for a disc instead of booting");

    m.fdc.disk(0).load(roms / "disassembled" / "Einstein" / "__XTALDOS131" / "SYSTEM131.CPM");

    m.keyboard.set_modifier(ein::Keyboard::kControl, true);
    m.keyboard.set_key(0, 0, true);  // BREAK, matrix row 01 column 0
    for (int i = 0; i < 200000; ++i) m.step();
    m.keyboard.set_key(0, 0, false);  // release BREAK, keep control down

    const std::uint64_t limit = m.tstates + 30ull * ein::Machine::kCpuClockHz;
    bool entered = false;
    while (m.tstates < limit && !m.cpu.halted) {
        if (!entered && m.cpu.pc.w == 0xfa00) {
            entered = true;
            m.keyboard.set_modifier(ein::Keyboard::kControl, false);
        }
        m.step();
    }

    const std::vector<std::string> after = read_screen(m, rom);
    std::printf("           ctrl-BREAK: entered=%d, %llu sectors read\n", entered,
                static_cast<unsigned long long>(m.fdc.sectors_read()));
    print_screen(after);
    check(entered, "Ctrl-BREAK booted drive 0 and entered XtalDOS 1.31 at FA00");
    check(m.read(0xfa00) == 0xc3, "XtalDOS 1.31 put its BIOS jump table at FA00");
    check(screen_has(after, "0:"), "and reached the CCP prompt");
}

// Types one character on the real matrix and waits for MOS to accept it.
//
// The key is released as soon as LKEY (FB46) goes non-zero, which is what stops
// the auto-repeat at L0708 from turning one keystroke into three, and then we
// wait for L0763 to clear LKEY again so the next character starts from a known
// state. MOS boots with caps on, so letters are typed unshifted and come out
// upper case -- which is what the CCP wants anyway.
bool type_char(ein::Machine& m, char ch) {
    char key = ch;
    if (key >= 'A' && key <= 'Z') key = static_cast<char>(key - 'A' + 'a');
    int r = 0, c = 0;
    bool sh = false;
    if (!ein::Keyboard::map_ascii(key, r, c, sh)) return false;

    // Caps is on, so an unshifted letter key comes back upper case: the code we
    // are waiting for is the character as written.
    const std::uint8_t want = static_cast<std::uint8_t>(ch);

    auto wait_for = [&](std::uint8_t value, int steps) {
        for (int i = 0; i < steps; ++i) {
            if (m.read(0xfb46) == value) return true;
            m.step();
        }
        return m.read(0xfb46) == value;
    };

    // LKEY must be clear before we start, otherwise a stale value from the
    // previous keystroke reads as an acceptance and the character is lost.
    if (!wait_for(0, 2'000'000)) return false;

    m.keyboard.set_modifier(ein::Keyboard::kShift, sh);
    m.keyboard.set_key(r, c, true);
    const bool accepted = wait_for(want, 2'000'000);
    m.keyboard.set_key(r, c, false);
    m.keyboard.set_modifier(ein::Keyboard::kShift, false);
    if (!accepted) {
        std::printf("           key '%c' (%02X) was never accepted; LKEY = %02X\n", ch, want,
                    m.read(0xfb46));
        return false;
    }
    return wait_for(0, 2'000'000);
}

bool type_line(ein::Machine& m, const std::string& text) {
    for (char ch : text) {
        if (!type_char(m, ch)) return false;
    }
    if (!type_char(m, '\r')) return false;
    // Let the CCP run the command.
    for (int i = 0; i < 6'000'000 && !m.cpu.halted; ++i) m.step();
    return true;
}

// A real session at the CCP: SAVE writes a file through the FDC, DIR reads the
// directory back. This is the whole path -- keyboard, MOS, BDOS, BIOS, the
// FD1770 and the disc image -- in both directions.
void test_dos_session(const ein::RomImage& rom, const std::filesystem::path& roms) {
    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    m.fdc.disk(0).load(roms / "disassembled" / "Einstein" / "__XTALDOS202" / "SYSTEM202.CPM");

    bool entered = false;
    const std::uint64_t limit = 30ull * ein::Machine::kCpuClockHz;
    while (m.tstates < limit && !m.cpu.halted && !entered) {
        m.step();
        if (m.cpu.pc.w == 0xf800) entered = true;
    }
    for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();
    check(entered, "booted XtalDOS 2.02 for the session");

    const std::uint64_t writes_before = m.fdc.sectors_written();
    check(type_line(m, "DIR"), "typed DIR at the prompt");
    std::vector<std::string> screen = read_screen(m, rom);
    print_screen(screen);
    check(screen_has(screen, "k Free"), "DIR read the directory of a blank disc");
    check(screen_has(screen, "No File") || screen_has(screen, "NO FILE") ||
              !screen_has(screen, "TEST"),
          "and found nothing on it");

    check(type_line(m, "SAVE 1 TEST.COM"), "typed SAVE 1 TEST.COM");
    std::printf("           sectors written: %llu\n",
                static_cast<unsigned long long>(m.fdc.sectors_written()));
    check(m.fdc.sectors_written() > writes_before, "SAVE wrote sectors through the FD1770");

    check(type_line(m, "DIR"), "typed DIR again");
    screen = read_screen(m, rom);
    print_screen(screen);
    check(screen_has(screen, "TEST"), "DIR now lists TEST");
    check(screen_has(screen, "COM"), "with type COM");
    check(m.fdc.not_found() == 0, "and nothing went missing along the way");

    // The file has to be on the disc image itself, not just in a cache: the
    // CP/M directory lives in the data area and its entries start with the user
    // number followed by the eight-character name.
    const std::vector<std::uint8_t>& img = m.fdc.disk(0).data();
    bool on_disc = false;
    for (std::size_t i = 0; i + 12 < img.size(); ++i) {
        if (img[i] == 0x00 && std::memcmp(&img[i + 1], "TEST    COM", 11) == 0) {
            std::printf("           directory entry for TEST.COM at image offset %zu "
                        "(track %zu, sector %zu)\n",
                        i, i / (10 * 512), (i % (10 * 512)) / 512);
            on_disc = true;
            break;
        }
    }
    check(on_disc, "the directory entry is in the disc image");
}

// A real commercial disc, in the format emulators actually trade Einstein
// software in: an Extended CPC DSK. Everything the geometry was derived from
// the firmware is confirmed here by the disc itself.
void test_real_disk(const ein::RomImage& rom, const std::filesystem::path& disks) {
    const auto path = disks / "OHMUMMY.DSK";
    if (!std::filesystem::exists(path)) {
        std::printf("[ SKIP ]  %s not present\n", path.string().c_str());
        return;
    }

    ein::DiskImage d;
    d.load(path);
    std::printf("           %s: %s, %d tracks, %d side(s), %zu sectors on track 0\n",
                d.name().c_str(), d.format_name().c_str(), d.tracks(), d.sides(),
                d.track(0, 0).size());
    std::string ids;
    for (const ein::DiskImage::Sector& s : d.track(0, 0)) {
        ids += std::to_string(s.id);
        ids += ' ';
    }
    std::printf("           track 0 physical order: %s\n", ids.c_str());

    check(d.format_name() == "Extended CPC DSK", "read as an Extended CPC DSK");
    check(d.tracks() == 40 && d.sides() == 1, "40 tracks, single sided");
    check(d.track(0, 0).size() == 10, "ten sectors per track");
    check(d.find(0, 0, 0) != nullptr && d.find(0, 0, 9) != nullptr,
          "sector ids run 0-9, as FBOOT reading sector 0 implies");
    check(d.find(0, 0, 0)->size == 512, "512-byte sectors");
    // 0 5 1 6 2 7 3 8 4 9: sequential ids land two slots apart, which is what
    // gives the CPU time to deal with one sector before the next arrives.
    check(d.slot_of(0, 0, 0) == 0 && d.slot_of(0, 0, 1) == 2 && d.slot_of(0, 0, 5) == 1,
          "a 2:1 physical interleave, which the controller's timing honours");

    // Boot it.
    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    m.fdc.disk(0).load(path);

    const std::uint64_t limit = 60ull * ein::Machine::kCpuClockHz;
    bool left_rom = false;
    std::uint64_t left_at = 0;
    while (m.tstates < limit && !m.cpu.halted) {
        m.step();
        if (!left_rom && !m.rom_enabled && m.cpu.pc.w >= 0x2000 && m.cpu.pc.w < 0xfb00) {
            left_rom = true;
            left_at = m.tstates;
        }
        if (left_rom && m.tstates > left_at + 15ull * ein::Machine::kCpuClockHz) break;
    }

    const std::vector<std::string> screen = read_screen(m, rom);
    std::printf("           Oh Mummy: pc=%04X, %llu sectors read, %llu RNF, %.1f s\n", m.cpu.pc.w,
                static_cast<unsigned long long>(m.fdc.sectors_read()),
                static_cast<unsigned long long>(m.fdc.not_found()),
                double(m.tstates) / ein::Machine::kCpuClockHz);
    print_screen(screen);

    // The game drives the VDP itself, with its own patterns rather than the MOS
    // font, so the screen is judged on pixels rather than on readable text.
    std::vector<std::uint32_t> fb(ein::Tms9129::kWidth * ein::Tms9129::kHeight);
    m.vdp.render(fb.data());
    const std::uint32_t bg = ein::Tms9129::palette(m.vdp.reg(7) & 0x0f);
    std::size_t lit = 0;
    for (std::uint32_t px : fb) {
        if (px != bg) ++lit;
    }
    static const char* kModes[] = {"Graphics I", "Graphics II", "Multicolour", "Text", "?"};
    std::printf("           VDP: %s, regs %02X %02X %02X %02X %02X %02X %02X %02X, "
                "%zu/%zu lit pixels, %llu VRAM writes\n",
                kModes[m.vdp.mode()], m.vdp.reg(0), m.vdp.reg(1), m.vdp.reg(2), m.vdp.reg(3),
                m.vdp.reg(4), m.vdp.reg(5), m.vdp.reg(6), m.vdp.reg(7), lit, fb.size(),
                static_cast<unsigned long long>(m.vdp.vram_writes()));

    check(left_rom, "the disc booted out of the ROM");
    check(m.fdc.sectors_read() > 14, "and kept loading from disc");
    check(m.fdc.not_found() == 0, "with no sector missing");
    check(lit > 20000, "and drew a screen of its own, not the MOS banner");
    check(m.vdp.vram_writes() > 50000, "with the VRAM traffic a running game makes");
}

// M6: AY-3-8910 sound.
//
// The clock is derived in ay38910.h from CTRLG, the BEL handler at
// Mos12.zsm:2679: one tone, period 142, everything else muted. These tests
// measure what actually comes out of the mixer rather than trusting the
// register file.

// Counts full cycles in a rendered buffer by looking for rising edges through
// the midpoint, and converts to Hz.
double measure_hz(const std::vector<float>& s) {
    if (s.size() < 2) return 0.0;
    float lo = s[0], hi = s[0];
    for (float v : s) {
        lo = std::min(lo, v);
        hi = std::max(hi, v);
    }
    if (hi - lo < 1e-4f) return 0.0;  // silence
    const float mid = (lo + hi) * 0.5f;

    int crossings = 0;
    std::size_t first = 0, last = 0;
    bool above = s[0] > mid;
    for (std::size_t i = 1; i < s.size(); ++i) {
        const bool now = s[i] > mid;
        if (now && !above) {
            if (crossings == 0) first = i;
            last = i;
            ++crossings;
        }
        above = now;
    }
    if (crossings < 2) return 0.0;
    const double cycles = crossings - 1;
    const double seconds = double(last - first) / ein::Ay38910::kSampleRate;
    return cycles / seconds;
}

std::vector<float> render(ein::Machine& m, double seconds) {
    m.psg.clear_samples();
    const int want = static_cast<int>(ein::Ay38910::kSampleRate * seconds);
    std::vector<float> out;
    out.reserve(want);
    float buf[1024];
    while (static_cast<int>(out.size()) < want) {
        m.tick(4096);
        int n;
        while ((n = m.psg.read_samples(buf, 1024)) > 0) {
            out.insert(out.end(), buf, buf + n);
        }
    }
    out.resize(want);
    return out;
}

void poke(ein::Machine& m, int reg, std::uint8_t value) {
    m.io_write(0x02, static_cast<std::uint8_t>(reg));
    m.io_write(0x03, value);
}

// The PSG output runs through a low-pass (Ay38910::kDefaultLowpassHz), so a
// step change in the registers reaches its new steady state over a fraction of
// a millisecond instead of instantly. Steady-state checks skip past that; 5 ms
// is two hundred times the filter's time constant and still a tenth of the
// shortest buffer rendered here.
constexpr int kSettleSamples = ein::Ay38910::kSampleRate / 200;

void test_sound() {
    ein::Machine m;
    m.rom_enabled = false;

    // Exactly what CTRLG does (Mos12.zsm:2679): tone A period 008E, amplitude
    // 10, mixer 7E -- tone A on, everything else off, port A out, port B in.
    poke(m, 0, 0x8e);
    poke(m, 1, 0x00);
    poke(m, 8, 0x0a);
    poke(m, 7, 0x7e);

    const std::vector<float> beep = render(m, 0.25);
    const double hz = measure_hz(beep);
    const double want = double(ein::Ay38910::kClockHz) / (16.0 * 142.0);
    std::printf("           BEL tone: %.1f Hz measured, %.1f Hz expected (A5 = 880.0)\n", hz, want);
    check(std::abs(hz - want) < want * 0.01, "the BEL tone comes out at clock/(16*period)");
    check(std::abs(hz - 880.0) < 10.0, "which is an A, as CTRLG's period 142 was chosen for");

    // Amplitude 0 must be silent, and the mixer must actually gate.
    poke(m, 8, 0x00);
    const std::vector<float> quiet = render(m, 0.05);
    float peak = 0.0f;
    for (std::size_t i = kSettleSamples; i < quiet.size(); ++i) {
        peak = std::max(peak, std::abs(quiet[i]));
    }
    check(peak < 1e-4f, "amplitude 0 is silence");

    // Tone and noise both disabled does NOT mean silent: the channel sits at a
    // constant level set by the amplitude register. That is not a quirk to work
    // around, it is how AY machines play digitised samples -- the music driver
    // rewrites the volume at the sample rate.
    poke(m, 8, 0x0f);
    poke(m, 7, 0x7f);  // every tone and noise disabled
    const std::vector<float> gated = render(m, 0.05);
    float gmin = gated[kSettleSamples], gmax = gated[kSettleSamples];
    for (std::size_t i = kSettleSamples; i < gated.size(); ++i) {
        gmin = std::min(gmin, gated[i]);
        gmax = std::max(gmax, gated[i]);
    }
    check(gmax - gmin < 1e-4f && gmax > 0.01f,
          "tone and noise both disabled holds a steady DC level, which is how samples play");

    // Pitch tracks the period across the range.
    struct Case { int period; };
    static const Case kCases[] = {{71}, {142}, {284}, {568}};
    bool all_ok = true;
    for (const Case& c : kCases) {
        poke(m, 7, 0x7e);
        poke(m, 8, 0x0f);
        poke(m, 0, static_cast<std::uint8_t>(c.period & 0xff));
        poke(m, 1, static_cast<std::uint8_t>((c.period >> 8) & 0x0f));
        const double got = measure_hz(render(m, 0.25));
        const double expect = double(ein::Ay38910::kClockHz) / (16.0 * c.period);
        if (std::abs(got - expect) > expect * 0.01) {
            std::printf("           period %d: %.1f Hz, wanted %.1f Hz\n", c.period, got, expect);
            all_ok = false;
        }
    }
    check(all_ok, "tone frequency tracks the period over four octaves");

    // Noise: enabled on its own it must be neither silent nor periodic.
    poke(m, 0, 0x00);
    poke(m, 1, 0x00);
    poke(m, 6, 0x10);  // noise period
    poke(m, 7, 0x77);  // noise on channel A only (bit 3 clear), all tones off
    poke(m, 8, 0x0f);
    const std::vector<float> noise = render(m, 0.25);
    float nmin = noise[0], nmax = noise[0];
    for (float v : noise) {
        nmin = std::min(nmin, v);
        nmax = std::max(nmax, v);
    }
    // A periodic square wave would give a strongly peaked crossing count; noise
    // gives a broad one. Just check it swings and is not a clean tone.
    const double noise_hz = measure_hz(noise);
    std::printf("           noise: swing %.3f..%.3f, %.0f crossings/s\n", nmin, nmax, noise_hz);
    check(nmax - nmin > 0.05f, "the noise generator actually swings");

    // The output low-pass. Two tones, one comfortably under the default cutoff
    // and one well over it, each rendered with the filter off and on. The low
    // one has to come through intact -- a filter that dulls the music is not
    // worth having -- and the high one has to go.
    auto swing = [&](int period, float cutoff) {
        m.psg.set_lowpass(cutoff);
        poke(m, 6, 0x00);
        poke(m, 7, 0x7e);  // tone A only
        poke(m, 8, 0x0f);
        poke(m, 0, static_cast<std::uint8_t>(period & 0xff));
        poke(m, 1, static_cast<std::uint8_t>((period >> 8) & 0x0f));
        const std::vector<float> v = render(m, 0.05);
        float lo = v[kSettleSamples], hi = v[kSettleSamples];
        for (std::size_t i = kSettleSamples; i < v.size(); ++i) {
            lo = std::min(lo, v[i]);
            hi = std::max(hi, v[i]);
        }
        return hi - lo;
    };
    // Pinned to 6 kHz rather than the default, because what is on trial here is
    // the filter, not which cutoff ships.
    const float kCut = 6000.0f;
    const float low_off = swing(40, 0.0f);   // 3.1 kHz -- under the cutoff
    const float low_on = swing(40, kCut);
    const float high_off = swing(8, 0.0f);   // 15.6 kHz -- well over it
    const float high_on = swing(8, kCut);
    std::printf("           low-pass at %.0f Hz: 3.1 kHz %.3f -> %.3f, 15.6 kHz %.3f -> %.3f\n",
                kCut, low_off, low_on, high_off, high_on);
    check(low_on > low_off * 0.6f, "the low-pass leaves a 3 kHz tone essentially alone");
    check(high_on < high_off * 0.35f, "and takes a 15.6 kHz whistle down by 9 dB and more");

    // The shipping default is the light setting. It has to be inaudible on the
    // music, or every game gets quietly dulled to fix a few.
    const float dflt = swing(40, ein::Ay38910::kDefaultLowpassHz);
    std::printf("           default %.0f Hz: 3.1 kHz %.3f -> %.3f\n",
                ein::Ay38910::kDefaultLowpassHz, low_off, dflt);
    check(dflt > low_off * 0.9f, "and the default cutoff barely touches a 3 kHz tone at all");

    m.psg.set_lowpass(0.0f);
    const float unfiltered = swing(8, 0.0f);
    check(std::abs(unfiltered - high_off) < high_off * 0.05f,
          "and switching it off restores what the chip actually generates");
}

// The envelope shapes, against the diagrams in the datasheet. Each is driven
// one step at a time and the level sequence read back.
void test_envelope() {
    ein::Machine m;
    m.rom_enabled = false;

    auto run_shape = [&](std::uint8_t shape, int steps) {
        poke(m, 11, 0x01);  // shortest envelope period
        poke(m, 12, 0x00);
        poke(m, 13, shape);
        std::vector<int> levels;
        levels.push_back(m.psg.envelope_level());
        // One envelope step is clock/16 * period = 32 CPU T-states here.
        for (int i = 0; i < steps; ++i) {
            m.tick(32);
            levels.push_back(m.psg.envelope_level());
        }
        return levels;
    };

    // Shape 0C (CONT ATT, no ALT/HOLD): a repeating upward ramp.
    std::vector<int> up = run_shape(0x0c, 40);
    check(up[0] == 0 && up[1] == 1 && up[15] == 15,
          "shape 0C ramps up 0..15");
    check(up[16] == 0 && up[17] == 1, "and repeats from 0");

    // Shape 08 (CONT only): a repeating downward ramp.
    std::vector<int> down = run_shape(0x08, 40);
    check(down[0] == 15 && down[1] == 14 && down[15] == 0, "shape 08 ramps down 15..0");
    check(down[16] == 15, "and repeats from 15");

    // Shape 0E (CONT ATT ALT): a triangle.
    std::vector<int> tri = run_shape(0x0e, 40);
    check(tri[0] == 0 && tri[15] == 15, "shape 0E ramps up");
    check(tri[16] == 15 && tri[17] == 14 && tri[31] == 0, "then back down");

    // Shape 0B (CONT ALT HOLD): down once, then hold high.
    std::vector<int> down_hold = run_shape(0x0b, 40);
    check(down_hold[15] == 0, "shape 0B ramps down");
    check(down_hold[16] == 15 && down_hold[39] == 15, "then holds at 15");

    // Shape 0D (CONT ATT HOLD): up once, then hold high.
    std::vector<int> up_hold = run_shape(0x0d, 40);
    check(up_hold[15] == 15 && up_hold[16] == 15 && up_hold[39] == 15,
          "shape 0D ramps up and holds at 15");

    // Shape 09 (CONT HOLD): down once, then silence -- and every shape with
    // CONT clear behaves the same way.
    std::vector<int> down_off = run_shape(0x09, 40);
    check(down_off[15] == 0 && down_off[39] == 0, "shape 09 ramps down and holds at 0");
    for (std::uint8_t s = 0; s < 4; ++s) {
        std::vector<int> v = run_shape(s, 24);
        if (!(v[0] == 15 && v[15] == 0 && v[23] == 0)) {
            check(false, "shapes 00-03 all behave as a single downward ramp");
            return;
        }
    }
    check(true, "shapes 00-03 all behave as a single downward ramp");
    for (std::uint8_t s = 4; s < 8; ++s) {
        std::vector<int> v = run_shape(s, 24);
        if (!(v[0] == 0 && v[15] == 15 && v[16] == 0 && v[23] == 0)) {
            check(false, "shapes 04-07 all ramp up once and then go quiet");
            return;
        }
    }
    check(true, "shapes 04-07 all ramp up once and then go quiet");

    // Writing R13 retriggers, which is how music drivers restart a note.
    poke(m, 13, 0x0c);
    for (int i = 0; i < 8; ++i) m.tick(32);
    const int mid = m.psg.envelope_level();
    poke(m, 13, 0x0c);
    check(mid > 0 && m.psg.envelope_level() == 0, "rewriting R13 restarts the envelope");
}

// The real thing: a commercial title's music driver.
//
// Pitch is checked against equal temperament rather than by ear. If the clock
// or the tone formula were wrong by anything other than a factor of two, the
// notes the game plays would not land on semitones.
void test_game_sound(const ein::RomImage& rom, const std::filesystem::path& disks) {
    const auto path = disks / "OHMUMMY.DSK";
    if (!std::filesystem::exists(path)) {
        std::printf("[ SKIP ]  OHMUMMY.DSK not present\n");
        return;
    }

    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    m.fdc.disk(0).load(path);

    float peak = 0.0f;
    std::uint64_t drained = 0;
    std::vector<int> periods;
    float buf[2048];
    const std::uint64_t seconds = 60;
    const std::uint64_t limit = seconds * ein::Machine::kCpuClockHz;
    long long since_sample = 0;
    while (m.tstates < limit && !m.cpu.halted) {
        m.step();
        if (++since_sample >= 512) {
            since_sample = 0;
            for (int ch = 0; ch < 3; ++ch) {
                // Only channels the mixer has actually enabled are notes.
                if (m.psg.reg(7) & (1u << ch)) continue;
                if (m.psg.channel_level(ch) == 0) continue;
                const int p = m.psg.tone_period(ch);
                if (p > 1 && std::find(periods.begin(), periods.end(), p) == periods.end()) {
                    periods.push_back(p);
                }
            }
        }
        // Drain the way the frontend does, so nothing is ever dropped.
        int n;
        while ((n = m.psg.read_samples(buf, 2048)) > 0) {
            for (int i = 0; i < n; ++i) peak = std::max(peak, std::abs(buf[i]));
            drained += n;
        }
    }

    // Tempo comes straight from the clock: the PSG must produce exactly
    // kSampleRate samples for every emulated second, no more and no less.
    const std::uint64_t want = seconds * ein::Ay38910::kSampleRate;
    const long long err = static_cast<long long>(drained) - static_cast<long long>(want);
    std::printf("           Oh Mummy: %llu samples in %llu s (%+lld vs %llu), peak %.3f, "
                "%llu dropped\n",
                static_cast<unsigned long long>(drained),
                static_cast<unsigned long long>(seconds), err,
                static_cast<unsigned long long>(want), peak,
                static_cast<unsigned long long>(m.psg.samples_dropped()));
    check(std::llabs(err) < static_cast<long long>(ein::Ay38910::kSampleRate) / 100,
          "the PSG produces 48000 samples per emulated second, so tempo follows the clock");
    check(m.psg.samples_dropped() == 0, "and nothing was dropped while it was drained");
    check(peak > 0.01f, "with something audible coming out of the mixer");

    // Every note the game plays should land on a semitone.
    std::sort(periods.begin(), periods.end());
    int on_pitch = 0;
    std::string worst_note;
    double worst = 0.0;
    for (int p : periods) {
        const double hz = double(ein::Ay38910::kClockHz) / (16.0 * p);
        if (hz < 40.0 || hz > 5000.0) continue;  // outside the musical range
        const double semis = 12.0 * std::log2(hz / 440.0);
        const double cents = 100.0 * (semis - std::round(semis));
        if (std::abs(cents) < 25.0) ++on_pitch;
        if (std::abs(cents) > std::abs(worst)) worst = cents;
    }
    std::printf("           %zu distinct tone periods; %d land on a semitone "
                "(worst %+.1f cents)\n",
                periods.size(), on_pitch, worst);
    check(!periods.empty(), "the game played notes");
    check(on_pitch * 4 >= static_cast<int>(periods.size()) * 3,
          "and they are equal-tempered pitches, which the clock and formula have to be right for");
}

// M7: the remaining peripherals -- Z80 PIO (printer), 8251 serial, ADC0844,
// and the interrupt daisy chain that now has more than one device on it.

// Plants a short program in RAM and runs it. ROM 0008 is JP L0FC2
// (Mos12.zsm:195), so MCALs work from anywhere while the overlay is in.
void plant(ein::Machine& m, std::uint16_t addr, std::initializer_list<std::uint8_t> code) {
    std::uint16_t a = addr;
    for (std::uint8_t b : code) m.write(a++, b);
}

void boot_mos(ein::Machine& m, const ein::RomImage& rom) {
    m.set_rom(rom);
    m.reset();
    for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();
}

// The printer hangs off PIO port A. ZPOUT (Mos12.zsm:2661) writes the byte and
// then spins on IFLAG bit 0, which only the port A interrupt handler at LFC84
// clears -- so a working print proves the PIO, the vector MOS gave it, the
// acknowledge strobe and the daisy chain all at once.
void test_printer(const ein::RomImage& rom) {
    ein::Machine m;
    boot_mos(m, rom);

    check(m.pio.vector(0) == 0x10, "MOS gave PIO port A vector 10 (IPRT at FB10)");
    check(m.pio.vector(1) == 0x12, "and port B vector 12");
    check(m.pio.mode(0) == ein::Z80Pio::kOutput, "port A is a mode 0 output");
    check(m.pio.int_enabled(0), "with interrupts enabled");

    // LD A,'H' / MCAL ZPOUT, three times, then halt.
    plant(m, 0x8000,
          {0x3e, 'H', 0xcf, 0x9f,
           0x3e, 'i', 0xcf, 0x9f,
           0x3e, '!', 0xcf, 0x9f,
           0x76, 0x18, 0xfe});  // HALT, then spin
    m.cpu.pc.w = 0x8000;
    m.cpu.halted = false;

    for (int i = 0; i < 2'000'000 && !m.cpu.halted; ++i) m.step();
    const bool finished = m.cpu.halted;
    // The last byte's acknowledge is still in flight when the program halts;
    // give it time to arrive and be handled.
    for (int i = 0; i < 50'000; ++i) m.step();

    std::string got(m.printer.begin(), m.printer.end());
    std::printf("           printer received %zu bytes: \"%s\", %llu strobes\n", m.printer.size(),
                got.c_str(), static_cast<unsigned long long>(m.pio.strobes(0)));
    check(finished, "the program ran to completion, so ZPOUT never hung");
    check(got == "Hi!", "MOS printed through the PIO and the printer got every byte");
    check(m.pio.strobes(0) >= 3, "each byte was acknowledged with a strobe");
    check((m.read(0xfb92) & 1) == 0, "and IFLAG bit 0 was cleared by the port A handler");
}

// The 8251's bit rate is not configured in the chip at all: it comes from CTC
// channels 0 and 1, which BAUD programmes from the table at Mos12.zsm:686.
void test_serial(const ein::RomImage& rom) {
    ein::Machine m;
    boot_mos(m, rom);

    check(m.uart.mode() == 0xce, "MOS set 8251 mode CE: x16, 8 data bits, 2 stop bits");
    check(m.uart.command() == 0x27, "and command 27: transmit and receive enabled");
    check(m.uart.clock_divisor() == 16, "the mode word asks for a x16 clock");
    check(m.uart.data_bits() == 8, "and 8 data bits");

    // Measure what the CTC is actually feeding the transmitter. MOS set up
    // 9600 baud at reset, which the table says is really 9615.38.
    const std::uint64_t before = m.uart.tx_clocks();
    const std::uint64_t t0 = m.tstates;
    while (m.tstates < t0 + ein::Machine::kCpuClockHz) m.step();
    const double clocks = double(m.uart.tx_clocks() - before);
    const double baud = clocks / 16.0;
    std::printf("           transmit clock %.0f Hz -> %.1f baud (table says 9615.38)\n", clocks,
                baud);
    check(std::abs(baud - 9615.38) < 20.0,
          "the CTC delivers 9615 baud, matching the table's own arithmetic");

    // Loop TxD back to RxD and push a character through MOS's own routines.
    m.uart.loopback = true;
    m.uart.clear_sent();
    plant(m, 0x8000,
          {0x3e, 'Z', 0xcf, 0xa0,   // LD A,'Z' / MCAL ZSLOUT
           0xcf, 0xa1,              // MCAL ZSRLIN -> A
           0x32, 0x00, 0x90,        // LD (9000),A
           0x76});                  // HALT
    m.cpu.pc.w = 0x8000;
    m.cpu.halted = false;
    for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();

    std::printf("           serial loopback: sent %zu byte(s), read back %02X\n",
                m.uart.sent().size(), m.read(0x9000));
    check(m.cpu.halted, "ZSLOUT and ZSRLIN both completed");
    check(m.uart.sent().size() == 1 && m.uart.sent().front() == 'Z', "the byte went out");
    check(m.read(0x9000) == 'Z', "and came back in through the receiver");
}

// ADC0844: write selects a channel and starts a conversion, read takes the
// result. Nothing in MOS drives it, so this is the datasheet's behaviour.
void test_adc() {
    ein::Machine m;
    m.rom_enabled = false;

    m.adc.set_axis(0, 0x20);
    m.adc.set_axis(1, 0xC0);

    m.io_write(0x38, 0x00);  // select channel 0
    check(m.adc.converting(), "writing the ADC starts a conversion");
    m.tick(ein::Adc0844::kConversionTStates / 2);
    check(m.adc.converting(), "which takes time -- about 40 us");
    m.tick(ein::Adc0844::kConversionTStates);
    check(!m.adc.converting(), "and then completes");
    check(m.io_read(0x38) == 0x20, "channel 0 reads back its axis value");

    m.io_write(0x39, 0x01);  // 38-3F all decode to the same chip
    m.tick(ein::Adc0844::kConversionTStates * 2);
    check(m.io_read(0x3f) == 0xC0, "channel 1 reads back its own");

    m.io_write(0x21, 0x01);
    m.io_write(0x25, 0x01);
    check(m.adc_mask == 0x01 && m.fire_mask == 0x01,
          "the ADC and fire-button interrupt masks latch (ports 21 and 25)");
}

// Two devices on the chain now. A device being serviced holds IEO low, so the
// PIO cannot interrupt while the CTC is still in its handler.
void test_daisy_chain() {
    ein::Machine m;
    m.rom_enabled = false;

    // PIO port A: vector 10, output mode, interrupts on.
    m.io_write(0x31, 0x10);
    m.io_write(0x31, 0x0f);
    m.io_write(0x31, 0x87);
    // CTC: vector base 00, channel 3 as a counter with an interrupt.
    m.io_write(0x28, 0x00);
    m.io_write(0x2b, 0xdf);
    m.io_write(0x2b, 0x01);

    // Vector table at FB00, and EI/RETI at both handlers.
    m.write(0xfb06, 0x00);
    m.write(0xfb07, 0x90);  // CTC3 -> 9000
    m.write(0xfb10, 0x00);
    m.write(0xfb11, 0x91);  // PIO A -> 9100
    plant(m, 0x9000, {0xfb, 0xed, 0x4d});  // EI / RETI
    plant(m, 0x9100, {0xfb, 0xed, 0x4d});
    for (std::uint16_t a = 0x8000; a < 0x8020; ++a) m.write(a, 0x00);  // NOPs

    m.cpu.pc.w = 0x8000;
    m.cpu.sp.w = 0x8f00;
    m.cpu.i = 0xfb;
    m.cpu.im = 2;
    m.cpu.iff1 = m.cpu.iff2 = true;

    // Raise both at once.
    m.pio.strobe(0);
    m.ctc.trigger(3);
    check(m.ctc.int_pending() && m.pio.int_pending(), "both devices are asking");

    m.step();
    check(m.cpu.pc.w == 0x9000, "the CTC wins: it is ahead of the PIO in the chain");
    check(m.ctc.int_in_service(), "and is now in service");
    check(m.pio.int_pending(), "the PIO is still asking");

    // The real test of the chain: with interrupts fully enabled again and the
    // PIO still requesting, it must *not* be accepted, because the CTC in
    // service holds IEO low. The CPU should just execute the EI at 9000.
    m.cpu.iff1 = true;
    m.cpu.ei_pending = false;
    m.step();
    check(m.cpu.pc.w == 0x9001,
          "the PIO is locked out while the CTC is in service -- IEO, not luck");

    m.step();  // RETI -> releases the CTC
    check(!m.ctc.int_in_service(), "RETI released the CTC");
    check(m.cpu.pc.w == 0x8000, "and returned to the interrupted code");

    // Now the PIO gets in, on the very next instruction boundary.
    m.step();
    check(m.cpu.pc.w == 0x9100, "now the PIO is serviced");
    check(m.pio.int_in_service(), "and holds the chain until its own RETI");
    m.step();  // EI
    m.step();  // RETI
    check(!m.pio.int_in_service(), "which releases it");
    check(!m.ctc.int_pending() && !m.pio.int_pending(), "nothing left pending");
}

// M8: the debugger.

void test_disasm() {
    // A byte buffer standing in for memory.
    std::vector<std::uint8_t> mem(0x10000, 0);
    auto rd = [&](std::uint16_t a) { return mem[a]; };

    struct Case {
        std::vector<std::uint8_t> bytes;
        const char* text;
        int length;
    };
    static const std::vector<Case> kCases = {
        {{0x00}, "NOP", 1},
        {{0x21, 0x00, 0x80}, "LD   HL,8000H", 3},
        {{0x11, 0x01, 0x80}, "LD   DE,8001H", 3},
        {{0x18, 0x38}, "JR   013AH", 2},
        {{0xcf}, "RST  08H", 1},
        {{0xd3, 0x24}, "OUT  (24H),A", 2},
        {{0xdb, 0x18}, "IN   A,(18H)", 2},
        {{0xed, 0x4d}, "RETI", 2},
        {{0xed, 0x45}, "RETN", 2},
        {{0xed, 0x5e}, "IM   2", 2},
        {{0xed, 0xb0}, "LDIR", 2},
        {{0xed, 0xb8}, "LDDR", 2},
        {{0xed, 0xa2}, "INI", 2},
        {{0xcb, 0x7e}, "BIT  7,(HL)", 2},
        {{0xcb, 0x30}, "SLL  B", 2},          // undocumented
        {{0xdd, 0x7e, 0x05}, "LD   A,(IX+5)", 3},
        {{0xfd, 0x36, 0xfe, 0x11}, "LD   (IY-2),11H", 4},
        {{0xdd, 0x24}, "INC  IXH", 2},        // undocumented
        {{0xfd, 0x2d}, "DEC  IYL", 2},        // undocumented
        {{0xdd, 0xcb, 0x02, 0x06}, "RLC  (IX+2)", 4},
        {{0xdd, 0xcb, 0x02, 0xc6}, "SET  0,(IX+2)", 4},
        {{0x76}, "HALT", 1},
        {{0xc3, 0x34, 0x12}, "JP   1234H", 3},
        {{0xcd, 0xc2, 0x0f}, "CALL 0FC2H", 3},
        {{0x08}, "EX   AF,AF'", 1},
        {{0xd9}, "EXX", 1},
        {{0xe3}, "EX   (SP),HL", 1},
        {{0xdd, 0xe3}, "EX   (SP),IX", 2},
        {{0x3a, 0x00, 0xfb}, "LD   A,(0FB00H)", 3},
        {{0xed, 0x53, 0x00, 0xfb}, "LD   (0FB00H),DE", 4},
        {{0xfb}, "EI", 1},
        {{0xf3}, "DI", 1},
        {{0x10, 0xfe}, "DJNZ 0100H", 2},
    };

    int bad = 0;
    for (const Case& c : kCases) {
        std::fill(mem.begin(), mem.end(), 0);
        for (std::size_t i = 0; i < c.bytes.size(); ++i) mem[0x0100 + i] = c.bytes[i];
        const ein::Instruction ins = ein::disassemble(rd, 0x0100);
        if (ins.text != c.text || ins.length != c.length) {
            std::printf("           got \"%s\" (%d bytes), wanted \"%s\" (%d)\n", ins.text.c_str(),
                        ins.length, c.text, c.length);
            ++bad;
        }
    }
    check(bad == 0, "the disassembler decodes prefixes, undocumented ops and all");

    // Lengths must agree with what the CPU actually consumes, across every
    // opcode -- that is what keeps the source walk in step.
    ein::Machine m;
    m.rom_enabled = false;
    int mismatches = 0;
    for (int op = 0; op < 256; ++op) {
        for (int second = 0; second < 256; ++second) {
            // Only the prefixes need the two-byte sweep.
            if (op != 0xcb && op != 0xed && op != 0xdd && op != 0xfd && second != 0) break;
            std::uint16_t a = 0x0200;
            m.write(a, static_cast<std::uint8_t>(op));
            m.write(static_cast<std::uint16_t>(a + 1), static_cast<std::uint8_t>(second));
            for (int i = 2; i < 6; ++i) m.write(static_cast<std::uint16_t>(a + i), 0x00);
            auto rd2 = [&](std::uint16_t x) { return m.peek(x); };
            const int want = ein::instruction_length(rd2, a);

            m.cpu.reset();
            m.cpu.pc.w = a;
            m.cpu.sp.w = 0x8000;
            m.cpu.step(m);
            // HALT parks PC on its own opcode on purpose, so it has no length
            // to compare against.
            if (m.cpu.halted) continue;
            // HALT does not advance; interrupts and jumps move elsewhere.
            const int got = static_cast<int>((m.cpu.pc.w - a) & 0xffff);
            if (got != want && got > 0 && got <= 6) {
                ++mismatches;
                if (mismatches <= 8) {
                    std::printf("           %02X %02X: CPU used %d bytes, disassembler said %d\n",
                                op, second, got, want);
                }
            }
        }
    }
    std::printf("           opcode length sweep: %d disagreements with the CPU\n", mismatches);
    check(mismatches == 0, "and its instruction lengths match what the CPU consumes");
}

// The point of the whole exercise: Hohne's labels and comments, on addresses.
void test_symbols(const std::filesystem::path& roms, const ein::RomImage& rom) {
    const auto dir = roms / "disassembled" / "Einstein";
    if (!std::filesystem::exists(dir / "__MOS12" / "Mos12.zsm")) {
        std::printf("[ SKIP ]  sources not found\n");
        return;
    }

    ein::Symbols sym;
    sym.load_equates(dir / "__MOS12" / "Einstein.zsm");
    sym.load_equates(dir / "__MOS12" / "MOSEQU.GEN");

    // Mos12.zsm is the ROM itself, so the ROM image is the byte source.
    auto rom_read = [&](std::uint16_t a) -> std::uint8_t {
        return a < rom.data.size() ? rom.data[a] : 0xff;
    };
    sym.load_source(dir / "__MOS12" / "Mos12.zsm", rom_read);

    const int anchors = sym.anchors();
    const int matched = sym.anchors_matched();
    std::printf("           Mos12.zsm: %zu lines mapped, %d/%d anchors agreed (%.1f%%), "
                "%zu equates\n",
                sym.line_count(), matched, anchors,
                anchors ? 100.0 * matched / anchors : 0.0, sym.equate_count());

    check(anchors > 300, "the source is full of Lxxxx labels to anchor on");
    // Every anchor that agrees is an independent confirmation that the walk
    // tracked the real instruction stream over the stretch before it.
    check(matched * 100 >= anchors * 95,
          "the walk lands on the address the next label declares, over and over");

    // Named labels get their addresses from the walk, so they are the real test
    // of it -- and the ROM carries an independent answer for a whole set of
    // them. RST 08 dispatches through a table of routine addresses at MOSITP
    // (0282, from UPPERMEM.ZSM), indexed by the MCAL number with bit 7 stripped
    // (Mos12.zsm:1013). So for every MCAL, the ROM itself says where the
    // routine lives, and the walk has to agree -- at eleven addresses spread
    // right across the image.
    struct Mcal { const char* name; int number; };
    static const Mcal kMcals[] = {
        {"ZKEYIN", 0x9c}, {"ZPOUT", 0x9f},  {"ZSLOUT", 0xa0}, {"ZSRLIN", 0xa1},
        {"ZRSECT", 0xa2}, {"ZWSECT", 0xa3}, {"ZMOUT", 0xb4},  {"ZKSCAN", 0xb5},
        {"ZZTIME", 0xbc}, {"ZPINIT", 0xc0}, {"ZVOUT", 0xd0},
    };
    int wrong = 0;
    for (const Mcal& k : kMcals) {
        const int slot = 0x0282 + 2 * (k.number & 0x7f);
        const int from_rom = rom.data[slot] | (rom.data[slot + 1] << 8);
        const int from_source = sym.lookup(k.name);
        if (from_rom != from_source) {
            std::printf("           %-7s MCAL %02X: ROM table says %04X, source walk says %04X\n",
                        k.name, k.number, from_rom, from_source);
            ++wrong;
        }
    }
    check(wrong == 0,
          "every MCAL label lands where the ROM's own dispatch table points");

    // ROM 0008 is JP L0FC2, so the dispatcher's address is checkable from the
    // bytes themselves rather than from the source.
    const int dispatcher = rom.data[0x0009] | (rom.data[0x000a] << 8);
    check(sym.lookup("L0FC2") == dispatcher,
          "and agree with the JP at ROM 0008, which is the RST 08 vector");

    // Spot-check that comments arrived with them.
    const ein::Symbols::Line* l = sym.line_at(0x0fae);
    if (l) {
        std::printf("           %04X  %-22s ; %s\n", l->addr, l->text.c_str(),
                    l->comment.c_str());
    }
    check(l != nullptr && l->label == "ZZTIME", "ZZTIME's own source line is at its address");

    // The trampolines MOS copies into RAM are documented in UPPERMEM.ZSM. Their
    // bytes live in the ROM at the source of the copy (MEMORY.TXT), so the walk
    // is pointed there.
    ein::Symbols upper;
    upper.load_equates(dir / "__MOS12" / "Einstein.zsm");
    auto copied = [&](std::uint16_t a) -> std::uint8_t {
        if (a >= 0xfb00 && a <= 0xfb45) return rom.data[0x0ff0 + (a - 0xfb00)];
        if (a >= 0xfc09 && a <= 0xfc8c) return rom.data[0x1041 + (a - 0xfc09)];
        if (a <= 0x000a) return rom.data[0x1036 + a];
        return 0xff;
    };
    upper.load_source(dir / "__MOS12" / "UPPERMEM.ZSM", copied);
    std::printf("           UPPERMEM.ZSM: %zu lines mapped, %d/%d anchors agreed\n",
                upper.line_count(), upper.anchors_matched(), upper.anchors());
    check(upper.lookup("LFC47") == 0xfc47, "the RTC interrupt handler is at FC47");
    check(upper.line_at(0xfc47) != nullptr,
          "so single-stepping the RAM trampolines shows Hohne's comments too");
}

void test_breakpoints(const ein::RomImage& rom) {
    ein::Machine m;
    m.set_rom(rom);
    m.reset();

    // Execute: stop at the instruction that enables interrupt mode 2.
    m.set_breakpoint(0x0090, ein::Machine::kBpExec);
    int guard = 0;
    while (!m.bp_hit() && guard++ < 3'000'000) m.step();
    check(m.bp_hit(), "an execute breakpoint fires");
    check(m.cpu.pc.w == 0x0090, "and stops before the instruction, not after");
    check(m.bp_hit_kind() == ein::Machine::kBpExec, "reporting what kind it was");

    // Continuing has to get past it rather than firing again immediately.
    m.resume_from_breakpoint();
    m.step();
    check(!m.bp_hit() && m.cpu.pc.w != 0x0090, "and continuing steps past it");

    // Write: MOS keeps its real-time clock at FB8C.
    m.clear_all_breakpoints();
    // FB8C is the hours digit; the seconds tick at RTCS+1 = FB91 every second.
    m.set_breakpoint(0xfb91, ein::Machine::kBpWrite);
    guard = 0;
    while (!m.bp_hit() && guard++ < 3'000'000) m.step();
    check(m.bp_hit() && m.bp_hit_kind() == ein::Machine::kBpWrite,
          "a write breakpoint catches the clock being set");
    check(m.bp_hit_addr() == 0xfb91, "at the address written");

    // Port: the VDP register port.
    m.clear_all_breakpoints();
    m.set_port_breakpoint(0x09, ein::Machine::kBpOut);
    guard = 0;
    while (!m.bp_hit() && guard++ < 3'000'000) m.step();
    check(m.bp_hit() && m.bp_hit_is_port(), "a port breakpoint catches an OUT");
    check(m.bp_hit_addr() == 0x09, "on port 09, the VDP control port");

    m.clear_all_breakpoints();
    check(m.breakpoint_count() == 0, "and they can all be cleared again");

    // peek() is what the debugger's own views use, and must not trip anything.
    m.set_breakpoint(0x1234, ein::Machine::kBpRead);
    m.resume_from_breakpoint();
    (void)m.peek(0x1234);
    check(!m.bp_hit(), "the debugger's own reads do not trip breakpoints");
}

// Boot to the banner and hash the screen. This is the regression gate: if
// anything in the CPU, the VDP or the timing changes what MOS draws, the hash
// moves and the test says so.
// HALT parks PC on its own opcode so the CPU re-fetches it while halted, which
// means accepting an interrupt has to step past it. Get that wrong and the
// address pushed is the HALT itself, so the program halts again forever -- the
// classic "wait for the frame interrupt" loop would never make progress.
void test_halt_wakeup() {
    ein::Machine m;
    m.rom_enabled = false;

    // 8000: HALT / INC A / JR $
    plant(m, 0x8000, {0x76, 0x3c, 0x18, 0xfe});
    // Interrupt vector FB06 -> 9000, handler is just EI / RETI.
    m.write(0xfb06, 0x00);
    m.write(0xfb07, 0x90);
    plant(m, 0x9000, {0xfb, 0xed, 0x4d});

    m.cpu.pc.w = 0x8000;
    m.cpu.sp.w = 0x8f00;
    m.cpu.i = 0xfb;
    m.cpu.im = 2;
    m.cpu.iff1 = m.cpu.iff2 = true;
    m.cpu.af.b.h = 0;

    m.step();
    check(m.cpu.halted, "the CPU halted");

    // Channel 3 raises one interrupt. Channels 0-2 take the 2 MHz CLK/TRG, so
    // a counter there with a time constant of 1 would re-interrupt every two
    // T-states and the CPU would never get an instruction in edgeways.
    m.io_write(0x28, 0x00);
    m.io_write(0x2b, 0xdf);
    m.io_write(0x2b, 0x01);
    m.ctc.trigger(3);
    for (int i = 0; i < 20 && m.cpu.pc.w != 0x8002; ++i) m.step();

    check(!m.cpu.halted, "the interrupt woke it");
    check(m.cpu.af.b.h == 1, "and it carried on past the HALT instead of re-halting");
}

// Save states. The proof is not that the bytes match, it is that the machine
// carries on identically: snapshot, run on, restore, run the same distance
// again, and every visible thing has to agree.
void test_save_state(const ein::RomImage& rom, const std::filesystem::path& disks) {
    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    const auto disk = disks / "OHMUMMY.DSK";
    const bool have_disk = std::filesystem::exists(disk);
    if (have_disk) m.fdc.disk(0).load(disk);

    // Get somewhere interesting -- past the boot, with devices busy.
    for (int i = 0; i < 3'000'000 && !m.cpu.halted; ++i) m.step();

    const std::vector<std::uint8_t> snap = m.save_state();
    std::printf("           save state is %zu bytes\n", snap.size());
    check(snap.size() > 65536, "a save state holds at least the 64K of RAM");

    // Run on, recording what the machine looks like after a fixed distance.
    auto fingerprint = [&](ein::Machine& mm) {
        std::vector<std::uint32_t> fb(ein::Tms9129::kWidth * ein::Tms9129::kHeight);
        mm.vdp.render(fb.data());
        std::string s = ein::md5_hex(reinterpret_cast<const char*>(fb.data()),
                                     fb.size() * sizeof(std::uint32_t));
        char buf[160];
        std::snprintf(buf, sizeof buf, "%s pc=%04X sp=%04X af=%04X hl=%04X t=%llu i=%llu", s.c_str(),
                      mm.cpu.pc.w, mm.cpu.sp.w, mm.cpu.af.w, mm.cpu.hl.w,
                      (unsigned long long)mm.tstates, (unsigned long long)mm.instructions);
        return std::string(buf);
    };

    for (int i = 0; i < 500'000 && !m.cpu.halted; ++i) m.step();
    const std::string after_first = fingerprint(m);

    // Restore into the same machine and repeat the run.
    check(m.load_state(snap), "the state loads back");
    for (int i = 0; i < 500'000 && !m.cpu.halted; ++i) m.step();
    const std::string after_second = fingerprint(m);
    check(after_first == after_second,
          "and the machine carries on to exactly the same place it did before");

    // Restoring into a different machine must work too -- that is the case
    // that catches a pointer saved into the snapshot.
    ein::Machine other;
    other.set_rom(rom);
    other.reset();
    if (have_disk) other.fdc.disk(0).load(disk);
    check(other.load_state(snap), "a state loads into a different machine object");
    for (int i = 0; i < 500'000 && !other.cpu.halted; ++i) other.step();
    check(fingerprint(other) == after_first, "and runs on identically there as well");

    // The keyboard is reached through a pointer inside the PSG; if the restore
    // left that dangling this is where it shows.
    other.keyboard.set_key(6, 6, true);
    other.io_write(0x02, 0x0e);
    other.io_write(0x03, 0x00);
    other.io_write(0x02, 0x0f);
    check(static_cast<std::uint8_t>(~other.io_read(0x02)) == 0x40,
          "the PSG's keyboard pointer points at the restored machine's own keyboard");

    std::vector<std::uint8_t> junk = {1, 2, 3, 4};
    check(!other.load_state(junk), "and a state that is not one is rejected");
}

void test_screenshot(const ein::RomImage& rom) {
    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    // A fixed number of instructions, so the result is deterministic.
    for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();

    std::vector<std::uint32_t> fb(ein::Tms9129::kWidth * ein::Tms9129::kHeight);
    m.vdp.render(fb.data());
    const std::string hash =
        ein::md5_hex(reinterpret_cast<const char*>(fb.data()), fb.size() * sizeof(std::uint32_t));

    const std::vector<std::string> screen = read_screen(m, rom);
    std::printf("           boot screen md5 %s\n", hash.c_str());
    print_screen(screen);

    // The text is what the hash is protecting; both are asserted so a failure
    // says which way it moved.
    check(screen_has(screen, "TATUNG/Xtal MOS 1.2"), "the banner is on screen");
    check(screen_has(screen, "Insert disc in drive 0"), "with the boot prompt");
    // The recorded hash. If this moves, something changed what MOS draws --
    // which is exactly what the gate is for. The two text assertions above
    // narrow down which way it went before you go looking.
    check(hash == "ef00ee9d60d66672cdf65acf741d4397",
          "the boot screen is pixel-for-pixel what it was when this was recorded");
}

// M9: accuracy.

// Sprite coincidence and the fifth-sprite flag, driven by putting sprites in
// VRAM and letting the raster cross them.
void test_sprites() {
    ein::Machine m;
    m.rom_enabled = false;

    auto vdp_reg = [&](int r, std::uint8_t v) {
        m.io_write(0x09, v);
        m.io_write(0x09, static_cast<std::uint8_t>(0x80 | r));
    };
    auto vram_write = [&](std::uint16_t a, std::uint8_t v) {
        m.io_write(0x09, static_cast<std::uint8_t>(a & 0xff));
        m.io_write(0x09, static_cast<std::uint8_t>(0x40 | (a >> 8)));
        m.io_write(0x08, v);
    };
    auto put_sprite = [&](int s, int y, int x, int name, int colour) {
        const std::uint16_t a = static_cast<std::uint16_t>(0x3b00 + s * 4);
        vram_write(a, static_cast<std::uint8_t>(y));
        vram_write(static_cast<std::uint16_t>(a + 1), static_cast<std::uint8_t>(x));
        vram_write(static_cast<std::uint16_t>(a + 2), static_cast<std::uint8_t>(name));
        vram_write(static_cast<std::uint16_t>(a + 3), static_cast<std::uint8_t>(colour));
    };
    auto run_a_frame = [&] {
        for (int i = 0; i < ein::Tms9129::kTStatesPerFrame + 4096; i += 64) m.tick(64);
    };

    // Graphics I, display on, sprites 8x8, attributes at 3B00, patterns 1800.
    vdp_reg(0, 0x00);
    vdp_reg(1, 0xc0);
    vdp_reg(5, 0x76);  // 3B00
    vdp_reg(6, 0x03);  // 1800
    // Pattern 0: a solid 8x8 block.
    for (int i = 0; i < 8; ++i) vram_write(static_cast<std::uint16_t>(0x1800 + i), 0xff);

    // One sprite on its own: no collision, no fifth.
    put_sprite(0, 50, 100, 0, 0x0f);
    for (int s = 1; s < 32; ++s) put_sprite(s, 0xd0, 0, 0, 0);
    m.vdp.read_status();
    run_a_frame();
    std::uint8_t st = m.vdp.read_status();
    std::printf("           one sprite: status %02X\n", st);
    check((st & ein::Tms9129::kStatusCollision) == 0, "a lone sprite collides with nothing");
    check((st & ein::Tms9129::kStatusFifthSprite) == 0, "and is not a fifth sprite");

    // Two overlapping: coincidence.
    put_sprite(1, 50, 104, 0, 0x0e);
    m.vdp.read_status();
    run_a_frame();
    st = m.vdp.read_status();
    std::printf("           two overlapping: status %02X\n", st);
    check((st & ein::Tms9129::kStatusCollision) != 0, "two overlapping sprites set coincidence");

    // Moved apart: no coincidence.
    put_sprite(1, 50, 200, 0, 0x0e);
    m.vdp.read_status();
    run_a_frame();
    st = m.vdp.read_status();
    check((st & ein::Tms9129::kStatusCollision) == 0, "and separating them clears it again");

    // A transparent sprite still collides -- colour has nothing to do with it.
    put_sprite(1, 50, 104, 0, 0x00);
    m.vdp.read_status();
    run_a_frame();
    st = m.vdp.read_status();
    check((st & ein::Tms9129::kStatusCollision) != 0,
          "a transparent sprite still sets coincidence, colour being irrelevant to it");

    // Five on one line: the fifth-sprite flag, carrying its number.
    for (int s = 0; s < 6; ++s) put_sprite(s, 80, 10 + s * 20, 0, 0x0f);
    for (int s = 6; s < 32; ++s) put_sprite(s, 0xd0, 0, 0, 0);
    m.vdp.read_status();
    run_a_frame();
    st = m.vdp.read_status();
    std::printf("           five on a line: status %02X (sprite %d)\n", st,
                st & ein::Tms9129::kStatusSpriteMask);
    check((st & ein::Tms9129::kStatusFifthSprite) != 0, "a fifth sprite on a line sets 5S");
    check((st & ein::Tms9129::kStatusSpriteMask) == 4,
          "and the low bits name it -- sprite 4, the fifth");

    // Only four are drawn. The fifth is dropped, which is the flicker.
    std::vector<std::uint32_t> fb(ein::Tms9129::kWidth * ein::Tms9129::kHeight);
    m.vdp.render(fb.data());
    const std::uint32_t bg = ein::Tms9129::palette(m.vdp.reg(7) & 0x0f);
    int drawn = 0;
    for (int s = 0; s < 6; ++s) {
        const int x = 10 + s * 20 + 2;
        if (fb[(81) * ein::Tms9129::kWidth + x] != bg) ++drawn;
    }
    std::printf("           %d of the six sprites were drawn on that line\n", drawn);
    check(drawn == 4, "only four sprites are displayed on a line");

    // Reading the status clears all three flags.
    check((m.vdp.read_status() & 0xe0) == 0, "reading the status clears F, 5S and C");
}

// MOS 1.21 is a different ROM, and nothing in the archive disassembles it. The
// only way to know it works is to run it.
void test_mos121(const std::filesystem::path& roms) {
    const auto path = roms / "einstein1.21.bin";
    if (!std::filesystem::exists(path)) {
        std::printf("[ SKIP ]  einstein1.21.bin not present\n");
        return;
    }
    const ein::RomImage rom = ein::load_rom(path);
    check(rom.identity == "MOS 1.21", "einstein1.21.bin is MOS 1.21");

    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();

    // The screen reader uses the 1.2 font; 1.21's is at the same place if the
    // two ROMs share their character set, which the banner will show.
    const std::vector<std::string> screen = read_screen(m, rom);
    std::printf("           MOS 1.21: pc=%04X im=%d iff1=%d, %llu VRAM writes\n", m.cpu.pc.w,
                m.cpu.im, m.cpu.iff1,
                static_cast<unsigned long long>(m.vdp.vram_writes()));
    print_screen(screen);

    check(m.cpu.im == 2 && m.cpu.iff1, "it completes its reset and enables IM 2 interrupts");
    check(m.vdp.vram_writes() > 1000, "and drives the VDP");
    check(screen_has(screen, "MOS 1.21") || screen_has(screen, "Xtal MOS"),
          "and puts its own banner on screen");

    // The clock has to tick on 1.21 as well.
    auto rtc = [&] {
        std::string s;
        for (int i = 0; i < 6; ++i) s.push_back(static_cast<char>(m.read(std::uint16_t(0xfb8c + i))));
        return s;
    };
    const std::string before = rtc();
    const std::uint64_t target = m.tstates + 3ull * ein::Machine::kCpuClockHz;
    while (m.tstates < target && !m.cpu.halted) m.step();
    std::printf("           MOS 1.21 RTC %s -> %s\n", before.c_str(), rtc().c_str());
    check(rtc() != before, "and its real-time clock runs");
}

// Boot MOS 1.21 from a disc, which is the part that would break if the two
// ROMs disagreed about the disc parameters.
void test_mos121_disk(const std::filesystem::path& roms) {
    const auto path = roms / "einstein1.21.bin";
    const auto sys = roms / "disassembled" / "Einstein" / "__XTALDOS202" / "SYSTEM202.CPM";
    if (!std::filesystem::exists(path) || !std::filesystem::exists(sys)) return;

    const ein::RomImage rom = ein::load_rom(path);
    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    m.fdc.disk(0).load(sys);

    bool entered = false;
    const std::uint64_t limit = 30ull * ein::Machine::kCpuClockHz;
    while (m.tstates < limit && !m.cpu.halted && !entered) {
        m.step();
        if (m.cpu.pc.w == 0xf800) entered = true;
    }
    for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();
    const std::vector<std::string> screen = read_screen(m, rom);
    std::printf("           MOS 1.21 boot: entered=%d, %llu sectors read, %llu RNF\n", entered,
                static_cast<unsigned long long>(m.fdc.sectors_read()),
                static_cast<unsigned long long>(m.fdc.not_found()));
    print_screen(screen);
    check(entered, "MOS 1.21 boots XtalDOS 2.02 from disc as well");
    check(m.fdc.not_found() == 0, "with no sector missing");
}

// The 80-column card. It was an add-on, so a base machine must report that it
// has none -- and then, with one fitted, MOS must find it and drive it.
void test_card80(const ein::RomImage& rom) {
    // Absent: the detection at L0E31 writes the complement of what it read to
    // port 40 and expects it back. An empty bus reads FF and cannot.
    {
        ein::Machine m;
        m.set_rom(rom);
        m.reset();
        for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();
        check((m.read(0xfb45) & 0x04) == 0,
              "with no card fitted MOS clears the 80-column bit in PCFLAGS");
        check(m.card80.reg_writes() == 0, "and never touches the 6845");
    }

    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    m.card80.present = true;
    for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();

    std::printf("           80-column: PCFLAGS=%02X, %llu 6845 register writes, "
                "%llu characters, start %04X\n",
                m.read(0xfb45), static_cast<unsigned long long>(m.card80.reg_writes()),
                static_cast<unsigned long long>(m.card80.ram_writes()),
                m.card80.start_address());
    for (int row = 0; row < 24; ++row) {
        const std::string l = m.card80.line(row);
        if (!l.empty()) std::printf("           80| %s\n", l.c_str());
    }

    check(m.card80.reg_writes() >= 16,
          "with a card fitted MOS programmes all sixteen 6845 registers");
    check((m.read(0xfb45) & 0x04) != 0, "and sets the 80-column bit in PCFLAGS");
    check(m.card80.ram_writes() > 0, "and writes characters to the card's RAM");

    bool any_text = false;
    for (int row = 0; row < 24 && !any_text; ++row) {
        if (m.card80.line(row).find("MOS") != std::string::npos ||
            m.card80.line(row).find("Xtal") != std::string::npos ||
            m.card80.line(row).find("EINSTEIN") != std::string::npos) {
            any_text = true;
        }
    }
    if (!any_text) {
        for (int base = 0; base < 512; base += 64) {
            std::string t;
            for (int i = 0; i < 64; ++i) {
                const std::uint8_t c = m.card80.ram(base + i);
                t.push_back((c >= 0x20 && c < 0x7f) ? static_cast<char>(c) : '.');
            }
            std::printf("           RAW %04X %s\n", base, t.c_str());
        }
    }
    check(any_text, "and its banner is readable out of that RAM");

    // The RAM addressing: the offset rides on A8-A15, the page on the low port.
    m.io_write(static_cast<std::uint16_t>((0x34 << 8) | 0x42), 0x5a);
    check(m.card80.ram(0x234) == 0x5a,
          "port 42 with B=34 addresses offset 234 -- page in the port, offset on A8-A15");
    check(m.io_read(static_cast<std::uint16_t>((0x34 << 8) | 0x42)) == 0x5a, "and reads back");
}

// Access timing, which is what "contended timing" amounts to on this machine.
// There is no shared-memory contention of the Spectrum kind here -- the VDP has
// its own private VRAM behind a port -- but the TMS9918 family does need time
// between VRAM accesses while the display is fetching, and MOS visibly paces
// itself to it: the character-set upload at Mos12.zsm:2370 pads each OUTI with
// PUSH AF / POP AF for no other reason.
void test_vdp_timing(const ein::RomImage& rom) {
    ein::Machine m;
    m.set_rom(rom);
    m.reset();
    for (int i = 0; i < 4'000'000 && !m.cpu.halted; ++i) m.step();

    const int gap = m.vdp.min_access_gap();
    std::printf("           VDP access: MOS's tightest VRAM spacing is %d T-states (%.2f us); "
                "the datasheet asks for %d (8.00 us)\n",
                gap, gap * 0.25, ein::Tms9129::kMinAccessTStates);
    std::printf("           %llu of its accesses are inside that figure\n",
                static_cast<unsigned long long>(m.vdp.fast_accesses()));

    // MOS is not free-running: OUTI on its own would space writes 16 T-states
    // apart, and it is nowhere near that.
    check(gap > 16, "MOS deliberately paces its VRAM writes rather than running flat out");
    // And it sits right on the published limit -- one T-state inside it. That
    // is the answer to whether to enforce the constraint by dropping accesses:
    // doing so would break the machine's own firmware, so it is measured and
    // reported instead. If a title ever misbehaves, fast_accesses() says
    // whether this is why.
    check(gap >= ein::Tms9129::kMinAccessTStates - 4 &&
              gap <= ein::Tms9129::kMinAccessTStates + 8,
          "and lands within a T-state or so of the datasheet's own 8 us");
}

}  // namespace

int main(int argc, char** argv) {
    const std::filesystem::path roms = (argc > 1) ? argv[1] : "roms";
    if (argc > 2) g_disks = argv[2];
    std::printf("core acceptance -- roms dir: %s\n\n", roms.string().c_str());

    test_md5_vectors();
    std::printf("\n");
    test_cpu();
    print_gap();
    test_sound();
    print_gap();
    test_envelope();
    print_gap();
    test_adc();
    print_gap();
    test_daisy_chain();
    print_gap();
    test_halt_wakeup();
    print_gap();
    test_disasm();
    print_gap();
    test_sprites();
    std::printf("\n");
    test_rom(roms);

    std::printf("\n%s (%d failure%s)\n", g_failures ? "FAILED" : "ALL PASSED",
                g_failures, g_failures == 1 ? "" : "s");
    return g_failures == 0 ? 0 : 1;
}
