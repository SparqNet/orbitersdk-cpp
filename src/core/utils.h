#ifndef UTILS_H
#define UTILS_H

#include <fstream>

#include <boost/lexical_cast.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include "../libs/devcore/CommonData.h"
#include "../libs/devcore/FixedHash.h"
#include "../libs/json.hpp"
#include "../libs/keccak.hpp"

using json = nlohmann::ordered_json;
using uint256_t = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<256, 256, boost::multiprecision::unsigned_magnitude, boost::multiprecision::cpp_int_check_type::unchecked, void>>;
using uint160_t = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<160, 160, boost::multiprecision::unsigned_magnitude, boost::multiprecision::cpp_int_check_type::unchecked, void>>;
static const uint256_t c_secp256k1n("115792089237316195423570985008687907852837564279074904382605163141518161494337");

template <typename ElemT>
struct HexTo {
  ElemT value;
  operator ElemT() const { return value; }
  friend std::istream& operator>>(std::istream& in, HexTo& out) {
    in >> std::hex >> out.value;
    return in;
  }
};

namespace Log {
  const std::string subnet = "Subnet::";
  const std::string chainHead = "ChainHead::";
  const std::string chainTip = "ChainTip::";
  const std::string block = "Block::";
  const std::string db = "DBService::";
  const std::string state = "State::";
  const std::string grpcServer = "VMServiceImplementation::";
  const std::string grpcClient = "VMCommClient::";
  const std::string utils = "Utils::";
  const std::string httpServer = "HTTPServer::";
};

namespace MessagePrefix {
  const char tx = 0x01;
  const char batchedTx = 0x02;
}

enum BlockStatus { Unknown, Processing, Rejected, Accepted };

namespace Utils {
  void logToFile(std::string str);
  void LogPrint(const std::string &prefix, std::string function, std::string data);
  void sha3(const std::string &input, std::string &output);
  std::string uint256ToBytes(const uint256_t &i);
  std::string uint160ToBytes(const uint160_t &i);
  std::string uint64ToBytes(const uint64_t &i);
  std::string uint32ToBytes(const uint32_t &i);
  std::string uint16ToBytes(const uint16_t &i);
  std::string uint8ToBytes(const uint8_t &i);
  uint256_t bytesToUint256(const std::string &bytes);
  uint160_t bytesToUint160(const std::string &bytes);
  uint64_t bytesToUint64(const std::string &bytes);
  uint32_t bytesToUint32(const std::string &bytes);
  uint16_t bytesToUint16(const std::string &bytes);
  uint8_t bytesToUint8(const std::string &bytes);
  int fromHexChar(char c) noexcept;
  void patchHex(std::string& str);
  template <typename T> std::string uintToHex(const T &i) {
    std::stringstream ss;
    std::string ret;
    ss << std::hex << i;
    ret = ss.str();
    for (auto &c : ret) if (std::isupper(c)) c = std::tolower(c);
    return ret;
  }
  uint256_t hexToUint(std::string &hex);
  std::string hexToBytes(std::string hex);
  std::string bytesToHex(const std::string& bytes);
  bool verifySignature(uint8_t const &v, uint256_t const &r, uint256_t const &s);
  std::string padLeft(std::string str, unsigned int charAmount, char sign = '0');
  std::string padRight(std::string str, unsigned int charAmount, char sign = '0');
} // Utils

struct Account {
  uint256_t balance = 0;
  uint32_t nonce = 0;
};

class Address {
  private:
    std::string innerAddress;

  public:
    Address() {}
    Address(std::string address, bool fromRPC = true) {
      if (fromRPC) {
        Utils::patchHex(address);
        innerAddress = Utils::hexToBytes(address);
      } else {
        innerAddress = address;
      }
    }

    // Copy constructor.
    Address(const Address& other) {
      this->innerAddress = other.innerAddress;
    }

    // Move constructor.
    Address(Address&& other) noexcept :
      innerAddress(std::move(other.innerAddress)) {}

    // Destructor.
    ~Address() { this->innerAddress = ""; }

    const std::string& get() const { return innerAddress; };
    const std::string hex() const { return Utils::bytesToHex(innerAddress); }
    dev::h160 toHash() const {
      return dev::h160(innerAddress, dev::FixedHash<20>::ConstructFromStringType::FromBinary);
    }
    void operator=(const std::string& address) { this->innerAddress = address; }
    void operator=(const Address& address) { this->innerAddress = address.innerAddress; }
    void operator=(const dev::h160 &address) { this->innerAddress = address.byteStr(); }
    void operator=(const uint160_t &address) { this->innerAddress = Utils::uint160ToBytes(address); }
    Address& operator=(Address&& other) {
      this->innerAddress = std::move(other.innerAddress);
      return *this;
    }
    bool operator==(const Address& rAddress) const { return bool(innerAddress == rAddress.innerAddress); }
    bool operator!=(const Address& rAddress) const { return bool(innerAddress != rAddress.innerAddress); }
};

template <>
struct std::hash<Address> {
  size_t operator() (const Address& address) const {
    return std::hash<std::string>()(address.get());
  }
};

#endif // UTILS_H