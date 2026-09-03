// Albert -- an emulator for the Tatung Einstein TC-01.
//
// This header holds the frontend's shared state and the seams between its
// translation units.
//
// Everything here was one 1700-line main.cpp until M10 phase 4. The core is
// still SDL- and ImGui-free; this header is the only place the frontend's own
// dependencies are gathered, and it is deliberately the whole set rather than a
// per-file minimum, because the panels move between files often enough that
// bookkeeping the includes would cost more than it saves.
#pragma once

#include <SDL3/SDL.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "imgui_internal.h"  // DockBuilder, for the default layout
#include "card80.h"
#include "disasm.h"
#include "i8251.h"
#include "machine.h"
#include "rom.h"
#include "symbols.h"
#include "z80pio.h"

#ifndef EIN_ROMS_DIR
#define EIN_ROMS_DIR "roms"
#endif

namespace albert {

// What a release ships as, and the first thing looked for.
constexpr const char* kDefaultRomName = "einstein.rom";
constexpr const char* kStateFile = "albert.state";

// Emulation speed is slaved to how fast the audio device consumes samples, not
// to a sleep or a fixed T-state budget. See audio.cpp.
constexpr int kAudioTargetMs = 60;  // queue depth to aim for
constexpr int kAudioMaxMs = 120;    // never run further ahead than this

// A joystick direction or button, bound to a host key. The Einstein's sticks
// are analogue, read through the ADC0844, so a direction is an axis driven to
// one end rather than a switch closing.
//
// **Every orientation here was measured against Oh Mummy rather than assumed**,
// because the archive documents the ports but not what a stick does with them.
// The method was: boot the game, choose its own Joystick option, snapshot the
// machine, then drive one thing and compare the screen against the identical
// state left alone.
//
//   ADC channel 0 is X -- 0 walks left, 255 walks right.
//   ADC channel 1 is Y -- **0 walks down, 255 walks up**, which is the way
//     round a screen coordinate would not be, and is exactly why it was worth
//     measuring instead of guessing.
//   Fire is **port 20 bit 0, active low** -- the same port as SHIFT, CONTROL
//     and GRAPH. Waiting at "PRESS 'C'/'FIRE' TO CONTINUE" the game reads
//     ports 02 and 20 and nothing else, so fire is neither an ADC channel nor
//     a matrix position; driving bit 0 low is what gets past that screen.
//
// The interrupt mask at port 25 and the vector IFIRE at FB0C (Einstein.zsm:34,
// 150) are a separate mechanism, and still not modelled: MOS's handler for
// that vector is the EI/RETI stub at LFC4E and nothing here determines where
// it sits in the daisy chain. A game polling the button does not need it.
//
// Joystick 2's fire is assumed to be bit 1 by symmetry. That one is a guess --
// there is no second stick here to check it against.
// **There are two joystick interfaces, and software uses one or the other.**
// Oh Mummy reads the analogue sticks on the ADC0844; COBRA reads a *digital*
// stick on Z80 PIO port B (port 32) and touches nothing else at all -- not the
// ADC, not the keyboard, not port 20. Both are driven from the same bindings
// below, because they are separate hardware and a game reads whichever it was
// written for.
//
// The port B bits, active low, measured against COBRA the same way as the rest:
// hold one bit and watch where the helicopter goes.
//
//   bit 0 down, bit 1 left, bit 2 up, bit 3 right, bit 4 fire
//
// Fire is bit 4 on the evidence of a bullet appearing -- eight extra white
// pixels on screen when it is pulsed, and nothing for any other bit.
// Joystick 2's place on the PIO is unknown; five bits do not fit in the three
// that are left, so it drives the ADC only.
constexpr std::uint8_t kPioJoyDown = 0x01;
constexpr std::uint8_t kPioJoyLeft = 0x02;
constexpr std::uint8_t kPioJoyUp = 0x04;
constexpr std::uint8_t kPioJoyRight = 0x08;
constexpr std::uint8_t kPioJoyFire = 0x10;

enum JoyInput {
    kJoyUp = 0,
    kJoyDown,
    kJoyLeft,
    kJoyRight,
    kJoyFire1,
    kJoyFire2,
    kJoyInputCount,
};

struct JoyBinding {
    SDL_Scancode key = SDL_SCANCODE_UNKNOWN;
    // Fire only: where on the 8x8 matrix the button presses. Ignored for the
    // four directions, which drive the ADC.
    int row = -1;
    int col = -1;
};

struct Joystick {
    bool enabled = false;
    JoyBinding bind[kJoyInputCount];
    // Which ADC channels this stick's X and Y arrive on. Joystick 1 is
    // channels 0 and 1, joystick 2 is 2 and 3.
    int axis_x = 0;
    int axis_y = 1;
    // The active-low bit this stick's fire pulls down on port 20.
    std::uint8_t fire_bit = 0x01;
    // Live state, so the panel can show what the emulator thinks is held.
    bool held[kJoyInputCount] = {};
};

constexpr int kJoysticks = 2;

struct AppState {
    ein::Machine machine;
    std::string rom_path;
    std::string load_error;
    // The machine runs. Pausing is something you ask for, not the state you
    // are left in after a reset or a cold start with no disc in the drive.
    bool running = true;
    int goto_addr = 0;
    bool scroll_to_addr = false;
    bool rebuild_layout = false;
    std::vector<std::uint32_t> framebuffer =
        std::vector<std::uint32_t>(ein::Tms9129::kWidth * ein::Tms9129::kHeight);
    SDL_Texture* screen_tex = nullptr;
    // The application icon, for the About window. Null if it could not be had.
    SDL_Texture* about_icon = nullptr;
    int screen_zoom = 2;
    std::string disk_error;
    char disk_path[512] = "";
    ein::Symbols symbols;
    std::string symbols_note;
    std::string state_note;
    int disasm_addr = 0;
    bool disasm_follow = true;
    bool disasm_source = true;
    int vram_addr = 0;
    SDL_AudioStream* audio = nullptr;
    bool audio_ok = false;
    bool sound_on = true;
    int queued_ms = 0;

    SDL_Window* window = nullptr;
    std::string window_title;

    // Emulator mode is the default: the screen is the window, and the debugger
    // is summoned. Debugger mode is the old dockspace, unchanged.
    bool emulator_mode = true;
    bool integer_scale = false;
    bool screen_focused = false;
    // The Screen panel scales to fit its window, the way emulator mode fits
    // the client area. Turn it off to pin it to the zoom factor instead.
    bool screen_fit = true;
    // Set when debugger mode is entered: check the dockspace actually has a
    // layout before trusting it. See the frame loop.
    bool check_layout = true;

    // Panel visibility, within debugger mode.
    bool show_screen = true;
    bool show_rom = true;
    bool show_registers = true;
    bool show_control = true;
    bool show_memory = true;
    bool show_vdp = true;
    bool show_disks = true;
    bool show_sound = true;
    bool show_keyboard = true;
    bool show_peripherals = true;
    bool show_disasm = true;
    bool show_vram = true;
    bool show_about = false;
    bool show_keymap = false;
    bool show_joystick = false;

    bool fullscreen = false;

    // Ctrl-BREAK, performed by the emulator rather than by the user's hands.
    // Windows makes the real chord unreachable: the Pause key sends a
    // different code when Ctrl is held (it becomes Break), and SDL reports
    // that as SDL_SCANCODE_UNKNOWN -- see SDL_windowsevents.c. So the gesture
    // is offered as an action, and timed here because the firmware requires
    // CONTROL to outlive BREAK.
    std::uint64_t boot_gesture_start = 0;
    bool boot_gesture = false;

    // Files. SDL's dialogs are asynchronous and may call back from another
    // thread, so the callback parks its answer here and the frame loop acts.
    struct Pending {
        SDL_Mutex* lock = nullptr;
        int kind = 0;
        std::string path;
        bool ready = false;
        bool open = false;
    } pending;
    std::vector<std::string> recent;
    bool save_config_on_exit = true;

    Joystick joy[kJoysticks];
    // Set while the joystick window is waiting for a key to bind.
    int binding_stick = -1;
    int binding_input = -1;

    // The frame loop already had to choose between following the audio device
    // and a fixed budget; this makes the choice visible and adds a turbo.
    enum Speed { kSpeedAuto = 0, kSpeedFrame = 1, kSpeedUnthrottled = 2 };
    int speed = kSpeedAuto;

    // Measured, for the status bar: T-states against wall time, not a target.
    std::uint64_t speed_tstates = 0;
    std::uint64_t speed_ticks = 0;
    double speed_percent = 0.0;
};

enum DialogKind {
    kDlgNone = 0,
    kDlgOpen,
    kDlgInsert0,
    kDlgInsert1,
    kDlgSaveSnapshot,
    kDlgLoadSnapshot,
    kDlgSavePrinter,
    kDlgSaveSerial,
};

// files.cpp
// Where the ROM archive lives. A "roms" folder beside the executable wins, so
// a distributed copy is self-contained; the path compiled in at build time is
// the fallback, which is what lets the build tree run against the checkout.
const std::filesystem::path& roms_dir();
// The ROM to load when nothing says which: "einstein.rom" if it is there,
// otherwise the first .rom in the archive whose contents are recognised. A
// dump keeps whatever name it arrived with and still works.
std::filesystem::path default_rom_path();
// A ROM in the archive whose contents identify as `identity` -- "MOS 1.2" or
// "MOS 1.21" -- or an empty path if there is none. Matched on MD5, not name.
std::filesystem::path find_rom(const std::string& identity);
void load(AppState& app, const std::filesystem::path& path);
void load_symbols(AppState& app);
bool looks_like_disc(const std::filesystem::path& path);
void insert_disc(AppState& app, int drive, const std::filesystem::path& path);
void open_path(AppState& app, const std::filesystem::path& path);
void note_recent(AppState& app, const std::string& path);
void write_state(AppState& app, const std::filesystem::path& path);
void read_state(AppState& app, const std::filesystem::path& path);
void save_state_file(AppState& app);
void load_state_file(AppState& app);
void show_dialog(AppState& app, int kind);
void pump_dialog(AppState& app);
void save_config(const AppState& app);
void load_config(AppState& app);

// audio.cpp
void drain_psg_to_device(AppState& app);
int queued_samples(AppState& app);
long long run_for_audio(AppState& app);

// screen.cpp
void update_screen_texture(AppState& app, SDL_Renderer* renderer);
// The application icon as a texture, read back out of the executable's own
// resources rather than embedded a second time -- the About window and the
// taskbar then cannot disagree about what Albert looks like. Windows only;
// null anywhere else, and null on failure, which callers must tolerate.
SDL_Texture* load_app_icon(SDL_Renderer* renderer, int px);
SDL_FRect emulator_screen_rect(const AppState& app);
void draw_screen_panel(AppState& app);

// panels.cpp
void build_default_layout(ImGuiID dockspace_id);
void draw_rom_panel(AppState& app);
void draw_registers(AppState& app);
void draw_controls(AppState& app);
void draw_memory(AppState& app);
void draw_vdp(AppState& app);
void draw_disks(AppState& app);
void draw_sound(AppState& app);
void draw_keyboard(AppState& app);
void draw_peripherals(AppState& app);
void draw_disassembly(AppState& app);
void draw_vram(AppState& app);

// input.cpp
void handle_key(AppState& app, const SDL_KeyboardEvent& e);
void start_boot_gesture(AppState& app);
void update_boot_gesture(AppState& app);
bool machine_owns_keyboard(const AppState& app);
void draw_keymap(AppState& app);
void draw_joystick(AppState& app);
void reset_joystick_defaults(AppState& app);
void apply_joystick(AppState& app);
const char* joy_input_name(int input);

// menu.cpp
void draw_menu(AppState& app, bool& quit);
void draw_status_bar(AppState& app);
void update_speed(AppState& app);
void draw_about(AppState& app);
void update_title(AppState& app);
void toggle_fullscreen(AppState& app);
bool debugger_visible(const AppState& app);
void set_debugger_visible(AppState& app, bool on);
void set_debugger_mode(AppState& app, bool on);

}  // namespace albert
