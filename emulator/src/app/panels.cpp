// Albert -- the debugger panels.
//
// Split out of main.cpp at M10 phase 4.
#include "app.h"

namespace albert {

// ------------------------------------------------------------ Default layout

// Memory gets the right two thirds; the left column stacks ROM, Registers and
// Control top to bottom.
void build_default_layout(ImGuiID dockspace_id) {
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

    ImGuiID left = 0, right = 0;
    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.38f, &left, &right);

    ImGuiID left_top = 0, left_rest = 0;
    ImGui::DockBuilderSplitNode(left, ImGuiDir_Up, 0.34f, &left_top, &left_rest);

    ImGuiID left_mid = 0, left_bottom = 0;
    ImGui::DockBuilderSplitNode(left_rest, ImGuiDir_Up, 0.5f, &left_mid, &left_bottom);

    ImGuiID right_top = 0, right_bottom = 0;
    ImGui::DockBuilderSplitNode(right, ImGuiDir_Up, 0.62f, &right_top, &right_bottom);

    ImGui::DockBuilderDockWindow("ROM", left_top);
    ImGui::DockBuilderDockWindow("Registers", left_mid);
    ImGui::DockBuilderDockWindow("Control", left_bottom);
    ImGui::DockBuilderDockWindow("VDP", left_bottom);
    ImGui::DockBuilderDockWindow("Keyboard / CTC", left_bottom);
    ImGui::DockBuilderDockWindow("Discs", left_bottom);
    ImGui::DockBuilderDockWindow("Sound", left_bottom);
    ImGui::DockBuilderDockWindow("Peripherals", left_bottom);
    ImGui::DockBuilderDockWindow("Screen", right_top);
    ImGui::DockBuilderDockWindow("Memory", right_bottom);
    ImGui::DockBuilderDockWindow("Disassembly", right_bottom);
    ImGui::DockBuilderDockWindow("VRAM", right_bottom);
    ImGui::DockBuilderFinish(dockspace_id);
}


// ---------------------------------------------------------------- ROM panel

void draw_rom_panel(AppState& app) {
    if (!app.show_rom) return;
    if (!ImGui::Begin("ROM", &app.show_rom)) { ImGui::End(); return; }

    ImGui::TextWrapped("%s", app.rom_path.c_str());
    ImGui::Separator();

    if (!app.load_error.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "load failed");
        ImGui::TextWrapped("%s", app.load_error.c_str());
        ImGui::End();
        return;
    }
    if (!app.machine.has_rom()) {
        ImGui::TextDisabled("no ROM loaded");
        ImGui::End();
        return;
    }

    const ein::RomImage& rom = app.machine.rom();
    ImGui::Text("identity   %s", rom.identity.c_str());
    ImGui::Text("file size  %zu bytes%s", rom.file_size,
                rom.truncated ? "  (truncated to 8192)" : "");
    ImGui::Text("md5        %s", rom.md5.c_str());

    ImGui::Spacing();
    if (rom.md5 == ein::kMos12Md5) {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "verified: MOS 1.2");
    } else if (rom.recognised) {
        ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f), "recognised, not MOS 1.2");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "unrecognised image");
    }
    ImGui::TextDisabled("expected %s", ein::kMos12Md5);

    ImGui::End();
}


// ----------------------------------------------------------- Register panel

void draw_registers(AppState& app) {
    if (!app.show_registers) return;
    if (!ImGui::Begin("Registers", &app.show_registers)) { ImGui::End(); return; }

    const ein::Z80& c = app.machine.cpu;

    if (ImGui::BeginTable("regs", 4, ImGuiTableFlags_SizingFixedFit)) {
        auto cell = [](const char* n, unsigned v) {
            ImGui::TableNextColumn();
            ImGui::TextDisabled("%s", n);
            ImGui::TableNextColumn();
            ImGui::Text("%04X", v);
        };
        ImGui::TableNextRow(); cell("AF", c.af.w); cell("AF'", c.af2.w);
        ImGui::TableNextRow(); cell("BC", c.bc.w); cell("BC'", c.bc2.w);
        ImGui::TableNextRow(); cell("DE", c.de.w); cell("DE'", c.de2.w);
        ImGui::TableNextRow(); cell("HL", c.hl.w); cell("HL'", c.hl2.w);
        ImGui::TableNextRow(); cell("IX", c.ix.w); cell("IY", c.iy.w);
        ImGui::TableNextRow(); cell("SP", c.sp.w); cell("PC", c.pc.w);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::TextDisabled("I");
        ImGui::TableNextColumn(); ImGui::Text("%02X", c.i);
        ImGui::TableNextColumn(); ImGui::TextDisabled("R");
        ImGui::TableNextColumn(); ImGui::Text("%02X", c.r);
        ImGui::TableNextRow(); cell("WZ", c.wz.w);
        ImGui::EndTable();
    }

    ImGui::Separator();

    // SZ5H3PNC, uppercase = set.
    static const char kNames[8] = {'S', 'Z', '5', 'H', '3', 'P', 'N', 'C'};
    char flags[9];
    for (int i = 0; i < 8; ++i) {
        const bool set = (c.af.b.l >> (7 - i)) & 1;
        flags[i] = set ? kNames[i] : static_cast<char>(kNames[i] | 0x20);
    }
    flags[8] = '\0';
    ImGui::Text("flags  %s", flags);
    ImGui::Text("IFF1 %d   IFF2 %d   IM %u  %s", c.iff1, c.iff2, c.im,
                c.halted ? "HALTED" : "");

    ImGui::End();
}


// ------------------------------------------------------------ Control panel

// Snapshots. The path is fixed until the file dialogs arrive; both the Control
// panel buttons and the File menu come through here so there is one behaviour.
void write_state(AppState& app, const std::filesystem::path& path) {
    const std::vector<std::uint8_t> st = app.machine.save_state();
    std::ofstream f(path, std::ios::binary);
    const std::string name = path.filename().string();
    if (f) {
        f.write(reinterpret_cast<const char*>(st.data()),
                static_cast<std::streamsize>(st.size()));
        app.state_note = "saved " + name;
    } else {
        app.state_note = "could not write " + name;
    }
}

void read_state(AppState& app, const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    const std::string name = path.filename().string();
    if (f) {
        const std::vector<std::uint8_t> st((std::istreambuf_iterator<char>(f)),
                                           std::istreambuf_iterator<char>());
        app.state_note =
            app.machine.load_state(st) ? "restored " + name : name + " is not a save state";
    } else {
        app.state_note = "no " + name + " to load";
    }
}

void save_state_file(AppState& app) { write_state(app, kStateFile); }
void load_state_file(AppState& app) { read_state(app, kStateFile); }

void draw_controls(AppState& app) {
    if (!app.show_control) return;
    if (!ImGui::Begin("Control", &app.show_control)) { ImGui::End(); return; }

    if (!ein::Machine::kCpuImplemented) {
        ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f), "%s",
                           ein::Machine::kCpuStatusNote);
        ImGui::TextDisabled("Transport works; nothing executes until the core lands.");
        ImGui::Separator();
    }

    if (app.running) {
        if (ImGui::Button("Pause")) app.running = false;
    } else {
        if (ImGui::Button("Run")) app.running = true;
    }
    ImGui::SameLine();
    // Machine::step() accounts for T-states and the instruction count itself
    // (every bus access ticks the machine), so nothing is added here.
    if (ImGui::Button("Step")) app.machine.step();
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        app.machine.reset();
        app.running = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Save state")) save_state_file(app);
    ImGui::SameLine();
    if (ImGui::Button("Load state")) load_state_file(app);
    if (!app.state_note.empty()) ImGui::TextDisabled("%s", app.state_note.c_str());

    ImGui::Separator();
    ImGui::Text("T-states      %llu", static_cast<unsigned long long>(app.machine.tstates));
    ImGui::Text("instructions  %llu", static_cast<unsigned long long>(app.machine.instructions));
    ImGui::Text("clock         %d Hz", ein::Machine::kCpuClockHz);
    if (app.machine.rom_enabled) {
        ImGui::TextColored(ImVec4(0.62f, 0.84f, 1.0f, 1.0f), "ROM overlay   in  (0000-1FFF)");
    } else {
        ImGui::Text("ROM overlay   out (RAM at 0000)");
    }
    ImGui::Text("port 24 hits  %llu", static_cast<unsigned long long>(app.machine.rom_toggles));
    if (app.machine.saw_ram_rom_write) {
        ImGui::TextDisabled("last data     %02X (ignored -- strobe only)",
                            app.machine.last_ram_rom_write);
    }

    ImGui::End();
}


// ------------------------------------------------------------- Memory panel

void draw_memory(AppState& app) {
    if (!app.show_memory) return;
    if (!ImGui::Begin("Memory", &app.show_memory)) { ImGui::End(); return; }

    ImGui::SetNextItemWidth(90.0f);
    ImGui::InputInt("##goto", &app.goto_addr, 0, 0, ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::SameLine();
    if (ImGui::Button("Go")) app.scroll_to_addr = true;
    ImGui::SameLine();
    if (ImGui::Button("0000")) { app.goto_addr = 0x0000; app.scroll_to_addr = true; }
    ImGui::SameLine();
    if (ImGui::Button("FB00")) { app.goto_addr = 0xfb00; app.scroll_to_addr = true; }
    ImGui::SameLine();
    ImGui::TextDisabled("scratchpad");

    ImGui::Separator();

    const float line_h = ImGui::GetTextLineHeightWithSpacing();
    if (ImGui::BeginChild("hex", ImVec2(0, 0), ImGuiChildFlags_None,
                          ImGuiWindowFlags_HorizontalScrollbar)) {
        // Scroll before the clipper runs -- adjusting scroll mid-clip breaks it.
        if (app.scroll_to_addr) {
            ImGui::SetScrollY(static_cast<float>((app.goto_addr & 0xffff) / 16) * line_h);
            app.scroll_to_addr = false;
        }

        constexpr int kRows = 0x10000 / 16;
        ImGuiListClipper clipper;
        clipper.Begin(kRows, line_h);
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                const int base = row * 16;
                const bool in_rom =
                    app.machine.rom_enabled && base < ein::Machine::kRomWindowEnd;

                char line[160];
                int n = std::snprintf(line, sizeof line, "%04X  ", base);
                char ascii[17];
                for (int i = 0; i < 16; ++i) {
                    const std::uint8_t v =
                        app.machine.read(static_cast<std::uint16_t>(base + i));
                    n += std::snprintf(line + n, sizeof line - n, "%02X ", v);
                    if (i == 7) n += std::snprintf(line + n, sizeof line - n, " ");
                    ascii[i] = (v >= 0x20 && v < 0x7f) ? static_cast<char>(v) : '.';
                }
                ascii[16] = '\0';

                if (in_rom) {
                    ImGui::TextColored(ImVec4(0.62f, 0.84f, 1.0f, 1.0f), "%s |%s|", line, ascii);
                } else {
                    ImGui::Text("%s |%s|", line, ascii);
                }
            }
        }
        clipper.End();
    }
    ImGui::EndChild();

    ImGui::End();
}


// ---------------------------------------------------------------- VDP panel

void draw_vdp(AppState& app) {
    if (!app.show_vdp) return;
    if (!ImGui::Begin("VDP", &app.show_vdp)) { ImGui::End(); return; }

    const auto& v = app.machine.vdp;
    static const char* kModeName[] = {"Graphics I", "Graphics II", "Multicolour", "Text", "?"};
    ImGui::Text("mode      %s", kModeName[v.mode()]);
    ImGui::Text("display   %s", v.display_enabled() ? "on" : "off");
    ImGui::Text("regs      ");
    for (int i = 0; i < 8; ++i) {
        ImGui::SameLine();
        ImGui::Text("%02X", v.reg(i));
    }
    ImGui::Text("name      %04X", v.name_base());
    ImGui::Text("sprite025 %04X", v.sprite_pat_base());
    ImGui::Text("colour    %s", (v.reg(3) & 0x80) ? "2000" : "0000");
    ImGui::Text("pattern   %s", (v.reg(4) & 0x04) ? "2000" : "0000");
    ImGui::Text("frames    %llu", static_cast<unsigned long long>(v.frames()));
    ImGui::Text("writes    reg %llu  vram %llu",
                static_cast<unsigned long long>(v.reg_writes()),
                static_cast<unsigned long long>(v.vram_writes()));
    ImGui::Text("INT line  %s", v.irq() ? "asserted" : "clear");

    ImGui::End();
}


// ---------------------------------------------------------------- Disc panel

void insert_disc(AppState& app, int drive, const std::filesystem::path& path) {
    try {
        app.machine.fdc.disk(drive).load(path);
        app.disk_error.clear();
    } catch (const std::exception& e) {
        app.disk_error = e.what();
    }
}

void draw_disks(AppState& app) {
    if (!app.show_disks) return;
    if (!ImGui::Begin("Discs", &app.show_disks)) { ImGui::End(); return; }

    const ein::Fd1770& f = app.machine.fdc;
    ImGui::Text("FD1770   status %02X  %s", f.status(), f.phase_name());
    ImGui::Text("  track %3u  sector %3u  data %02X  last cmd %02X", f.track_reg(),
                f.sector_reg(), f.data_reg(), f.last_command());
    ImGui::Text("  drive %d side %d  head at track %d  motor %s", f.selected_drive(), f.side(),
                f.head_track(), f.motor_on() ? "on" : "off");
    ImGui::Text("  read %llu  written %llu  not found %llu",
                static_cast<unsigned long long>(f.sectors_read()),
                static_cast<unsigned long long>(f.sectors_written()),
                static_cast<unsigned long long>(f.not_found()));

    ImGui::Separator();
    for (int d = 0; d < 2; ++d) {
        ImGui::PushID(d);
        const ein::DiskImage& disk = app.machine.fdc.disk(d);
        if (disk.loaded()) {
            ImGui::Text("drive %d  %s", d, disk.name().c_str());
            ImGui::TextDisabled("        %s, %d tracks, %d side(s)", disk.format_name().c_str(),
                                disk.tracks(), disk.sides());
            bool wp = disk.write_protected;
            if (ImGui::Checkbox("write protect", &wp)) {
                app.machine.fdc.disk(d).write_protected = wp;
            }
            ImGui::SameLine();
            if (ImGui::Button("Eject")) app.machine.fdc.disk(d).eject();
        } else {
            ImGui::Text("drive %d  (empty)", d);
        }
        ImGui::PopID();
    }

    ImGui::Separator();
    ImGui::TextDisabled("Insert a .dsk, a plain sector image, or a SYSTEM*.CPM:");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##path", app.disk_path, sizeof app.disk_path);
    if (ImGui::Button("Insert in drive 0")) insert_disc(app, 0, app.disk_path);
    ImGui::SameLine();
    if (ImGui::Button("Insert in drive 1")) insert_disc(app, 1, app.disk_path);
    ImGui::SameLine();
    if (ImGui::Button("Blank disc")) {
        app.machine.fdc.disk(0).format(1, "blank");
        app.disk_error.clear();
    }
    if (!app.disk_error.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", app.disk_error.c_str());
    }
    ImGui::TextDisabled("Reset the machine after inserting to boot from it.");

    ImGui::End();
}


void draw_sound(AppState& app) {
    if (!app.show_sound) return;
    if (!ImGui::Begin("Sound", &app.show_sound)) { ImGui::End(); return; }

    const ein::Ay38910& p = app.machine.psg;
    if (!app.audio_ok) {
        ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f), "no audio device");
        ImGui::TextDisabled("Emulation is paced on a fixed frame budget instead.");
    } else {
        ImGui::Checkbox("sound", &app.sound_on);
        ImGui::Text("queue     %d ms  (target %d)", app.queued_ms, kAudioTargetMs);
    }
    ImGui::Text("clock     %d Hz", ein::Ay38910::kClockHz);
    // The bottom of the slider is off rather than a 0 Hz cutoff, so one drag
    // covers both choosing a cutoff and turning it off. Options > Sound filter
    // offers the same control as a short list of round numbers.
    float lp = app.machine.psg.lowpass_hz();
    if (ImGui::SliderFloat("low-pass", &lp, 0.0f, 20000.0f,
                           lp < 250.0f ? "off" : "%.0f Hz", ImGuiSliderFlags_AlwaysClamp)) {
        app.machine.psg.set_lowpass(lp < 250.0f ? 0.0f : lp);
    }
    ImGui::Text("generated %llu samples, %llu dropped",
                static_cast<unsigned long long>(p.samples_generated()),
                static_cast<unsigned long long>(p.samples_dropped()));

    ImGui::Separator();
    ImGui::Text("mixer R7  %02X", p.reg(7));
    for (int ch = 0; ch < 3; ++ch) {
        const bool tone = (p.reg(7) & (1u << ch)) == 0;
        const bool noise = (p.reg(7) & (1u << (ch + 3))) == 0;
        const double hz = p.tone_period(ch) > 0
                              ? double(ein::Ay38910::kClockHz) / (16.0 * p.tone_period(ch))
                              : 0.0;
        ImGui::Text("  %c  period %4d (%7.1f Hz)  level %2d %s%s%s", 'A' + ch,
                    p.tone_period(ch), hz, p.channel_level(ch), tone ? "tone " : "",
                    noise ? "noise " : "", (p.reg(8 + ch) & 0x10) ? "env" : "");
    }
    ImGui::Text("noise     period %d", p.noise_period());
    ImGui::Text("envelope  period %d  shape %02X  level %d", p.envelope_period(), p.reg(13),
                p.envelope_level());

    ImGui::End();
}


// ------------------------------------------------------- Keyboard/CTC panel

void draw_keyboard(AppState& app) {
    if (!app.show_keyboard) return;
    if (!ImGui::Begin("Keyboard / CTC", &app.show_keyboard)) { ImGui::End(); return; }

    const auto& kbd = app.machine.keyboard;
    ImGui::TextDisabled("Shift, Ctrl and Alt (GRAPH) are modifiers on port 20.");
    ImGui::TextDisabled("Caps Lock is the ALPHA key; Pause is BREAK.");
    ImGui::Separator();

    ImGui::Text("matrix");
    for (int r = 0; r < 8; ++r) {
        const std::uint8_t cols = kbd.columns(static_cast<std::uint8_t>(1u << r));
        std::string bits;
        for (int c = 7; c >= 0; --c) bits.push_back((cols & (1u << c)) ? '1' : '.');
        ImGui::Text("  row %02X  %s", 1u << r, bits.c_str());
    }
    ImGui::Text("port 20   %02X   PSG row select %02X", kbd.modifier_port(),
                app.machine.psg.row_select());
    ImGui::Text("LKEY      %02X   KFLAG %02X", app.machine.read(0xfb46),
                app.machine.read(0xfb3e));

    ImGui::Separator();
    ImGui::Text("CTC (ports 28-2B), vector base %02X", app.machine.ctc.vector());
    for (int c = 0; c < 4; ++c) {
        const std::uint8_t ctrl = app.machine.ctc.control(c);
        ImGui::Text("  ch%d ctrl %02X tc %3u count %3u  %-7s /%-3d %s  zeros %llu", c, ctrl,
                    app.machine.ctc.time_constant(c), app.machine.ctc.counter(c),
                    (ctrl & 0x40) ? "counter" : "timer", (ctrl & 0x20) ? 256 : 16,
                    (ctrl & 0x80) ? "INT" : "   ",
                    static_cast<unsigned long long>(app.machine.ctc.zero_counts(c)));
    }
    ImGui::Text("interrupts taken %llu",
                static_cast<unsigned long long>(app.machine.interrupts_taken));

    // The BCD real-time clock the CTC3 handler maintains at FB8C.
    char rtc[7] = {};
    for (int i = 0; i < 6; ++i) {
        rtc[i] = static_cast<char>(app.machine.read(static_cast<std::uint16_t>(0xfb8c + i)));
    }
    ImGui::Text("RTC (FB8C)       %c%c:%c%c:%c%c", rtc[0], rtc[1], rtc[2], rtc[3], rtc[4], rtc[5]);

    ImGui::End();
}


// ---------------------------------------------------------- Peripherals

void draw_peripherals(AppState& app) {
    if (!app.show_peripherals) return;
    if (!ImGui::Begin("Peripherals", &app.show_peripherals)) { ImGui::End(); return; }

    const ein::Z80Pio& pio = app.machine.pio;
    static const char* kModes[] = {"output", "input", "bidirectional", "bit control"};
    ImGui::Text("Z80 PIO (30-37)");
    for (int port = 0; port < 2; ++port) {
        ImGui::Text("  port %c  vector %02X  %-13s %s  data %02X  strobes %llu", 'A' + port,
                    pio.vector(port), kModes[pio.mode(port) & 3],
                    pio.int_enabled(port) ? "INT" : "   ", pio.output(port),
                    static_cast<unsigned long long>(pio.strobes(port)));
    }

    // Port B is where the digital joystick arrives (port 32), active low.
    ImGui::Text("  port B input  %02X   %s%s%s%s%s", app.machine.pio.input(1),
                (app.machine.pio.input(1) & kPioJoyUp) ? "" : "up ",
                (app.machine.pio.input(1) & kPioJoyDown) ? "" : "down ",
                (app.machine.pio.input(1) & kPioJoyLeft) ? "" : "left ",
                (app.machine.pio.input(1) & kPioJoyRight) ? "" : "right ",
                (app.machine.pio.input(1) & kPioJoyFire) ? "" : "fire");

    ImGui::Separator();
    ImGui::Text("Printer (PIO port A)  %zu bytes", app.machine.printer.size());
    {
        // Show the tail as text; control codes as dots.
        std::string tail;
        const std::size_t n = app.machine.printer.size();
        const std::size_t from = n > 240 ? n - 240 : 0;
        for (std::size_t i = from; i < n; ++i) {
            const std::uint8_t c = app.machine.printer[i];
            tail.push_back((c >= 0x20 && c < 0x7f) ? static_cast<char>(c)
                                                   : (c == 0x0a ? '\n' : '.'));
        }
        ImGui::TextWrapped("%s", tail.c_str());
    }
    if (ImGui::Button("Clear printer")) app.machine.printer.clear();
    ImGui::SameLine();
    ImGui::TextDisabled("set PCFLAGS bit 1 (FB45) to echo the screen to it");

    ImGui::Separator();
    const ein::I8251& u = app.machine.uart;
    ImGui::Text("8251 serial (10-17)");
    ImGui::Text("  mode %02X  command %02X  status %02X", u.mode(), u.command(), u.status());
    ImGui::Text("  %d data bits, x%d clock", u.data_bits(), u.clock_divisor());
    // The bit rate is whatever the CTC is delivering, so show it rather than a
    // configured value.
    ImGui::Text("  tx clocks %llu  rx clocks %llu",
                static_cast<unsigned long long>(u.tx_clocks()),
                static_cast<unsigned long long>(u.rx_clocks()));
    bool loop = u.loopback;
    if (ImGui::Checkbox("loopback (TxD to RxD)", &loop)) app.machine.uart.loopback = loop;
    ImGui::SameLine();
    ImGui::TextDisabled("%zu byte(s) sent", u.sent().size());

    ImGui::Separator();
    ImGui::Text("ADC0844 (38-3F)   channel %d  result %02X %s",
                app.machine.adc.selected_channel(), app.machine.adc.result(),
                app.machine.adc.converting() ? "converting" : "");
    for (int ch = 0; ch < 2; ++ch) {
        ImGui::PushID(ch);
        int v = app.machine.adc.axis(ch);
        if (ImGui::SliderInt(ch == 0 ? "joystick X" : "joystick Y", &v, 0, 255)) {
            app.machine.adc.set_axis(ch, static_cast<std::uint8_t>(v));
        }
        ImGui::PopID();
    }
    ImGui::Text("masks: keyboard %02X  ADC %02X  fire %02X", app.machine.kbd_mask,
                app.machine.adc_mask, app.machine.fire_mask);
    ImGui::Text("ALPHA LED %s", app.machine.alpha_led ? "on" : "off");

    ImGui::Separator();
    bool card = app.machine.card80.present;
    if (ImGui::Checkbox("80-column card fitted", &card)) {
        app.machine.card80.present = card;
        app.machine.card80.reset();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("reset the machine for MOS to find it");
    if (app.machine.card80.present) {
        ImGui::Text("6845: %llu register writes, %llu characters, start %04X",
                    static_cast<unsigned long long>(app.machine.card80.reg_writes()),
                    static_cast<unsigned long long>(app.machine.card80.ram_writes()),
                    app.machine.card80.start_address());
        // The card carries its own character generator, which is not in this
        // archive, so its text is shown rather than drawn.
        if (ImGui::BeginChild("col80", ImVec2(0, 200), true,
                              ImGuiWindowFlags_HorizontalScrollbar)) {
            for (int row = 0; row < ein::Card80::kRows; ++row) {
                ImGui::TextUnformatted(app.machine.card80.line(row).c_str());
            }
        }
        ImGui::EndChild();
    }

    ImGui::End();
}


// ------------------------------------------------------------- Disassembly

// Loads Hohne's commented sources, if they are next to the ROM. Mos12.zsm maps
// onto the ROM directly; UPPERMEM.ZSM describes code MOS copies into RAM, whose
// bytes live in the ROM at the source of each copy (MEMORY.TXT).
void load_symbols(AppState& app) {
    app.symbols = ein::Symbols();
    app.symbols_note.clear();
    if (!app.machine.has_rom()) return;

    const auto dir = roms_dir() / "disassembled" / "Einstein" / "__MOS12";
    if (!std::filesystem::exists(dir / "Mos12.zsm")) {
        app.symbols_note = "sources not found next to the ROM";
        return;
    }
    const ein::RomImage& rom = app.machine.rom();
    auto rom_read = [&rom](std::uint16_t a) -> std::uint8_t {
        return a < rom.data.size() ? rom.data[a] : 0xff;
    };
    auto copied = [&rom](std::uint16_t a) -> std::uint8_t {
        if (a >= 0xfb00 && a <= 0xfb45) return rom.data[0x0ff0 + (a - 0xfb00)];
        if (a >= 0xfc09 && a <= 0xfc8c) return rom.data[0x1041 + (a - 0xfc09)];
        if (a <= 0x000a) return rom.data[0x1036 + a];
        return 0xff;
    };
    app.symbols.load_equates(dir / "Einstein.zsm");
    app.symbols.load_equates(dir / "MOSEQU.GEN");
    app.symbols.load_source(dir / "Mos12.zsm", rom_read);
    app.symbols.load_source(dir / "UPPERMEM.ZSM", copied);

    char note[160];
    std::snprintf(note, sizeof note, "%zu lines, %d/%d anchors agreed",
                  app.symbols.line_count(), app.symbols.anchors_matched(),
                  app.symbols.anchors());
    app.symbols_note = note;
}

void draw_disassembly(AppState& app) {
    if (!app.show_disasm) return;
    if (!ImGui::Begin("Disassembly", &app.show_disasm)) { ImGui::End(); return; }

    ein::Machine& m = app.machine;

    if (ImGui::Button("Follow PC")) app.disasm_follow = true;
    ImGui::SameLine();
    ImGui::Checkbox("source", &app.disasm_source);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(90);
    ImGui::InputInt("##goto", &app.disasm_addr, 0, 0, ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::SameLine();
    if (ImGui::Button("Go")) app.disasm_follow = false;
    if (!app.symbols_note.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", app.symbols_note.c_str());
    }

    if (m.bp_hit()) {
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "stopped: %s at %s%04X",
                           m.bp_hit_kind() == ein::Machine::kBpExec     ? "execute"
                           : m.bp_hit_kind() == ein::Machine::kBpWrite  ? "write"
                           : m.bp_hit_kind() == ein::Machine::kBpRead   ? "read"
                           : m.bp_hit_kind() == ein::Machine::kBpOut    ? "OUT"
                                                                        : "IN",
                           m.bp_hit_is_port() ? "port " : "", m.bp_hit_addr());
        ImGui::SameLine();
        if (ImGui::Button("Continue")) {
            m.resume_from_breakpoint();
            app.running = true;
        }
    }

    ImGui::Separator();

    if (app.disasm_follow) app.disasm_addr = m.cpu.pc.w;
    auto rd = [&m](std::uint16_t a) { return m.peek(a); };

    if (ImGui::BeginChild("code", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        auto addr = static_cast<std::uint16_t>(app.disasm_addr & 0xffff);
        for (int row = 0; row < 64; ++row) {
            const ein::Symbols::Line* src = app.symbols.line_at(addr);
            const std::string* label = app.symbols.label_at(addr);

            if (label) {
                ImGui::TextColored(ImVec4(0.55f, 0.85f, 1.0f, 1.0f), "%s:", label->c_str());
            }

            // Breakpoint toggle in the margin.
            ImGui::PushID(addr);
            const std::uint8_t bp = m.breakpoint(addr);
            const bool exec_bp = (bp & ein::Machine::kBpExec) != 0;
            if (ImGui::SmallButton(exec_bp ? "*" : " ")) {
                m.set_breakpoint(addr, exec_bp ? 0 : ein::Machine::kBpExec);
            }
            ImGui::PopID();
            ImGui::SameLine();

            const ein::Instruction ins = ein::disassemble(rd, addr);
            const bool at_pc = addr == m.cpu.pc.w;

            std::string bytes;
            for (int i = 0; i < ins.length && i < 4; ++i) {
                char b[4];
                std::snprintf(b, sizeof b, "%02X", m.peek(static_cast<std::uint16_t>(addr + i)));
                bytes += b;
            }
            while (bytes.size() < 8) bytes += ' ';

            // Where Hohne wrote something, show his line; the raw decode is
            // always there underneath as the fallback.
            std::string text = ins.text;
            std::string comment;
            if (app.disasm_source && src) {
                if (src->is_data) text = src->text;
                comment = src->comment;
            }
            // Name the target if a label is known for it.
            if (ins.target >= 0) {
                if (const std::string* t =
                        app.symbols.label_at(static_cast<std::uint16_t>(ins.target))) {
                    text += "   ; -> ";
                    text += *t;
                }
            }

            const ImVec4 col = at_pc ? ImVec4(1.0f, 0.9f, 0.4f, 1.0f)
                                     : ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
            ImGui::TextColored(col, "%c%04X  %-8s  %-28s", at_pc ? '>' : ' ', addr, bytes.c_str(),
                               text.c_str());
            if (!comment.empty()) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.45f, 0.75f, 0.45f, 1.0f), "; %s", comment.c_str());
            }

            // Data lines advance by whatever the source says; code by the
            // instruction length.
            int step = ins.length;
            if (app.disasm_source && src && src->is_data) {
                const auto next = app.symbols.line_at(static_cast<std::uint16_t>(addr + 1));
                (void)next;
            }
            addr = static_cast<std::uint16_t>(addr + step);
        }
    }
    ImGui::EndChild();
    ImGui::End();
}


// ------------------------------------------------------------- VRAM inspector

void draw_vram(AppState& app) {
    if (!app.show_vram) return;
    if (!ImGui::Begin("VRAM", &app.show_vram)) { ImGui::End(); return; }

    const ein::Tms9129& v = app.machine.vdp;
    ImGui::Text("name %04X  pattern %04X  colour %04X  sprites %04X", v.name_base(),
                v.pattern_base(), v.colour_base(), v.sprite_pat_base());
    ImGui::SetNextItemWidth(120);
    ImGui::InputInt("address", &app.vram_addr, 0, 0, ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::SameLine();
    if (ImGui::Button("name")) app.vram_addr = v.name_base();
    ImGui::SameLine();
    if (ImGui::Button("pattern")) app.vram_addr = v.pattern_base();
    ImGui::SameLine();
    if (ImGui::Button("colour")) app.vram_addr = v.colour_base();
    ImGui::Separator();

    if (ImGui::BeginChild("vram")) {
        const int start = app.vram_addr & 0x3ff0;
        for (int row = 0; row < 32; ++row) {
            const int base = start + row * 16;
            if (base >= 0x4000) break;
            std::string hex, ascii;
            for (int i = 0; i < 16; ++i) {
                const std::uint8_t b = v.vram(static_cast<std::uint16_t>(base + i));
                char t[4];
                std::snprintf(t, sizeof t, "%02X", b);
                hex += t;
                hex += ' ';
                ascii.push_back((b >= 0x20 && b < 0x7f) ? static_cast<char>(b) : '.');
            }
            ImGui::Text("%04X  %s %s", base, hex.c_str(), ascii.c_str());
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

}  // namespace albert
