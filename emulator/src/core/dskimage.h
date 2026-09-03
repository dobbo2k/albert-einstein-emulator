#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ein {

// A floppy disc: the bytes, plus what is written in each track's ID fields and
// the order the sectors physically appear in.
//
// The Einstein geometry is read out of the MOS driver rather than guessed:
//
//   * **512-byte sectors.** ZRSECT transfers `SSIZE * 256` bytes and `SCTSIZE`
//     is preset to 2 (`UPPERMEM.ZSM:70`); ZRBLK advances `HSTDMA` by 512
//     between sectors (`Mos12.zsm:971`).
//   * **10 sectors per track per side.** GNSECT (`Mos12.zsm:4091`) returns 10,
//     doubled to 20 only for a drive whose bit is set in `DTYPE`.
//   * **40 tracks.** ZRBLK gives up once the track reaches 40
//     (`Mos12.zsm:962`).
//   * **Sectors are numbered from 0**, and on a double-sided disc side 1
//     carries ids 10-19: CALDSC (`Mos12.zsm:4079`) sets the port 23 side bit
//     when `HSTSEC >= 10` and still writes the raw number to the FDC's sector
//     register, so the id itself says which side it is on.
//
// Every one of those is confirmed by a real commercial disc: the Oh Mummy
// image is 40 tracks, one side, ten 512-byte sectors with ids 0-9.
//
// Two container formats are read:
//
//   * **Plain sector images** -- 204800 or 409600 bytes, sectors in id order,
//     sides interleaved per track. Also the 10240-byte SYSTEM*.CPM system
//     tracks from this archive, padded out to a blank bootable disc.
//   * **CPC DSK and Extended CPC DSK**, the format emulators actually trade
//     Einstein discs in. This one carries the real ID fields, so it also
//     carries the physical sector order -- Oh Mummy is laid out
//     0,5,1,6,2,7,3,8,4,9, a 2:1 interleave -- which the controller's
//     rotational timing then reproduces.
class DiskImage {
public:
    static constexpr int kSectorSize = 512;
    static constexpr int kSectorsPerTrack = 10;
    static constexpr int kTracks = 40;
    static constexpr int kTrackBytes = kSectorsPerTrack * kSectorSize;

    static constexpr std::size_t kSingleSidedBytes = std::size_t(kTracks) * kTrackBytes;
    static constexpr std::size_t kDoubleSidedBytes = kSingleSidedBytes * 2;
    static constexpr std::size_t kSystemTracksBytes = 2 * kTrackBytes;

    // A freshly formatted CP/M data area is filled with E5 -- which is also the
    // filler byte the Oh Mummy disc records in its track headers.
    static constexpr std::uint8_t kFormatFill = 0xe5;

    // One sector as its ID field describes it.
    struct Sector {
        std::uint8_t id = 0;       // R, what MOS writes to the FDC sector register
        std::uint32_t offset = 0;  // into data()
        std::uint16_t size = kSectorSize;
        std::uint8_t st1 = 0;      // FDC status 1 recorded in the image
        std::uint8_t st2 = 0;      // FDC status 2 (deleted data, CRC errors)
    };

    bool loaded() const { return !data_.empty(); }
    int sides() const { return sides_; }
    int tracks() const { return tracks_; }
    const std::string& name() const { return name_; }
    const std::string& format_name() const { return format_; }
    bool write_protected = false;

    // Sectors of one track in the order they physically pass the head.
    const std::vector<Sector>& track(int track, int side) const;

    // Finds a sector by the id in its address mark. Null if this track has none.
    const Sector* find(int track, int side, int id) const;

    // Where that sector sits in the rotation, or -1 if it is not there. This is
    // the physical position, so an interleaved disc reads at interleaved speed.
    int slot_of(int track, int side, int id) const;

    std::uint8_t* bytes(const Sector& s) { return data_.data() + s.offset; }
    const std::uint8_t* bytes(const Sector& s) const { return data_.data() + s.offset; }

    // Convenience for tests and tools: the whole sector, or null.
    const std::uint8_t* sector(int track, int side, int id) const;
    std::uint8_t* sector(int track, int side, int id);

    // A blank formatted disc: E5 throughout, so CP/M sees an empty directory.
    void format(int sides, std::string name = "blank");

    // Plain image, SYSTEM*.CPM system tracks, or a CPC/Extended DSK.
    // Throws std::runtime_error if the file is none of those.
    void load(const std::filesystem::path& path);

    void eject();

    const std::vector<std::uint8_t>& data() const { return data_; }

private:
    std::vector<std::uint8_t> data_;
    // tracks_layout_[track * sides_ + side], in physical order.
    std::vector<std::vector<Sector>> layout_;
    int sides_ = 1;
    int tracks_ = kTracks;
    std::string name_;
    std::string format_ = "none";

    void build_plain_layout();
    void load_dsk(const std::vector<std::uint8_t>& raw, const std::string& what);
};

}  // namespace ein
