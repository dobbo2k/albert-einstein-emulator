#include "dskimage.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace ein {
namespace {

const std::vector<DiskImage::Sector>& empty_track() {
    static const std::vector<DiskImage::Sector> kEmpty;
    return kEmpty;
}

bool starts_with(const std::vector<std::uint8_t>& d, const char* s) {
    const std::size_t n = std::strlen(s);
    return d.size() >= n && std::memcmp(d.data(), s, n) == 0;
}

}  // namespace

const std::vector<DiskImage::Sector>& DiskImage::track(int t, int side) const {
    if (t < 0 || t >= tracks_ || side < 0 || side >= sides_) return empty_track();
    const std::size_t i = static_cast<std::size_t>(t) * sides_ + side;
    return i < layout_.size() ? layout_[i] : empty_track();
}

const DiskImage::Sector* DiskImage::find(int t, int side, int id) const {
    for (const Sector& s : track(t, side)) {
        if (s.id == id) return &s;
    }
    return nullptr;
}

int DiskImage::slot_of(int t, int side, int id) const {
    const std::vector<Sector>& v = track(t, side);
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (v[i].id == id) return static_cast<int>(i);
    }
    return -1;
}

const std::uint8_t* DiskImage::sector(int t, int side, int id) const {
    const Sector* s = find(t, side, id);
    return s ? bytes(*s) : nullptr;
}

std::uint8_t* DiskImage::sector(int t, int side, int id) {
    const Sector* s = find(t, side, id);
    return s ? data_.data() + s->offset : nullptr;
}

void DiskImage::eject() {
    data_.clear();
    layout_.clear();
    name_.clear();
    format_ = "none";
}

// A plain image has no ID fields of its own, so they are synthesised from the
// Einstein convention: ids 0-9 on side 0, 10-19 on side 1, in id order.
void DiskImage::build_plain_layout() {
    layout_.assign(static_cast<std::size_t>(tracks_) * sides_, {});
    for (int t = 0; t < tracks_; ++t) {
        for (int side = 0; side < sides_; ++side) {
            std::vector<Sector>& v = layout_[static_cast<std::size_t>(t) * sides_ + side];
            v.resize(kSectorsPerTrack);
            for (int i = 0; i < kSectorsPerTrack; ++i) {
                v[i].id = static_cast<std::uint8_t>(side * kSectorsPerTrack + i);
                v[i].offset = static_cast<std::uint32_t>(
                    ((t * sides_) + side) * kTrackBytes + i * kSectorSize);
                v[i].size = kSectorSize;
            }
        }
    }
}

void DiskImage::format(int sides, std::string name) {
    sides_ = (sides == 2) ? 2 : 1;
    tracks_ = kTracks;
    data_.assign(sides_ == 2 ? kDoubleSidedBytes : kSingleSidedBytes, kFormatFill);
    build_plain_layout();
    name_ = std::move(name);
    format_ = "plain";
    write_protected = false;
}

// CPC DSK and Extended CPC DSK.
//
// 256-byte disc information block, then one 256-byte track information block
// per track per side followed by that track's sector data. In the standard
// format every track is the same size, given at offset 32-33; in the extended
// format each has its own entry in the table at offset 34, counted in 256-byte
// units. Sector data lengths come from the sector info list in the extended
// format and from the size code alone in the standard one.
void DiskImage::load_dsk(const std::vector<std::uint8_t>& raw, const std::string& what) {
    const bool extended = starts_with(raw, "EXTENDED CPC DSK File");
    if (raw.size() < 256) throw std::runtime_error(what + ": DSK header is truncated");

    tracks_ = raw[0x30];
    sides_ = raw[0x31];
    if (tracks_ <= 0 || tracks_ > 84 || sides_ < 1 || sides_ > 2) {
        throw std::runtime_error(what + ": DSK claims an implausible geometry");
    }

    const int entries = tracks_ * sides_;
    std::vector<std::size_t> track_len(entries, 0);
    if (extended) {
        if (raw.size() < std::size_t(0x34) + entries) {
            throw std::runtime_error(what + ": extended DSK track table is truncated");
        }
        for (int i = 0; i < entries; ++i) track_len[i] = std::size_t(raw[0x34 + i]) * 256;
    } else {
        const std::size_t fixed = std::size_t(raw[0x32]) | (std::size_t(raw[0x33]) << 8);
        std::fill(track_len.begin(), track_len.end(), fixed);
    }

    data_.clear();
    layout_.assign(entries, {});

    std::size_t pos = 256;
    for (int i = 0; i < entries; ++i) {
        const std::size_t len = track_len[i];
        if (len == 0) continue;  // unformatted track
        if (pos + len > raw.size()) {
            throw std::runtime_error(what + ": DSK ends inside a track");
        }
        const std::uint8_t* ti = raw.data() + pos;
        if (std::memcmp(ti, "Track-Info", 10) != 0) {
            throw std::runtime_error(what + ": missing Track-Info block");
        }
        const int t = ti[0x10];
        const int side = ti[0x11];
        const int count = ti[0x15];
        const int size_code = ti[0x14];
        if (t >= tracks_ || side >= sides_ || count < 0 || count > 29) {
            pos += len;
            continue;
        }

        std::vector<Sector>& v = layout_[static_cast<std::size_t>(t) * sides_ + side];
        v.clear();
        std::size_t data_at = pos + 256;
        for (int s = 0; s < count; ++s) {
            const std::uint8_t* si = ti + 0x18 + s * 8;
            std::size_t slen = extended ? (std::size_t(si[6]) | (std::size_t(si[7]) << 8))
                                        : (std::size_t(128) << size_code);
            if (slen == 0) slen = std::size_t(128) << si[3];
            if (data_at + slen > pos + len) break;  // malformed; keep what we have

            Sector sec;
            sec.id = si[2];
            sec.size = static_cast<std::uint16_t>(std::min<std::size_t>(slen, 0xffff));
            sec.st1 = si[4];
            sec.st2 = si[5];
            sec.offset = static_cast<std::uint32_t>(data_.size());
            data_.insert(data_.end(), raw.begin() + data_at, raw.begin() + data_at + slen);
            v.push_back(sec);
            data_at += slen;
        }
        pos += len;
    }

    if (data_.empty()) throw std::runtime_error(what + ": DSK contains no sector data");
    format_ = extended ? "Extended CPC DSK" : "CPC DSK";
}

void DiskImage::load(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open disk image: " + path.string());
    std::vector<std::uint8_t> raw((std::istreambuf_iterator<char>(f)),
                                  std::istreambuf_iterator<char>());
    const std::string what = path.filename().string();

    if (starts_with(raw, "EXTENDED CPC DSK File") || starts_with(raw, "MV - CPC")) {
        load_dsk(raw, what);
    } else if (raw.size() == kSingleSidedBytes || raw.size() == kDoubleSidedBytes) {
        sides_ = (raw.size() == kDoubleSidedBytes) ? 2 : 1;
        tracks_ = kTracks;
        data_ = std::move(raw);
        build_plain_layout();
        format_ = "plain";
    } else if (raw.size() == kSystemTracksBytes) {
        // Just the system tracks. Pad to a blank single-sided disc so the
        // result is bootable: FBOOT only ever reads track 0 onwards, and the
        // data area past track 1 stays empty.
        format(1, what);
        std::copy(raw.begin(), raw.end(), data_.begin());
    } else {
        char msg[320];
        std::snprintf(msg, sizeof msg,
                      "%s is %zu bytes and does not start with a DSK signature; expected "
                      "%zu (single sided), %zu (double sided) or %zu (SYSTEM*.CPM system tracks)",
                      what.c_str(), raw.size(), kSingleSidedBytes, kDoubleSidedBytes,
                      kSystemTracksBytes);
        throw std::runtime_error(msg);
    }
    name_ = what;
    write_protected = false;
}

}  // namespace ein
