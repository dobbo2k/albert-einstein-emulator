#include "md5.h"

#include <cstring>

namespace ein {
namespace {

// K[i] = floor(abs(sin(i + 1)) * 2^32)
constexpr std::uint32_t kK[64] = {
    0xd76aa478u, 0xe8c7b756u, 0x242070dbu, 0xc1bdceeeu,
    0xf57c0fafu, 0x4787c62au, 0xa8304613u, 0xfd469501u,
    0x698098d8u, 0x8b44f7afu, 0xffff5bb1u, 0x895cd7beu,
    0x6b901122u, 0xfd987193u, 0xa679438eu, 0x49b40821u,
    0xf61e2562u, 0xc040b340u, 0x265e5a51u, 0xe9b6c7aau,
    0xd62f105du, 0x02441453u, 0xd8a1e681u, 0xe7d3fbc8u,
    0x21e1cde6u, 0xc33707d6u, 0xf4d50d87u, 0x455a14edu,
    0xa9e3e905u, 0xfcefa3f8u, 0x676f02d9u, 0x8d2a4c8au,
    0xfffa3942u, 0x8771f681u, 0x6d9d6122u, 0xfde5380cu,
    0xa4beea44u, 0x4bdecfa9u, 0xf6bb4b60u, 0xbebfbc70u,
    0x289b7ec6u, 0xeaa127fau, 0xd4ef3085u, 0x04881d05u,
    0xd9d4d039u, 0xe6db99e5u, 0x1fa27cf8u, 0xc4ac5665u,
    0xf4292244u, 0x432aff97u, 0xab9423a7u, 0xfc93a039u,
    0x655b59c3u, 0x8f0ccc92u, 0xffeff47du, 0x85845dd1u,
    0x6fa87e4fu, 0xfe2ce6e0u, 0xa3014314u, 0x4e0811a1u,
    0xf7537e82u, 0xbd3af235u, 0x2ad7d2bbu, 0xeb86d391u,
};

constexpr int kS[64] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
};

inline std::uint32_t rotl(std::uint32_t x, int c) {
    return (x << c) | (x >> (32 - c));
}

void transform(std::uint32_t state[4], const std::uint8_t block[64]) {
    std::uint32_t m[16];
    for (int i = 0; i < 16; ++i) {
        m[i] = static_cast<std::uint32_t>(block[i * 4]) |
               (static_cast<std::uint32_t>(block[i * 4 + 1]) << 8) |
               (static_cast<std::uint32_t>(block[i * 4 + 2]) << 16) |
               (static_cast<std::uint32_t>(block[i * 4 + 3]) << 24);
    }

    std::uint32_t a = state[0], b = state[1], c = state[2], d = state[3];

    for (int i = 0; i < 64; ++i) {
        std::uint32_t f;
        int g;
        if (i < 16) {
            f = (b & c) | (~b & d);
            g = i;
        } else if (i < 32) {
            f = (d & b) | (~d & c);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            f = b ^ c ^ d;
            g = (3 * i + 5) % 16;
        } else {
            f = c ^ (b | ~d);
            g = (7 * i) % 16;
        }
        const std::uint32_t tmp = d;
        d = c;
        c = b;
        b = b + rotl(a + f + kK[i] + m[g], kS[i]);
        a = tmp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

}  // namespace

Md5Digest md5(const void* data, std::size_t len) {
    std::uint32_t state[4] = {0x67452301u, 0xefcdab89u, 0x98badcfeu, 0x10325476u};
    const auto* p = static_cast<const std::uint8_t*>(data);

    const std::size_t whole = len / 64;
    for (std::size_t i = 0; i < whole; ++i) transform(state, p + i * 64);

    // Final block(s): 0x80 terminator, zero pad, then the bit length little-endian.
    std::uint8_t tail[128] = {};
    const std::size_t rem = len - whole * 64;
    std::memcpy(tail, p + whole * 64, rem);
    tail[rem] = 0x80;
    const std::size_t tail_len = (rem < 56) ? 64 : 128;

    const std::uint64_t bits = static_cast<std::uint64_t>(len) * 8;
    for (int i = 0; i < 8; ++i) {
        tail[tail_len - 8 + i] = static_cast<std::uint8_t>((bits >> (8 * i)) & 0xff);
    }

    transform(state, tail);
    if (tail_len == 128) transform(state, tail + 64);

    Md5Digest out{};
    for (int i = 0; i < 4; ++i) {
        out[i * 4] = static_cast<std::uint8_t>(state[i] & 0xff);
        out[i * 4 + 1] = static_cast<std::uint8_t>((state[i] >> 8) & 0xff);
        out[i * 4 + 2] = static_cast<std::uint8_t>((state[i] >> 16) & 0xff);
        out[i * 4 + 3] = static_cast<std::uint8_t>((state[i] >> 24) & 0xff);
    }
    return out;
}

std::string to_hex(const Md5Digest& d) {
    static constexpr char kHex[] = "0123456789abcdef";
    std::string s;
    s.reserve(32);
    for (std::uint8_t b : d) {
        s.push_back(kHex[b >> 4]);
        s.push_back(kHex[b & 0x0f]);
    }
    return s;
}

std::string md5_hex(const void* data, std::size_t len) {
    return to_hex(md5(data, len));
}

}  // namespace ein
