// MD5 (RFC 1321). Used only to verify ROM image identity, never for security.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace ein {

using Md5Digest = std::array<std::uint8_t, 16>;

Md5Digest md5(const void* data, std::size_t len);
std::string md5_hex(const void* data, std::size_t len);
std::string to_hex(const Md5Digest& d);

}  // namespace ein
