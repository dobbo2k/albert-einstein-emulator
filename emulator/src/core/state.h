#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

namespace ein {

// Minimal serialisation for save states. Everything written is a fixed-size
// trivially copyable value, so the format is just the fields in order.
//
// What a save state deliberately does not contain: the discs in the drives
// (the media is not machine state -- a state saved with a disc in drive 0 is
// restored into whatever is in drive 0 now), the audio ring buffer (it
// regenerates within milliseconds), and serial bytes still on the wire.
class StateWriter {
public:
    explicit StateWriter(std::vector<std::uint8_t>& out) : out_(out) {}

    template <typename T>
    void put(const T& v) {
        static_assert(std::is_trivially_copyable_v<T>, "save state fields must be POD");
        const auto* p = reinterpret_cast<const std::uint8_t*>(&v);
        out_.insert(out_.end(), p, p + sizeof(T));
    }

    void bytes(const void* p, std::size_t n) {
        const auto* b = static_cast<const std::uint8_t*>(p);
        out_.insert(out_.end(), b, b + n);
    }

private:
    std::vector<std::uint8_t>& out_;
};

class StateReader {
public:
    StateReader(const std::uint8_t* p, const std::uint8_t* end) : p_(p), end_(end) {}

    template <typename T>
    bool get(T& v) {
        static_assert(std::is_trivially_copyable_v<T>, "save state fields must be POD");
        if (static_cast<std::size_t>(end_ - p_) < sizeof(T)) {
            ok_ = false;
            return false;
        }
        std::memcpy(&v, p_, sizeof(T));
        p_ += sizeof(T);
        return true;
    }

    bool bytes(void* p, std::size_t n) {
        if (static_cast<std::size_t>(end_ - p_) < n) {
            ok_ = false;
            return false;
        }
        std::memcpy(p, p_, n);
        p_ += n;
        return true;
    }

    bool ok() const { return ok_; }

private:
    const std::uint8_t* p_;
    const std::uint8_t* end_;
    bool ok_ = true;
};

}  // namespace ein
