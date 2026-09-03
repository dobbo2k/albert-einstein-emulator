// Albert -- loading ROMs, discs, snapshots and settings.
//
// Split out of main.cpp at M10 phase 4.
#include "app.h"

namespace albert {

void load(AppState& app, const std::filesystem::path& path) {
    app.rom_path = path.string();
    app.load_error.clear();
    try {
        app.machine.set_rom(ein::load_rom(path));
        app.machine.reset();
    } catch (const std::exception& e) {
        app.load_error = e.what();
    }
    load_symbols(app);
}

const std::filesystem::path& roms_dir() {
    static const std::filesystem::path dir = [] {
        std::error_code ec;
        // SDL hands back the directory the executable is in, and owns the
        // string -- it must not be freed.
        const char* base = SDL_GetBasePath();
        const std::filesystem::path beside =
            (base != nullptr) ? std::filesystem::path(base) / "roms"
                              : std::filesystem::path("roms");
        if (std::filesystem::exists(beside, ec)) return beside;
        const std::filesystem::path built(EIN_ROMS_DIR);
        if (std::filesystem::exists(built, ec)) return built;
        // Neither: name the one a user can actually create, so the error
        // message points somewhere useful.
        return beside;
    }();
    return dir;
}

namespace {

struct RomEntry {
    std::filesystem::path path;
    std::string identity;
};

// Every recognised ROM in the archive, hashed once. Identity comes from the
// MD5 rather than the filename, so a dump named anything at all is usable --
// which matters because dumps circulate under several names and nobody
// downloading a release should have to rename one to match a constant.
//
// Scanned once: adding a ROM while Albert is running wants File > Open.
const std::vector<RomEntry>& rom_catalogue() {
    static const std::vector<RomEntry> list = [] {
        std::vector<RomEntry> found;
        std::vector<std::filesystem::path> candidates;
        std::error_code ec;

        for (const auto& entry : std::filesystem::directory_iterator(roms_dir(), ec)) {
            if (!entry.is_regular_file(ec)) continue;
            std::string ext = entry.path().extension().string();
            for (char& c : ext) c = static_cast<char>(std::tolower(c));
            if (ext == ".rom") candidates.push_back(entry.path());
        }
        // Directory order is not defined, and "the first one" has to mean the
        // same thing on every machine.
        std::sort(candidates.begin(), candidates.end());

        // The archive's own MOS 1.21 image is a .bin, so name it explicitly
        // rather than lose the ROM menu entry in a source checkout.
        const std::filesystem::path legacy = roms_dir() / "einstein1.21.bin";
        if (std::filesystem::exists(legacy, ec)) candidates.push_back(legacy);

        for (const std::filesystem::path& p : candidates) {
            try {
                const ein::RomImage rom = ein::load_rom(p);
                if (rom.recognised) found.push_back({p, rom.identity});
            } catch (const std::exception&) {
                // Not a ROM, or unreadable. Not an error: the folder is the
                // user's, and may hold anything.
            }
        }
        return found;
    }();
    return list;
}

}  // namespace

std::filesystem::path default_rom_path() {
    std::error_code ec;
    const std::filesystem::path named = roms_dir() / kDefaultRomName;
    if (std::filesystem::exists(named, ec)) return named;
    const std::vector<RomEntry>& all = rom_catalogue();
    if (!all.empty()) return all.front().path;
    // Nothing to load. Name the file that was expected, so the failure in the
    // ROM panel points at something the user can act on.
    return named;
}

std::filesystem::path find_rom(const std::string& identity) {
    for (const RomEntry& e : rom_catalogue()) {
        if (e.identity == identity) return e.path;
    }
    return {};
}

bool looks_like_disc(const std::filesystem::path& path) {
    std::string ext = path.extension().string();
    for (char& ch : ext) ch = static_cast<char>(std::tolower(ch));
    return ext == ".dsk" || ext == ".cpm" || ext == ".img";
}

// -------------------------------------------------------------------- Files

// SDL's dialogs are native on Windows and run on their own thread, so the
// frame loop keeps going while one is open and the callback can land on a
// thread that is not this one. It therefore parks the answer under a mutex and
// pump_dialog(), called once a frame, is what actually touches the machine.
const SDL_DialogFileFilter kMediaFilters[] = {
    {"Einstein media", "rom;bin;obj;dsk;cpm;img"},
    {"ROM images", "rom;bin;obj"},
    {"Disc images", "dsk;cpm;img"},
    {"All files", "*"},
};
const SDL_DialogFileFilter kDiscFilters[] = {
    {"Disc images", "dsk;cpm;img"},
    {"All files", "*"},
};
const SDL_DialogFileFilter kStateFilters[] = {
    {"Save states", "state"},
    {"All files", "*"},
};
const SDL_DialogFileFilter kDumpFilters[] = {
    {"Text and binary dumps", "txt;bin"},
    {"All files", "*"},
};

void note_recent(AppState& app, const std::string& path) {
    auto& r = app.recent;
    r.erase(std::remove(r.begin(), r.end(), path), r.end());
    r.insert(r.begin(), path);
    if (r.size() > 8) r.resize(8);
}

void open_path(AppState& app, const std::filesystem::path& path) {
    if (looks_like_disc(path)) {
        insert_disc(app, 0, path);
    } else {
        load(app, path);
    }
    app.running = true;
    note_recent(app, path.string());
}

template <typename Bytes>
void save_bytes(AppState& app, const std::filesystem::path& path, const Bytes& data) {
    std::ofstream f(path, std::ios::binary);
    const std::string name = path.filename().string();
    if (!f) {
        app.state_note = "could not write " + name;
        return;
    }
    for (const std::uint8_t b : data) f.put(static_cast<char>(b));
    app.state_note = "wrote " + name;
}

void SDLCALL dialog_done(void* userdata, const char* const* filelist, int filter) {
    (void)filter;
    AppState& app = *static_cast<AppState*>(userdata);
    SDL_LockMutex(app.pending.lock);
    if (filelist != nullptr && filelist[0] != nullptr) {
        app.pending.path = filelist[0];
        app.pending.ready = true;
    } else {
        // Cancelled, or the platform could not show it. Either way, nothing.
        app.pending.kind = kDlgNone;
    }
    app.pending.open = false;
    SDL_UnlockMutex(app.pending.lock);
}

void show_dialog(AppState& app, int kind) {
    if (app.pending.open) return;  // one at a time
    app.pending.kind = kind;
    app.pending.open = true;
    switch (kind) {
        case kDlgOpen:
            SDL_ShowOpenFileDialog(dialog_done, &app, app.window, kMediaFilters, 4,
                                   roms_dir().string().c_str(), false);
            break;
        case kDlgInsert0:
        case kDlgInsert1:
            SDL_ShowOpenFileDialog(dialog_done, &app, app.window, kDiscFilters, 2, nullptr, false);
            break;
        case kDlgLoadSnapshot:
            SDL_ShowOpenFileDialog(dialog_done, &app, app.window, kStateFilters, 2, nullptr,
                                   false);
            break;
        case kDlgSaveSnapshot:
            SDL_ShowSaveFileDialog(dialog_done, &app, app.window, kStateFilters, 2, kStateFile);
            break;
        case kDlgSavePrinter:
            SDL_ShowSaveFileDialog(dialog_done, &app, app.window, kDumpFilters, 2, "printer.txt");
            break;
        case kDlgSaveSerial:
            SDL_ShowSaveFileDialog(dialog_done, &app, app.window, kDumpFilters, 2, "serial.bin");
            break;
        default:
            app.pending.open = false;
            app.pending.kind = kDlgNone;
            break;
    }
}

void pump_dialog(AppState& app) {
    std::string path;
    int kind = kDlgNone;
    SDL_LockMutex(app.pending.lock);
    if (app.pending.ready) {
        path = app.pending.path;
        kind = app.pending.kind;
        app.pending.ready = false;
        app.pending.kind = kDlgNone;
    }
    SDL_UnlockMutex(app.pending.lock);
    if (kind == kDlgNone || path.empty()) return;

    switch (kind) {
        case kDlgOpen:
            open_path(app, path);
            break;
        case kDlgInsert0:
        case kDlgInsert1:
            insert_disc(app, kind == kDlgInsert0 ? 0 : 1, path);
            note_recent(app, path);
            break;
        case kDlgSaveSnapshot:
            write_state(app, path);
            break;
        case kDlgLoadSnapshot:
            read_state(app, path);
            break;
        case kDlgSavePrinter:
            save_bytes(app, path, app.machine.printer);
            break;
        case kDlgSaveSerial:
            save_bytes(app, path, app.machine.uart.sent());
            break;
        default:
            break;
    }
}


// ------------------------------------------------------------------- Config

constexpr const char* kConfigFile = "albert.cfg";

void save_config(const AppState& app) {
    std::ofstream f(kConfigFile);
    if (!f) return;
    f << "zoom=" << app.screen_zoom << "\n";
    f << "integer_scale=" << (app.integer_scale ? 1 : 0) << "\n";
    f << "sound=" << (app.sound_on ? 1 : 0) << "\n";
    f << "lowpass=" << static_cast<int>(app.machine.psg.lowpass_hz() + 0.5f) << "\n";
    f << "speed=" << app.speed << "\n";
    f << "emulator_mode=" << (app.emulator_mode ? 1 : 0) << "\n";
    f << "card80=" << (app.machine.card80.present ? 1 : 0) << "\n";
    for (const std::string& r : app.recent) f << "recent=" << r << "\n";
    for (int j = 0; j < kJoysticks; ++j) {
        const Joystick& s = app.joy[j];
        f << "joy" << j << ".enabled=" << (s.enabled ? 1 : 0) << "\n";
        for (int i = 0; i < kJoyInputCount; ++i) {
            f << "joy" << j << ".key" << i << "=" << static_cast<int>(s.bind[i].key) << "\n";
        }
        f << "joy" << j << ".fire1=" << s.bind[kJoyFire1].row << "," << s.bind[kJoyFire1].col
          << "\n";
        f << "joy" << j << ".fire2=" << s.bind[kJoyFire2].row << "," << s.bind[kJoyFire2].col
          << "\n";
    }
}

void load_config(AppState& app) {
    std::ifstream f(kConfigFile);
    if (!f) return;
    std::string line;
    bool first = true;
    while (std::getline(f, line)) {
        // Notepad writes a UTF-8 BOM. Compared byte by byte rather than
        // against a literal, so nothing depends on this file's encoding;
        // left alone it would glue itself to the first key and silently
        // lose that one setting.
        if (first) {
            first = false;
            if (line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF) {
                line.erase(0, 3);
            }
        }
        if (!line.empty() && line.back() == 0x0D) line.pop_back();
        const std::size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = line.substr(0, eq);
        const std::string val = line.substr(eq + 1);
        if (key == "zoom") app.screen_zoom = std::clamp(std::atoi(val.c_str()), 1, 4);
        else if (key == "integer_scale") app.integer_scale = (val == "1");
        else if (key == "sound") app.sound_on = (val == "1");
        // 0 is off, and so is anything below the bottom of the panel's slider
        // -- a 50 Hz cutoff is not a setting anyone means.
        else if (key == "lowpass") {
            const int hz = std::clamp(std::atoi(val.c_str()), 0, 24000);
            app.machine.psg.set_lowpass(hz < 250 ? 0.0f : static_cast<float>(hz));
        }
        else if (key == "speed") app.speed = std::clamp(std::atoi(val.c_str()), 0, 2);
        else if (key == "emulator_mode") app.emulator_mode = (val == "1");
        else if (key == "card80") app.machine.card80.present = (val == "1");
        else if (key == "recent" && !val.empty() && app.recent.size() < 8) app.recent.push_back(val);
        else if (key.rfind("joy", 0) == 0 && key.size() > 4 && key[4] == '.') {
            const int j = key[3] - '0';
            if (j < 0 || j >= kJoysticks) continue;
            Joystick& s = app.joy[j];
            const std::string field = key.substr(5);
            if (field == "enabled") {
                s.enabled = (val == "1");
            } else if (field.rfind("key", 0) == 0) {
                const int i = std::atoi(field.c_str() + 3);
                if (i >= 0 && i < kJoyInputCount) {
                    s.bind[i].key = static_cast<SDL_Scancode>(std::atoi(val.c_str()));
                }
            } else if (field == "fire1" || field == "fire2") {
                const std::size_t comma = val.find(',');
                if (comma != std::string::npos) {
                    const int i = (field == "fire1") ? kJoyFire1 : kJoyFire2;
                    s.bind[i].row = std::atoi(val.substr(0, comma).c_str());
                    s.bind[i].col = std::atoi(val.substr(comma + 1).c_str());
                }
            }
        }
    }
}

}  // namespace albert
