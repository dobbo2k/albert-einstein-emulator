// Albert -- the host keyboard, the matrix and the joysticks.
//
// Split out of main.cpp at M10 phase 4.
#include "app.h"

namespace albert {

// --------------------------------------------------------------- Keyboard

// Host key to Einstein matrix position. Rows are the mask bit written to PSG
// register 14 (row r is mask 1<<r); columns are the bits read back from
// register 15. Layout per the ROM key table at L10C5 -- see keyboard.cpp -- with
// matrix row 01 carrying BREAK, ALPHA, ENTER, SPACE, ESC and two function keys.
struct KeyMap {
    SDL_Scancode sc;
    int row;
    int col;
};

constexpr KeyMap kKeyMap[] = {
    // row 01: BREAK, -, F0, F7, ALPHA, ENTER, SPACE, ESC
    // End doubles for BREAK: holding Ctrl turns the Pause key into Break,
    // which Windows reports as a different key entirely and SDL cannot name,
    // so Ctrl+Pause never arrives. Ctrl+End does.
    {SDL_SCANCODE_PAUSE, 0, 0},   {SDL_SCANCODE_END, 0, 0},   {SDL_SCANCODE_F8, 0, 2},      {SDL_SCANCODE_F7, 0, 3},
    {SDL_SCANCODE_CAPSLOCK, 0, 4}, {SDL_SCANCODE_RETURN, 0, 5}, {SDL_SCANCODE_SPACE, 0, 6},
    {SDL_SCANCODE_ESCAPE, 0, 7},
    // row 02: i o p LEFT _ DOWN | 0
    {SDL_SCANCODE_I, 1, 0},       {SDL_SCANCODE_O, 1, 1},       {SDL_SCANCODE_P, 1, 2},
    {SDL_SCANCODE_LEFT, 1, 3},    {SDL_SCANCODE_MINUS, 1, 4},   {SDL_SCANCODE_DOWN, 1, 5},
    {SDL_SCANCODE_BACKSLASH, 1, 6}, {SDL_SCANCODE_0, 1, 7},
    // row 04: k l ; : RIGHT BS 9 F5
    {SDL_SCANCODE_K, 2, 0},       {SDL_SCANCODE_L, 2, 1},       {SDL_SCANCODE_SEMICOLON, 2, 2},
    {SDL_SCANCODE_APOSTROPHE, 2, 3}, {SDL_SCANCODE_RIGHT, 2, 4},
    {SDL_SCANCODE_BACKSPACE, 2, 5}, {SDL_SCANCODE_9, 2, 6},     {SDL_SCANCODE_F5, 2, 7},
    // row 08: , . / 8 DEL = UP F4
    {SDL_SCANCODE_COMMA, 3, 0},   {SDL_SCANCODE_PERIOD, 3, 1},  {SDL_SCANCODE_SLASH, 3, 2},
    {SDL_SCANCODE_8, 3, 3},       {SDL_SCANCODE_DELETE, 3, 4},  {SDL_SCANCODE_EQUALS, 3, 5},
    {SDL_SCANCODE_UP, 3, 6},      {SDL_SCANCODE_F4, 3, 7},
    // row 10: 7 6 5 4 3 2 1 F3
    {SDL_SCANCODE_7, 4, 0},       {SDL_SCANCODE_6, 4, 1},       {SDL_SCANCODE_5, 4, 2},
    {SDL_SCANCODE_4, 4, 3},       {SDL_SCANCODE_3, 4, 4},       {SDL_SCANCODE_2, 4, 5},
    {SDL_SCANCODE_1, 4, 6},       {SDL_SCANCODE_F3, 4, 7},
    // row 20: u y t r e w q F2
    {SDL_SCANCODE_U, 5, 0},       {SDL_SCANCODE_Y, 5, 1},       {SDL_SCANCODE_T, 5, 2},
    {SDL_SCANCODE_R, 5, 3},       {SDL_SCANCODE_E, 5, 4},       {SDL_SCANCODE_W, 5, 5},
    {SDL_SCANCODE_Q, 5, 6},       {SDL_SCANCODE_F2, 5, 7},
    // row 40: j h g f d s a F1
    {SDL_SCANCODE_J, 6, 0},       {SDL_SCANCODE_H, 6, 1},       {SDL_SCANCODE_G, 6, 2},
    {SDL_SCANCODE_F, 6, 3},       {SDL_SCANCODE_D, 6, 4},       {SDL_SCANCODE_S, 6, 5},
    {SDL_SCANCODE_A, 6, 6},       {SDL_SCANCODE_F1, 6, 7},
    // row 80: m n b v c x z F6
    {SDL_SCANCODE_M, 7, 0},       {SDL_SCANCODE_N, 7, 1},       {SDL_SCANCODE_B, 7, 2},
    {SDL_SCANCODE_V, 7, 3},       {SDL_SCANCODE_C, 7, 4},       {SDL_SCANCODE_X, 7, 5},
    {SDL_SCANCODE_Z, 7, 6},       {SDL_SCANCODE_F6, 7, 7},
};

void handle_key(AppState& app, const SDL_KeyboardEvent& e) {
    auto& kbd = app.machine.keyboard;

    // The joystick window is waiting for a key to bind.
    if (app.binding_stick >= 0 && app.binding_input >= 0) {
        if (e.down) {
            if (e.scancode != SDL_SCANCODE_ESCAPE) {
                app.joy[app.binding_stick].bind[app.binding_input].key = e.scancode;
            }
            app.binding_stick = -1;
            app.binding_input = -1;
        }
        return;
    }

    // Joystick bindings are read before the matrix, so a key given to a stick
    // is a stick key and nothing else -- including when it is also a matrix
    // key, which is the user's choice to make.
    for (int j = 0; j < kJoysticks; ++j) {
        Joystick& s = app.joy[j];
        if (!s.enabled) continue;
        for (int i = 0; i < kJoyInputCount; ++i) {
            if (s.bind[i].key == SDL_SCANCODE_UNKNOWN || s.bind[i].key != e.scancode) continue;
            s.held[i] = e.down;
            if ((i == kJoyFire1 || i == kJoyFire2) && s.bind[i].row >= 0) {
                kbd.set_key(s.bind[i].row, s.bind[i].col, e.down);
            }
            return;
        }
    }
    switch (e.scancode) {
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT:
            kbd.set_modifier(ein::Keyboard::kShift, e.down);
            return;
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_RCTRL:
            kbd.set_modifier(ein::Keyboard::kControl, e.down);
            return;
        case SDL_SCANCODE_LALT:
        case SDL_SCANCODE_RALT:
            kbd.set_modifier(ein::Keyboard::kGraph, e.down);
            return;
        default:
            break;
    }
    for (const KeyMap& k : kKeyMap) {
        if (k.sc == e.scancode) {
            kbd.set_key(k.row, k.col, e.down);
            return;
        }
    }
}


// Who owns the host keyboard. Keyboard navigation is off entirely (see main),
// so ImGui only claims keys for a text field or an open menu; everything else
// belongs to the matrix.
bool machine_owns_keyboard(const AppState& app) {
    const ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput) return false;
    if (app.emulator_mode && io.WantCaptureKeyboard) return false;
    return true;
}

void draw_keymap(AppState& app) {
    if (!app.show_keymap) return;
    ImGui::SetNextWindowSize(ImVec2(460, 420), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Keyboard map", &app.show_keymap)) {
        ImGui::TextWrapped("Host keys to the Einstein's 8x8 matrix. The modifiers are separate "
                           "lines on port 20, not matrix positions.");
        ImGui::Separator();
        ImGui::Text("Caps Lock   ALPHA");
        ImGui::Text("Pause       BREAK");
        ImGui::Text("Alt         GRAPH");
        ImGui::Text("Shift       SHIFT");
        ImGui::Text("Ctrl        CONTROL");
        ImGui::TextDisabled("Ctrl+Pause boots a disc -- hold Ctrl a moment longer than Pause,");
        ImGui::TextDisabled("because ZKSCAN reads the modifiers after BREAK is released.");
        ImGui::Separator();
        if (ImGui::BeginTable("keys", 3,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("host key");
            ImGui::TableSetupColumn("row");
            ImGui::TableSetupColumn("col");
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();
            for (const KeyMap& k : kKeyMap) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                const char* name = SDL_GetScancodeName(k.sc);
                ImGui::TextUnformatted((name && *name) ? name : "?");
                ImGui::TableNextColumn();
                ImGui::Text("%d", k.row);
                ImGui::TableNextColumn();
                ImGui::Text("%d", k.col);
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

// ------------------------------------------------------------- Joysticks

const char* joy_input_name(int input) {
    switch (input) {
        case kJoyUp: return "Up";
        case kJoyDown: return "Down";
        case kJoyLeft: return "Left";
        case kJoyRight: return "Right";
        case kJoyFire1: return "Fire 1";
        case kJoyFire2: return "Fire 2";
        default: return "?";
    }
}

void reset_joystick_defaults(AppState& app) {
    for (int j = 0; j < kJoysticks; ++j) {
        app.joy[j] = Joystick{};
        app.joy[j].axis_x = j * 2;
        app.joy[j].axis_y = j * 2 + 1;
        app.joy[j].fire_bit = static_cast<std::uint8_t>(1u << j);
    }
    // Joystick 1 lands on the numeric keypad, which is the one stretch of the
    // host keyboard the Einstein's matrix does not claim -- so the default
    // costs the machine no keys. Fire presses SPACE, the usual choice.
    Joystick& one = app.joy[0];
    one.bind[kJoyUp].key = SDL_SCANCODE_KP_8;
    one.bind[kJoyDown].key = SDL_SCANCODE_KP_2;
    one.bind[kJoyLeft].key = SDL_SCANCODE_KP_4;
    one.bind[kJoyRight].key = SDL_SCANCODE_KP_6;
    one.bind[kJoyFire1].key = SDL_SCANCODE_KP_0;
    // No matrix key by default: fire has a real line of its own on port 20.
    // The matrix option stays for games that read a key instead.
}

// Ctrl-BREAK is what boots a disc put in after the machine has started, and
// it is not a simple chord: ZKSCAN waits for BREAK to be released and only
// then reads the modifier keys, so CONTROL has to still be down after BREAK
// has come up (Mos12.zsm:1336-1348). Held by the clock rather than by the
// user, over about eight tenths of a second.
void start_boot_gesture(AppState& app) {
    app.boot_gesture = true;
    app.boot_gesture_start = SDL_GetTicks();
}

void update_boot_gesture(AppState& app) {
    if (!app.boot_gesture) return;
    const std::uint64_t t = SDL_GetTicks() - app.boot_gesture_start;
    ein::Keyboard& kbd = app.machine.keyboard;
    if (t < 200) {
        kbd.set_modifier(ein::Keyboard::kControl, true);
    } else if (t < 500) {
        kbd.set_key(0, 0, true);  // BREAK, row 01 column 0
    } else if (t < 800) {
        kbd.set_key(0, 0, false);  // released, CONTROL still down
    } else {
        kbd.set_modifier(ein::Keyboard::kControl, false);
        app.boot_gesture = false;
    }
}

// Directions are analogue: the axis goes to one end and back to 128 at rest,
// which is what the ADC0844 reports and what a game reading it expects.
// Opposite directions held together cancel, as they do on a real stick.
//
// Y runs the opposite way to a screen coordinate -- 0 is DOWN and 255 is UP.
// That was measured against Oh Mummy, not assumed; see the note in app.h.
// Fire pulls its bit low on port 20, the modifier port, which is where the
// game actually looks for it.
void apply_joystick(AppState& app) {
    std::uint8_t fire_low = 0;
    std::uint8_t pio_b = 0xff;  // active low, so idle is all ones
    for (int j = 0; j < kJoysticks; ++j) {
        Joystick& s = app.joy[j];
        if (!s.enabled) continue;
        std::uint8_t x = 128;
        std::uint8_t y = 128;
        if (s.held[kJoyLeft] && !s.held[kJoyRight]) x = 0;
        else if (s.held[kJoyRight] && !s.held[kJoyLeft]) x = 255;
        if (s.held[kJoyDown] && !s.held[kJoyUp]) y = 0;
        else if (s.held[kJoyUp] && !s.held[kJoyDown]) y = 255;
        app.machine.adc.set_axis(s.axis_x, x);
        app.machine.adc.set_axis(s.axis_y, y);
        if (s.held[kJoyFire1] || s.held[kJoyFire2]) fire_low |= s.fire_bit;

        // The digital stick on PIO port B, which is a different interface to
        // the ADC rather than a different encoding of it: a game reads one or
        // the other. Only stick 1 has a known place there.
        if (j == 0) {
            if (s.held[kJoyDown]) pio_b &= static_cast<std::uint8_t>(~kPioJoyDown);
            if (s.held[kJoyLeft]) pio_b &= static_cast<std::uint8_t>(~kPioJoyLeft);
            if (s.held[kJoyUp]) pio_b &= static_cast<std::uint8_t>(~kPioJoyUp);
            if (s.held[kJoyRight]) pio_b &= static_cast<std::uint8_t>(~kPioJoyRight);
            if (s.held[kJoyFire1] || s.held[kJoyFire2]) {
                pio_b &= static_cast<std::uint8_t>(~kPioJoyFire);
            }
        }
    }
    // All written every frame, including when nothing is held, so letting go
    // of a direction actually lets go of it.
    app.machine.keyboard.set_port20_low(fire_low);
    app.machine.pio.set_input(1, pio_b);
}

void draw_joystick(AppState& app) {
    if (!app.show_joystick) return;
    ImGui::SetNextWindowSize(ImVec2(520, 480), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Joysticks", &app.show_joystick)) {
        ImGui::TextWrapped(
            "The Einstein's sticks are analogue. A direction drives an ADC0844 axis to one end "
            "rather than closing a switch, and lets go back to 128 at rest.");
        ImGui::TextWrapped(
            "Fire pulls port 20 bit 0 low -- the same port as SHIFT, CONTROL and GRAPH. That was "
            "found by experiment: waiting at Oh Mummy's \"PRESS 'C'/'FIRE'\" the game reads only "
            "ports 02 and 20, and bit 0 is the one that gets past it. A fire button can also "
            "press a key on the matrix, for games that want a key instead.");
        ImGui::TextWrapped(
            "Up is 255 and down is 0 on the Y axis, which is the opposite way round to a screen "
            "coordinate. Measured, not assumed.");
        ImGui::TextWrapped(
            "There are two joystick interfaces and software uses one or the other, so both are "
            "driven at once: the analogue sticks on the ADC (Oh Mummy) and a digital stick on Z80 "
            "PIO port B, port 32 (COBRA, which reads nothing else at all -- not the ADC, not the "
            "keyboard). Only joystick 1 has a known place on the PIO.");
        ImGui::Separator();

        for (int j = 0; j < kJoysticks; ++j) {
            ImGui::PushID(j);
            Joystick& s = app.joy[j];
            char title[32];
            std::snprintf(title, sizeof title, "Joystick %d", j + 1);
            if (ImGui::CollapsingHeader(title, j == 0 ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
                if (ImGui::Checkbox("enabled", &s.enabled) && !s.enabled) {
                    // Let the axes go, or the Peripherals sliders stay stuck
                    // wherever the stick last was.
                    for (bool& h : s.held) h = false;
                    app.machine.adc.set_axis(s.axis_x, 128);
                    app.machine.adc.set_axis(s.axis_y, 128);
                }
                ImGui::SameLine();
                ImGui::TextDisabled("ADC channel %d is X, %d is Y", s.axis_x, s.axis_y);

                if (ImGui::BeginTable("binds", 4,
                                      ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("input");
                    ImGui::TableSetupColumn("host key");
                    ImGui::TableSetupColumn("");
                    ImGui::TableSetupColumn("state");
                    ImGui::TableHeadersRow();
                    for (int i = 0; i < kJoyInputCount; ++i) {
                        ImGui::PushID(i);
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(joy_input_name(i));
                        ImGui::TableNextColumn();
                        if (app.binding_stick == j && app.binding_input == i) {
                            ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.3f, 1.0f),
                                               "press a key (Esc to cancel)");
                        } else {
                            const char* n = SDL_GetScancodeName(s.bind[i].key);
                            if (n != nullptr && *n != '\0') ImGui::TextUnformatted(n);
                            else ImGui::TextDisabled("(unbound)");
                        }
                        ImGui::TableNextColumn();
                        if (ImGui::SmallButton("Set")) {
                            app.binding_stick = j;
                            app.binding_input = i;
                        }
                        ImGui::SameLine();
                        if (ImGui::SmallButton("Clear")) s.bind[i].key = SDL_SCANCODE_UNKNOWN;
                        ImGui::TableNextColumn();
                        if (s.held[i]) ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "held");
                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }

                for (int i = kJoyFire1; i <= kJoyFire2; ++i) {
                    ImGui::PushID(64 + i);
                    JoyBinding& b = s.bind[i];
                    const char* cur = "(nothing)";
                    for (const KeyMap& k : kKeyMap) {
                        if (k.row == b.row && k.col == b.col) {
                            const char* n = SDL_GetScancodeName(k.sc);
                            if (n != nullptr && *n != '\0') cur = n;
                            break;
                        }
                    }
                    char label[32];
                    std::snprintf(label, sizeof label, "%s presses", joy_input_name(i));
                    ImGui::SetNextItemWidth(180.0f);
                    if (ImGui::BeginCombo(label, cur)) {
                        if (ImGui::Selectable("(nothing)", b.row < 0)) {
                            b.row = -1;
                            b.col = -1;
                        }
                        for (const KeyMap& k : kKeyMap) {
                            const char* n = SDL_GetScancodeName(k.sc);
                            if (n == nullptr || *n == '\0') continue;
                            if (ImGui::Selectable(n, k.row == b.row && k.col == b.col)) {
                                b.row = k.row;
                                b.col = k.col;
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::PopID();
                }
            }
            ImGui::PopID();
        }

        ImGui::Separator();
        if (ImGui::Button("Restore defaults")) reset_joystick_defaults(app);
        ImGui::SameLine();
        ImGui::TextDisabled("keypad for joystick 1; the matrix does not use it");
    }
    ImGui::End();
}

}  // namespace albert
