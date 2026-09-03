// Albert -- the menu bar, the status bar and the help windows.
//
// Split out of main.cpp at M10 phase 4.
#include "app.h"

namespace albert {

// ------------------------------------------------------------------ Menu bar

// Fuse's five menus: File, Options, Machine, Media, Help.
//
// **Accelerators are limited to F9-F12** and that is a hardware constraint, not
// a style choice. Ctrl is the Einstein CONTROL key, Alt is GRAPH, Caps Lock is
// ALPHA, Pause is BREAK, and F1-F8 are the machine's own function keys (see
// kKeyMap, column 7 of every row); the arrows, Enter, Space, Escape, Delete and
// Backspace are all in the matrix too. So Alt cannot open the menu the way it
// does in a normal Windows application -- any title using graphics characters
// would lose GRAPH to the menu bar. F10 is unbound for a related reason:
// opening a menu from the keyboard needs ImGui's nav layer, and navigation is
// off (see main) because every key it wants is a key on the matrix. So the
// menu is mouse-driven, and the accelerators are F9, F11 and F12.

void toggle_fullscreen(AppState& app) {
    app.fullscreen = !app.fullscreen;
    if (app.window) SDL_SetWindowFullscreen(app.window, app.fullscreen);
}

bool debugger_visible(const AppState& app) {
    return app.show_rom || app.show_registers || app.show_control || app.show_memory ||
           app.show_vdp || app.show_disks || app.show_sound || app.show_keyboard ||
           app.show_peripherals || app.show_disasm || app.show_vram;
}

// The mode switch. Entering the debugger with every window closed would look
// like nothing happened, so it brings them back.
void set_debugger_mode(AppState& app, bool on);

void set_debugger_visible(AppState& app, bool on) {
    app.show_rom = app.show_registers = app.show_control = app.show_memory = on;
    app.show_vdp = app.show_disks = app.show_sound = app.show_keyboard = on;
    app.show_peripherals = app.show_disasm = app.show_vram = on;
}

void set_debugger_mode(AppState& app, bool on) {
    app.emulator_mode = !on;
    if (on && !debugger_visible(app)) set_debugger_visible(app, true);
    // Entering the debugger, make sure there is actually a layout to enter.
    if (on) app.check_layout = true;
}


void draw_menu(AppState& app, bool& quit) {
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open...")) show_dialog(app, kDlgOpen);
        if (ImGui::BeginMenu("Recent", !app.recent.empty())) {
            for (const std::string& r : app.recent) {
                if (ImGui::MenuItem(r.c_str())) {
                    open_path(app, r);
                    break;  // open_path rewrites the list under us
                }
            }
            ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Save snapshot", nullptr, false, app.machine.has_rom())) {
            save_state_file(app);
        }
        if (ImGui::MenuItem("Save snapshot as...", nullptr, false, app.machine.has_rom())) {
            show_dialog(app, kDlgSaveSnapshot);
        }
        if (ImGui::MenuItem("Load snapshot")) load_state_file(app);
        if (ImGui::MenuItem("Load snapshot from...")) show_dialog(app, kDlgLoadSnapshot);
        if (!app.state_note.empty()) ImGui::TextDisabled("%s", app.state_note.c_str());
        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "Alt+F4")) quit = true;
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Options")) {
        if (app.audio_ok) {
            ImGui::MenuItem("Sound", nullptr, &app.sound_on);
        } else {
            ImGui::MenuItem("Sound", nullptr, false, false);
            ImGui::TextDisabled("  no audio device");
        }
        if (ImGui::BeginMenu("Sound filter")) {
            // A low-pass on the PSG output, standing in for the internal
            // speaker. Without it, games that park a tone channel at a very
            // short period whistle; see ein::Ay38910::kDefaultLowpassHz.
            const float hz = app.machine.psg.lowpass_hz();
            if (ImGui::MenuItem("Off", nullptr, hz <= 0.0f)) app.machine.psg.set_lowpass(0.0f);
            ImGui::Separator();
            static constexpr float kCutoffs[] = {3000.0f,  4500.0f,  6000.0f,
                                                9000.0f, 12000.0f, 16000.0f};
            for (const float f : kCutoffs) {
                char label[32];
                std::snprintf(label, sizeof label, "%.1f kHz%s", f / 1000.0f,
                              f == ein::Ay38910::kDefaultLowpassHz ? "  (default)" : "");
                if (ImGui::MenuItem(label, nullptr, hz == f)) app.machine.psg.set_lowpass(f);
            }
            ImGui::TextDisabled("  12 dB/octave, ahead of the resampler");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Video")) {
            for (int z = 1; z <= 4; ++z) {
                char label[16];
                std::snprintf(label, sizeof label, "Zoom %dx", z);
                if (ImGui::MenuItem(label, nullptr, app.screen_zoom == z)) app.screen_zoom = z;
            }
            ImGui::TextDisabled("  (the Screen panel; emulator mode fits the window)");
            ImGui::Separator();
            ImGui::MenuItem("Integer scaling", nullptr, &app.integer_scale);
            ImGui::MenuItem("Fit screen panel to its window", nullptr, &app.screen_fit);
            if (ImGui::MenuItem("Full screen", "F11", app.fullscreen)) toggle_fullscreen(app);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Speed")) {
            // Auto is what the frame loop did before this menu existed: follow
            // the audio device if there is one, fall back to a frame budget.
            if (ImGui::MenuItem("Paced to audio", nullptr, app.speed == AppState::kSpeedAuto)) {
                app.speed = AppState::kSpeedAuto;
            }
            if (ImGui::MenuItem("Frame budget", nullptr, app.speed == AppState::kSpeedFrame)) {
                app.speed = AppState::kSpeedFrame;
            }
            if (ImGui::MenuItem("Unthrottled", nullptr,
                                app.speed == AppState::kSpeedUnthrottled)) {
                app.speed = AppState::kSpeedUnthrottled;
            }
            ImGui::EndMenu();
        }
        ImGui::MenuItem("Joysticks...", nullptr, &app.show_joystick);
        ImGui::Separator();
        if (ImGui::MenuItem("Reset window layout")) app.rebuild_layout = true;
        ImGui::MenuItem("Save configuration on exit", nullptr, &app.save_config_on_exit);
        if (ImGui::MenuItem("Save configuration now")) save_config(app);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Machine")) {
        if (ImGui::MenuItem("Reset")) {
            app.machine.reset();
            app.running = true;
        }
        // The disc in drive 0, booted. MOS boots a disc that was already
        // there at power-on by itself; one put in afterwards needs this.
        if (ImGui::MenuItem("Boot disc (Ctrl-BREAK)", nullptr, false,
                            app.machine.fdc.disk(0).loaded() && !app.boot_gesture)) {
            start_boot_gesture(app);
            app.running = true;
        }
        ImGui::Separator();
        if (ImGui::MenuItem(app.running ? "Pause" : "Run", "F9")) app.running = !app.running;
        if (ImGui::MenuItem("Step", nullptr, false, !app.running)) app.machine.step();
        ImGui::Separator();
        if (ImGui::BeginMenu("ROM")) {
            // Offered only if an image that actually is that ROM is present,
            // whatever it happens to be called.
            const bool is121 = app.machine.has_rom() && app.machine.rom().identity == "MOS 1.21";
            const std::filesystem::path mos12 = find_rom("MOS 1.2");
            const std::filesystem::path mos121 = find_rom("MOS 1.21");
            if (ImGui::MenuItem("MOS 1.2", nullptr, app.machine.has_rom() && !is121,
                                !mos12.empty())) {
                load(app, mos12);
                app.running = true;
            }
            if (ImGui::MenuItem("MOS 1.21", nullptr, is121, !mos121.empty())) {
                load(app, mos121);
                app.running = true;
            }
            if (mos12.empty() && mos121.empty()) {
                ImGui::TextDisabled("  no recognised ROM in %s", roms_dir().string().c_str());
            }
            ImGui::EndMenu();
        }
        bool card = app.machine.card80.present;
        if (ImGui::MenuItem("80-column card fitted", nullptr, &card)) {
            app.machine.card80.present = card;
            app.machine.card80.reset();
        }
        if (app.machine.card80.present) ImGui::TextDisabled("  reset for MOS to find it");
        ImGui::Separator();
        bool dbg = !app.emulator_mode;
        if (ImGui::MenuItem("Debugger", "F12", &dbg)) set_debugger_mode(app, dbg);
        if (ImGui::BeginMenu("Debugger windows", !app.emulator_mode)) {
            ImGui::MenuItem("Screen", nullptr, &app.show_screen);
            ImGui::Separator();
            ImGui::MenuItem("Registers", nullptr, &app.show_registers);
            ImGui::MenuItem("Disassembly", nullptr, &app.show_disasm);
            ImGui::MenuItem("Memory", nullptr, &app.show_memory);
            ImGui::MenuItem("VRAM", nullptr, &app.show_vram);
            ImGui::Separator();
            ImGui::MenuItem("Control", nullptr, &app.show_control);
            ImGui::MenuItem("ROM", nullptr, &app.show_rom);
            ImGui::MenuItem("VDP", nullptr, &app.show_vdp);
            ImGui::MenuItem("Sound", nullptr, &app.show_sound);
            ImGui::MenuItem("Keyboard / CTC", nullptr, &app.show_keyboard);
            ImGui::MenuItem("Peripherals", nullptr, &app.show_peripherals);
            ImGui::MenuItem("Discs", nullptr, &app.show_disks);
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Media")) {
        for (int d = 0; d < 2; ++d) {
            char label[32];
            std::snprintf(label, sizeof label, "Drive %d", d);
            if (ImGui::BeginMenu(label)) {
                ein::DiskImage& disk = app.machine.fdc.disk(d);
                if (disk.loaded()) {
                    ImGui::TextDisabled("%s", disk.name().c_str());
                    ImGui::TextDisabled("%s, %d tracks, %d side(s)", disk.format_name().c_str(),
                                        disk.tracks(), disk.sides());
                    ImGui::Separator();
                    ImGui::MenuItem("Write protected", nullptr, &disk.write_protected);
                    if (ImGui::MenuItem("Eject")) disk.eject();
                } else {
                    ImGui::TextDisabled("(empty)");
                    ImGui::Separator();
                }
                if (ImGui::MenuItem("Insert...")) {
                    show_dialog(app, d == 0 ? kDlgInsert0 : kDlgInsert1);
                }
                if (d == 0 && ImGui::MenuItem("Boot it (Ctrl-BREAK)", nullptr, false,
                                              disk.loaded() && !app.boot_gesture)) {
                    start_boot_gesture(app);
                    app.running = true;
                }
                if (ImGui::MenuItem("Insert blank disc")) {
                    disk.format(1, "blank");
                    app.disk_error.clear();
                }
                ImGui::EndMenu();
            }
        }
        ImGui::Separator();
        if (ImGui::BeginMenu("Printer")) {
            ImGui::TextDisabled("%zu bytes", app.machine.printer.size());
            ImGui::Separator();
            if (ImGui::MenuItem("Show output")) {
                app.show_peripherals = true;
                ImGui::SetWindowFocus("Peripherals");
            }
            if (ImGui::MenuItem("Save to file...", nullptr, false,
                                !app.machine.printer.empty())) {
                show_dialog(app, kDlgSavePrinter);
            }
            if (ImGui::MenuItem("Clear", nullptr, false, !app.machine.printer.empty())) {
                app.machine.printer.clear();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Serial")) {
            bool loop = app.machine.uart.loopback;
            if (ImGui::MenuItem("Loopback (TxD to RxD)", nullptr, &loop)) {
                app.machine.uart.loopback = loop;
            }
            ImGui::TextDisabled("%zu byte(s) sent", app.machine.uart.sent().size());
            if (ImGui::MenuItem("Save sent bytes...", nullptr, false,
                                !app.machine.uart.sent().empty())) {
                show_dialog(app, kDlgSaveSerial);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
        ImGui::MenuItem("Keyboard map", nullptr, &app.show_keymap);
        ImGui::Separator();
        ImGui::MenuItem("About", nullptr, &app.show_about);
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}


// ---------------------------------------------------------------- Status bar

// Measured speed, not a target: T-states actually executed against wall time.
void update_speed(AppState& app) {
    const std::uint64_t now = SDL_GetTicks();
    if (app.speed_ticks == 0) {
        app.speed_ticks = now;
        app.speed_tstates = app.machine.tstates;
        return;
    }
    const std::uint64_t dt = now - app.speed_ticks;
    if (dt < 500) return;
    const std::uint64_t done = app.machine.tstates - app.speed_tstates;
    app.speed_percent =
        100.0 * static_cast<double>(done) /
        (static_cast<double>(ein::Machine::kCpuClockHz) * (static_cast<double>(dt) / 1000.0));
    app.speed_ticks = now;
    app.speed_tstates = app.machine.tstates;
}

void draw_status_bar(AppState& app) {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    if (ImGui::BeginViewportSideBar("##status", vp, ImGuiDir_Down, ImGui::GetFrameHeight(),
                                    flags)) {
        if (ImGui::BeginMenuBar()) {
            if (app.running) {
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Running");
            } else {
                ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f), "Paused");
            }
            ImGui::Separator();
            ImGui::Text("%3.0f%%", app.speed_percent);
            ImGui::Separator();
            ImGui::TextUnformatted(app.machine.has_rom() ? app.machine.rom().identity.c_str()
                                                         : "no ROM");
            ImGui::Separator();

            const ein::Fd1770& f = app.machine.fdc;
            const ein::DiskImage& disk = app.machine.fdc.disk(f.selected_drive());
            if (f.motor_on()) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), "drive %d  track %d",
                                   f.selected_drive(), f.head_track());
            } else if (disk.loaded()) {
                ImGui::Text("drive %d  %s", f.selected_drive(), disk.name().c_str());
            } else {
                ImGui::TextDisabled("no disc");
            }
            ImGui::Separator();

            if (app.machine.alpha_led) {
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "ALPHA");
            } else {
                ImGui::TextDisabled("ALPHA");
            }
            ImGui::Separator();
            if (!app.audio_ok) {
                ImGui::TextDisabled("no audio");
            } else if (app.sound_on) {
                ImGui::Text("sound %d ms", app.queued_ms);
            } else {
                ImGui::TextDisabled("muted");
            }
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}


// ------------------------------------------------------------- Help windows

void draw_about(AppState& app) {
    if (!app.show_about) return;
    ImGui::SetNextWindowSize(ImVec2(500, 0), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("About", &app.show_about, ImGuiWindowFlags_NoDocking)) {
        // Icon on the left, everything else in a group beside it, so the text
        // wraps in its own column instead of running under the image. The
        // chip line is the long one and wraps to two, which is also what makes
        // the block roughly as tall as the 96-pixel icon.
        if (app.about_icon) {
            ImGui::Image(reinterpret_cast<ImTextureID>(app.about_icon), ImVec2(96, 96));
            ImGui::SameLine();
        }
        ImGui::BeginGroup();
        ImGui::TextWrapped("Albert: A Tatung Einstein TC-01 emulator for Windows");
        ImGui::TextWrapped("Vibe coded with Claude Code by Steve Dobbs 2026.");
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("Z80 at %d Hz, TMS9129 video, AY-3-8910 sound, FD1770 discs",
                           ein::Machine::kCpuClockHz);
        ImGui::PopStyleColor();
        ImGui::EndGroup();
        ImGui::Separator();
        if (app.machine.has_rom()) {
            const ein::RomImage& rom = app.machine.rom();
            ImGui::Text("ROM      %s", rom.identity.c_str());
            ImGui::Text("md5      %s", rom.md5.c_str());
            ImGui::TextWrapped("%s", app.rom_path.c_str());
        } else {
            ImGui::TextDisabled("no ROM loaded");
        }
        ImGui::Separator();
        ImGui::TextWrapped(
            "The debugger single-steps against Ric Hohne's commented disassembly of MOS 1.2 "
            "(1986-87), which is what makes named labels and source lines possible at all.");
        ImGui::Spacing();
        if (ImGui::Button("Close")) app.show_about = false;
    }
    ImGui::End();
}


// The window title names the media, the way a document application names its
// document. Set only when it changes -- SDL_SetWindowTitle is not free.
void update_title(AppState& app) {
    std::string t;
    const ein::DiskImage& d0 = app.machine.fdc.disk(0);
    if (d0.loaded()) {
        t = d0.name() + " -- Albert";
    } else {
        t = "Albert";
    }
    if (app.machine.has_rom()) t += " (" + app.machine.rom().identity + ")";
    if (t != app.window_title) {
        app.window_title = t;
        if (app.window) SDL_SetWindowTitle(app.window, t.c_str());
    }
}

}  // namespace albert
