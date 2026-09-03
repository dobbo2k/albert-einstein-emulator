#include "rom.h"

#include <fstream>
#include <stdexcept>

#include "md5.h"

namespace ein {

RomImage load_rom(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open ROM: " + path.string());

    std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(f),
                                    std::istreambuf_iterator<char>()};
    if (f.bad()) throw std::runtime_error("error reading ROM: " + path.string());

    RomImage rom;
    rom.file_size = bytes.size();
    if (bytes.size() < kRomSize) {
        throw std::runtime_error("ROM too small: " + path.string() + " (" +
                                 std::to_string(bytes.size()) + " bytes, need " +
                                 std::to_string(kRomSize) + ")");
    }
    rom.truncated = bytes.size() > kRomSize;
    bytes.resize(kRomSize);
    rom.data = std::move(bytes);

    rom.md5 = md5_hex(rom.data.data(), rom.data.size());
    if (rom.md5 == kMos12Md5) {
        rom.identity = "MOS 1.2";
        rom.recognised = true;
    } else if (rom.md5 == kMos121Md5) {
        rom.identity = "MOS 1.21";
        rom.recognised = true;
    } else {
        rom.identity = "unrecognised";
    }
    return rom;
}

}  // namespace ein
