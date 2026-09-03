#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ein {

inline constexpr std::size_t kRomSize = 8192;

// Verified against this repository's archive; see CLAUDE.md.
inline constexpr const char* kMos12Md5 = "a10c89a81d5bda7b96f5e948245d45ce";
inline constexpr const char* kMos121Md5 = "7e4bc612466683027da521edfb36cac5";

struct RomImage {
    std::vector<std::uint8_t> data;  // exactly kRomSize bytes
    std::string md5;
    std::string identity;            // "MOS 1.2", "MOS 1.21", or "unrecognised"
    bool recognised = false;
    std::size_t file_size = 0;       // size on disk, before any truncation
    bool truncated = false;          // true when trailing bytes were dropped
};

// Loads an 8K Einstein ROM. roms/rom.bin is 8193 bytes -- an 8K image plus one
// stray trailing 0x21 -- so any file longer than kRomSize is truncated and
// flagged rather than rejected. Throws std::runtime_error if the file cannot be
// read or is shorter than kRomSize.
RomImage load_rom(const std::filesystem::path& path);

}  // namespace ein
