// Albert -- application entry point.
//
// Everything else was split out at M10 phase 4; what is left is the SDL and
// ImGui setup, the command line, and the frame loop.
#include "app.h"

using namespace albert;

int main(int argc, char** argv) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("Albert", 1280, 800,
                                     SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        std::fprintf(stderr, "SDL_CreateWindowAndRenderer failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderVSync(renderer, 1);

    // Build the default layout only when there is no saved one to respect.
    constexpr const char* kIniFile = "albert-layout-v3.ini";
    const bool had_ini = std::filesystem::exists(kIniFile);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // The layout is saved by hand rather than by ImGui, because emulator mode
    // never submits the panels: letting ImGui write the file from there
    // records a layout with no docking in it at all, which silently destroys
    // whatever arrangement the debugger had. See the frame loop.
    io.IniFilename = nullptr;
    if (had_ini) ImGui::LoadIniSettingsFromDisk(kIniFile);
    // Keyboard navigation stays OFF, and this is forced rather than chosen:
    // every key it wants -- the arrows, Enter, Space, Escape -- is a key on the
    // Einstein's matrix, and Alt, which opens ImGui's menu layer, is GRAPH.
    // There is no sharing them. The mouse drives the interface; the keyboard
    // belongs to the machine. (This is also why F10 does not open the menu.)
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Args: [rom-path] [--steps N]
    std::filesystem::path rom_arg = default_rom_path();
    long long presteps = 0;
    std::filesystem::path disk_arg[2];
    bool col80 = false;
    bool start_in_debugger = false;
    std::vector<std::string> named;  // paths named on the command line
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--steps") == 0 && i + 1 < argc) {
            presteps = std::atoll(argv[++i]);
        } else if (std::strcmp(argv[i], "--disk") == 0 && i + 1 < argc) {
            disk_arg[0] = argv[++i];
            named.push_back(argv[i]);
        } else if (std::strcmp(argv[i], "--col80") == 0) {
            col80 = true;
        } else if (std::strcmp(argv[i], "--debugger") == 0) {
            start_in_debugger = true;
        } else if (std::strcmp(argv[i], "--disk1") == 0 && i + 1 < argc) {
            disk_arg[1] = argv[++i];
            named.push_back(argv[i]);
        } else if (argv[i][0] != '-') {
            // A bare path is the ROM, unless it looks like a disc image --
            // the same rule the dialogs and dropped files go through.
            if (looks_like_disc(argv[i])) disk_arg[0] = argv[i];
            else rom_arg = argv[i];
            named.push_back(argv[i]);
        }
    }

    AppState app;
    app.pending.lock = SDL_CreateMutex();
    reset_joystick_defaults(app);
    // Settings first, so anything named on the command line overrides them.
    load_config(app);

    // Mono float at the PSG's own output rate, so nothing has to resample.
    SDL_AudioSpec want{};
    want.format = SDL_AUDIO_F32;
    want.channels = 1;
    want.freq = ein::Ay38910::kSampleRate;
    app.audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &want, nullptr,
                                          nullptr);
    if (app.audio) {
        SDL_ResumeAudioStreamDevice(app.audio);
        app.audio_ok = true;
    } else {
        std::fprintf(stderr, "audio unavailable (%s); pacing on a frame budget\n",
                     SDL_GetError());
    }

    app.window = window;
    // 128 is a real entry in the icon, so this is a lookup rather than a
    // stretch; the About window draws it smaller and lets the GPU filter it.
    app.about_icon = load_app_icon(renderer, 128);
    if (start_in_debugger) set_debugger_mode(app, true);
    app.rebuild_layout = !had_ini;
    if (col80) app.machine.card80.present = true;
    load(app, rom_arg);
    for (int d = 0; d < 2; ++d) {
        if (!disk_arg[d].empty()) {
            insert_disc(app, d, disk_arg[d]);
            std::snprintf(app.disk_path, sizeof app.disk_path, "%s", disk_arg[d].string().c_str());
            // A machine started with a disc in it is meant to boot from it.
            app.running = true;
        }
    }
    if (!app.disk_error.empty()) {
        std::fprintf(stderr, "disc: %s\n", app.disk_error.c_str());
    }

    // Anything named on the command line belongs in the recent list, newest
    // first -- so walk backwards and let the first one named end up at the top.
    for (auto it = named.rbegin(); it != named.rend(); ++it) note_recent(app, *it);

    // Run headlessly before opening the window; useful for landing on a known
    // point in the ROM without clicking Step.
    for (long long i = 0; i < presteps && !app.machine.cpu.halted; ++i) {
        app.machine.step();
    }

    bool quit = false;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL3_ProcessEvent(&e);
            if (e.type == SDL_EVENT_QUIT) quit = true;
            // A disc or a ROM dropped on the window, by the same rule as the
            // command line: the extension decides which it is.
            if (e.type == SDL_EVENT_DROP_FILE && e.drop.data != nullptr) {
                open_path(app, std::filesystem::path(e.drop.data));
            }
            if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                e.window.windowID == SDL_GetWindowID(window)) {
                quit = true;
            }
            // F9-F12 are the only host keys the Einstein does not claim,
            // so they are the whole accelerator budget. Taken here, before the
            // matrix, though none of them is in kKeyMap anyway.
            if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat && app.binding_stick < 0) {
                if (e.key.scancode == SDL_SCANCODE_F9) {
                    app.running = !app.running;
                    continue;
                }
                if (e.key.scancode == SDL_SCANCODE_F11) {
                    toggle_fullscreen(app);
                    continue;
                }
                if (e.key.scancode == SDL_SCANCODE_F12) {
                    set_debugger_mode(app, app.emulator_mode);
                    continue;
                }
            }

            // Host keys drive the matrix unless ImGui is collecting text.
            if ((e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP) &&
                machine_owns_keyboard(app) && !e.key.repeat) {
                handle_key(app, e.key);
            }
        }

        if (app.running && app.machine.bp_hit()) app.running = false;
        if (app.running) {
            if (app.speed == AppState::kSpeedAuto && app.audio_ok && app.sound_on) {
                // Speed comes from the audio device's appetite.
                run_for_audio(app);
            } else {
                // No device, muted, or a speed chosen by hand: a fixed budget
                // per frame, ten frames' worth of it when unthrottled.
                const int budget = app.speed == AppState::kSpeedUnthrottled
                                       ? ein::Machine::kCpuClockHz / 6
                                       : ein::Machine::kCpuClockHz / 60;
                int spent = 0;
                while (spent < budget) {
                    const auto r = app.machine.step();
                    if (!r.ok) { app.running = false; break; }
                    spent += r.tstates;
                }
                app.machine.psg.clear_samples();
            }
        } else {
            // Paused: do not let stale audio pile up.
            app.machine.psg.clear_samples();
        }

        pump_dialog(app);
        apply_joystick(app);
        update_boot_gesture(app);
        update_speed(app);
        update_title(app);

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Menu bar and status bar first: both shrink the viewport work area,
        // and everything after them -- the dockspace, or the picture in
        // emulator mode -- has to be laid out inside what is left.
        draw_menu(app, quit);
        draw_status_bar(app);

        update_screen_texture(app, renderer);

        SDL_FRect screen_rect{};
        if (app.emulator_mode) {
            // No dockspace and no panels: the window is the screen.
            screen_rect = emulator_screen_rect(app);
            app.screen_focused = true;
        } else {
            const ImGuiID dock_id = ImGui::DockSpaceOverViewport();
            // A dockspace whose node has no split and no windows means the
            // layout was never built or has been lost -- the panels would come
            // up floating on top of one another. Checked when the debugger is
            // entered rather than every frame, so deliberately undocking
            // everything is still allowed to stand.
            bool empty_layout = false;
            if (app.check_layout) {
                app.check_layout = false;
                const ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_id);
                empty_layout = (node == nullptr) ||
                               (node->IsLeafNode() && node->Windows.Size == 0);
            }
            if (app.rebuild_layout || empty_layout) {
                app.rebuild_layout = false;
                build_default_layout(dock_id);
            }

            draw_screen_panel(app);
            draw_vdp(app);
            draw_keyboard(app);
            draw_disks(app);
            draw_sound(app);
            draw_peripherals(app);
            draw_disassembly(app);
            draw_vram(app);
            draw_rom_panel(app);
            draw_registers(app);
            draw_controls(app);
            draw_memory(app);
        }
        draw_about(app);
        draw_keymap(app);
        draw_joystick(app);

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 18, 18, 22, 255);
        SDL_RenderClear(renderer);
        if (app.emulator_mode && app.screen_tex) {
            SDL_RenderTexture(renderer, app.screen_tex, nullptr, &screen_rect);
        }
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

        // Persist the layout only from debugger mode, where the panels exist.
        if (io.WantSaveIniSettings) {
            io.WantSaveIniSettings = false;
            if (!app.emulator_mode) ImGui::SaveIniSettingsToDisk(kIniFile);
        }
    }

    if (!app.emulator_mode) ImGui::SaveIniSettingsToDisk(kIniFile);
    if (app.save_config_on_exit) save_config(app);
    if (app.pending.lock) SDL_DestroyMutex(app.pending.lock);
    if (app.screen_tex) SDL_DestroyTexture(app.screen_tex);
    if (app.about_icon) SDL_DestroyTexture(app.about_icon);
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    if (app.audio) SDL_DestroyAudioStream(app.audio);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
