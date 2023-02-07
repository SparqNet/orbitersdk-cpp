#include "hex.h"

Hex::Hex(bool strict) : strict(strict) {
  if (strict) { this->hex = "0x"; }
}

Hex::Hex(const std::string_view& v, bool strict) : strict(strict) {
  std::string ret(v);
  if (strict) {
    if (ret[0] != '0' && (ret[1] != 'x' || ret[1] != 'X')) ret.insert(0, "0x");
  } else {
    if (ret[0] == '0' && (ret[1] == 'x' || ret[1] == 'X')) ret.erase(0, 2);
  }
  if (!this->isHexValid(ret)) throw std::runtime_error("Invalid Hex string at constructor");
  this->hex = std::move(ret);
}

Hex::Hex(std::string&& v, bool strict) : hex(std::move(v)), strict(strict) {
  if (strict) {
    if (this->hex[0] != '0' && (this->hex[1] != 'x' || this->hex[1] != 'X')) {
      this->hex.insert(0, "0x");
    }
  } else {
    if (this->hex[0] == '0' && (this->hex[1] == 'x' || this->hex[1] == 'X')) {
      this->hex.erase(0, 2);
    }
  }
  if (!this->isHexValid()) throw std::runtime_error("Invalid Hex string at constructor");
}

bool Hex::isHexValid(const std::string_view& v) const {
  std::string hex = (v.empty()) ? this->hex : v.data();
  if (strict && hex.substr(0, 2) != "0x" && hex.substr(0, 2) != "0X") return false;
  if (hex.find_first_not_of(filter) != std::string::npos) return false;
  return true;
}

Hex Hex::fromBytes(std::string_view bytes, bool strict) {
  auto beg = bytes.begin();
  auto end = bytes.end();
  static const char* digits = "0123456789abcdef";
  size_t off = (strict) ? 2 : 0;
  std::string hex(std::distance(beg, end) * 2 + off, '0');
  hex.replace(0, 2, "0x");
  for (; beg != end; beg++) {
    hex[off++] = digits[(*beg >> 4) & 0x0f];
    hex[off++] = digits[*beg & 0x0f];
  }
  return Hex(std::move(hex), strict);
}

Hex Hex::fromUTF8(std::string_view str, bool strict) {
  std::stringstream ss;
  if (strict) ss << "0x";
  for (int i = 0; i < str.length(); i++) {
    // We need two casts in order to properly cast char to uint
    ss << std::hex << std::setfill('0') << std::setw(2)
      << static_cast<uint>(static_cast<uint8_t>(str[i]));
  }
  return Hex(std::move(ss.str()), strict);
}

// TODO: This function is identical in CommonData.h, is for the better a re-write of commonly used functions at CommonData.h
int Hex::hexCharToInt(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')	return c - 'A' + 10;
  return -1;
}

std::string Hex::bytes() const {
  // Parse two by two chars until the end
  std::string ret;
  uint32_t i = (this->hex.size() % 2 != 0); // Odd offset ("0xaaa")
  i += (this->strict) ? 2 : 0; // Strict offset ("0x")
  for (; i < this->hex.size(); i += 2) {
    int h = Hex::hexCharToInt(this->hex[i]);
    int l = Hex::hexCharToInt(this->hex[i + 1]);
    if (h != -1 && l != -1) {
      ret += (char) uint8_t(h * 16 + l);
    } else {
      throw std::runtime_error(std::string(__func__)
        + ": One or more invalid hex chars: " + this->hex[i] + this->hex[i + 1]
      );
    }
  }
  return ret;
}