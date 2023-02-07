#include "../new_src/libs/catch2/catch_amalgamated.hpp"
#include "../new_src/utils/utils.h"
#include "../new_src/utils/strings.h"
#include "../new_src/utils/hex.h"

using Catch::Matchers::Equals;

namespace TUtils {

  TEST_CASE("Utils Namespace") {
    SECTION("Sha3 Test") {
      std::string sha3Input = "My SHA3 Input";
      auto sha3Output = Utils::sha3(sha3Input);
      std::string sha3ExpectedOutput = "\x10\x11\x40\xd6\xe7\x50\x6f\x80\x4c\xf7\xb0\x37\x0f\xa9\x0b\x04\xc5\xe9\x37\x4d\xdb\x0c\x8c\xbe\x12\xaf\x15\x0c\x8f\xf3\xee\x36";
      REQUIRE_THAT(sha3Output.get(), Equals(sha3ExpectedOutput));
    }

    SECTION("uint256ToBytes Test") {
      uint256_t uint256Input = uint256_t("91830918212381802449294565349763096207758814059154440393436864477986483867239");
      auto uint256Output = Utils::uint256ToBytes(uint256Input);
      std::string uint256ExpectedOutput = "\xcb\x06\x75\x32\x90\xff\xac\x16\x72\x05\xd0\xf5\x3b\x64\xac\xfd\x80\xbe\x11\xed\xbb\x26\xa2\x24\xbe\xd9\x23\x9a\xe6\x74\x0e\x67";
      REQUIRE_THAT(uint256Output, Equals(uint256ExpectedOutput));
    }

    SECTION("uint160ToBytes Test") {
      uint160_t uint160Input = uint160_t("506797479317435130489084083375319966488594602593");
      auto uint160Output = Utils::uint160ToBytes(uint160Input);
      std::string uint160ExpectedOutput = "\x58\xc5\x95\xbe\xdf\x1d\xea\x53\x2c\xf0\x6a\xf9\x09\x1a\x51\xb7\x5a\x11\xda\x61";
      REQUIRE_THAT(uint160Output, Equals(uint160ExpectedOutput));
    }

    SECTION("uint64ToBytes Test") {
      uint64_t uint64Input = uint64_t(11155010102558518614);
      auto uint64Output = Utils::uint64ToBytes(uint64Input);
      std::string uint64ExpectedOutput = "\x9a\xce\x8e\x96\x24\xe4\xed\x56";
      REQUIRE_THAT(uint64Output, Equals(uint64ExpectedOutput));
    }

    SECTION("uint32ToBytes Test") {
      uint32_t uint32Input = 2004601498;
      auto uint32Output = Utils::uint32ToBytes(uint32Input);
      std::string uint32ExpectedOutput = "\x77\x7b\xca\x9a";
      REQUIRE_THAT(uint32Output, Equals(uint32ExpectedOutput));
    }

    SECTION("uint16ToBytes Test") {
      uint16_t uint16Input = 65452;
      auto uint16Output = Utils::uint16ToBytes(uint16Input);
      std::string uint16ExpectedOutput = "\xff\xac";
      REQUIRE_THAT(uint16Output, Equals(uint16ExpectedOutput));
    }

    SECTION("uint8ToBytes Test") {
      uint8_t uint8Input = 120;
      auto uint8Output = Utils::uint8ToBytes(uint8Input);
      std::string uint8ExpectedOutput = "\x78";
      REQUIRE_THAT(uint8Output, Equals(uint8ExpectedOutput));
    }

    SECTION("randBytes Test") {
      std::string randBytesOutput = Utils::randBytes(32);
      REQUIRE(randBytesOutput.size() == 32);
    }

    SECTION("bytesToUint256 Test") {
      FixedStr<32> bytesStr(std::string("\xcb\x06\x75\x32\x90\xff\xac\x16\x72\x05\xd0\xf5\x3b\x64\xac\xfd\x80\xbe\x11\xed\xbb\x26\xa2\x24\xbe\xd9\x23\x9a\xe6\x74\x0e\x67"));
      auto uint256Output = Utils::bytesToUint256(bytesStr.view());
      uint256_t uint256ExpectedOutput = uint256_t("91830918212381802449294565349763096207758814059154440393436864477986483867239");
      REQUIRE(uint256Output == uint256ExpectedOutput);
    }

    SECTION("bytesToUint160 Test") {
      FixedStr<20> bytesStr(std::string("\x58\xc5\x95\xbe\xdf\x1d\xea\x53\x2c\xf0\x6a\xf9\x09\x1a\x51\xb7\x5a\x11\xda\x61"));
      auto uint160Output = Utils::bytesToUint160(bytesStr.view());
      uint160_t uint160ExpectedOutput = uint160_t("506797479317435130489084083375319966488594602593");
      REQUIRE(uint160Output == uint160ExpectedOutput);
    }

    SECTION("bytesToUint64 Test") {
      FixedStr<8> bytesStr(std::string("\x9a\xce\x8e\x96\x24\xe4\xed\x56"));
      auto uint64Output = Utils::bytesToUint64(bytesStr.view());
      uint64_t uint64ExpectedOutput = uint64_t(11155010102558518614);
      REQUIRE(uint64Output == uint64ExpectedOutput);
    }

    SECTION("bytesToUint32 Test") {
      FixedStr<4> bytesStr(std::string("\x77\x7b\xca\x9a"));
      auto uint32Output = Utils::bytesToUint32(bytesStr.view());
      uint32_t uint32ExpectedOutput = 2004601498;
      REQUIRE(uint32Output == uint32ExpectedOutput);
    }

    SECTION("bytesToUint16 Test") {
      FixedStr<2> bytesStr(std::string("\xff\xac"));
      auto uint16Output = Utils::bytesToUint16(bytesStr.view());
      uint16_t uint16ExpectedOutput = 65452;
      REQUIRE(uint16Output == uint16ExpectedOutput);
    }

    SECTION("bytesToUint8 Test") {
      FixedStr<1> bytesStr(std::string("\x78"));
      auto uint8Output = Utils::bytesToUint8(bytesStr.view());
      uint8_t uint8ExpectedOutput = 120;
      REQUIRE(uint8Output == uint8ExpectedOutput);
    }

    SECTION("hexCharToInt Test") {
      REQUIRE(Utils::hexCharToInt('0') == 0); // 0
      REQUIRE(Utils::hexCharToInt('1') == 1); // 1
      REQUIRE(Utils::hexCharToInt('2') == 2); // 2
      REQUIRE(Utils::hexCharToInt('3') == 3); // 3
      REQUIRE(Utils::hexCharToInt('4') == 4); // 4
      REQUIRE(Utils::hexCharToInt('5') == 5); // 5
      REQUIRE(Utils::hexCharToInt('6') == 6); // 6
      REQUIRE(Utils::hexCharToInt('7') == 7); // 7
      REQUIRE(Utils::hexCharToInt('8') == 8); // 8
      REQUIRE(Utils::hexCharToInt('9') == 9); // 9
      REQUIRE(Utils::hexCharToInt('a') == 10); // a
      REQUIRE(Utils::hexCharToInt('b') == 11); // b
      REQUIRE(Utils::hexCharToInt('c') == 12); // c
      REQUIRE(Utils::hexCharToInt('d') == 13); // d
      REQUIRE(Utils::hexCharToInt('e') == 14); // e
      REQUIRE(Utils::hexCharToInt('f') == 15); // f
      REQUIRE(Utils::hexCharToInt('A') == 10); // A
      REQUIRE(Utils::hexCharToInt('B') == 11); // B
      REQUIRE(Utils::hexCharToInt('C') == 12); // C
      REQUIRE(Utils::hexCharToInt('D') == 13); // D
      REQUIRE(Utils::hexCharToInt('E') == 14); // E
      REQUIRE(Utils::hexCharToInt('F') == 15); // F
    }

    SECTION("padLeft Test") {
      std::string inputStr = "abcdef";
      std::string outputStr = Utils::padLeft(inputStr, 10, '0');
      std::string outputStr2 = Utils::padLeft(inputStr, 20, '1');
      std::string expectedOutputStr = "0000abcdef";
      std::string expectedOutputStr2 = "11111111111111abcdef";
      REQUIRE_THAT(outputStr, Equals(expectedOutputStr));
      REQUIRE_THAT(outputStr2, Equals(expectedOutputStr2));
    }

    SECTION("padRight Test") {
      std::string inputStr = "abcdef";
      std::string outputStr = Utils::padRight(inputStr, 10, '0');
      std::string outputStr2 = Utils::padRight(inputStr, 20, '1');
      std::string expectedOutputStr = "abcdef0000";
      std::string expectedOutputStr2 = "abcdef11111111111111";
      REQUIRE_THAT(outputStr, Equals(expectedOutputStr));
      REQUIRE_THAT(outputStr2, Equals(expectedOutputStr2));
    }

    SECTION("fromBigEndian Test") {
      std::string_view inputBytes("\x10\x11\x40\xd6\xe7\x50\x6f\x80\x4c\xf7\xb0\x37\x0f\xa9\x0b\x04\xc5\xe9\x37\x4d\xdb\x0c\x8c\xbe\x12\xaf\x15\x0c\x8f\xf3\xee\x36");

      auto uint256Output = Utils::fromBigEndian<uint256_t>(inputBytes);
      auto uint64Ouput12To20 = Utils::fromBigEndian<uint64_t>(inputBytes.substr(12, 8));
      auto uint64Ouput20To28 = Utils::fromBigEndian<uint64_t>(inputBytes.substr(20, 8));
      auto uint64Output24To28 = Utils::fromBigEndian<uint64_t>(inputBytes.substr(24, 4));
      auto uint32Output28To32 = Utils::fromBigEndian<uint32_t>(inputBytes.substr(28, 4));
      auto uint160Output5To25 = Utils::fromBigEndian<uint160_t>(inputBytes.substr(5, 20));
      uint256_t uint256ExpectedOutput = uint256_t("7267489482988504755957722036644729207517128093499486419604741885099068616246");
      uint64_t uint64ExpectedOutput12To20 = uint64_t(1128445296761190221);
      uint64_t uint64ExpectedOutput20To28 = uint64_t(15784145542011884812);
      uint64_t uint64ExpectedOutput24To28 = uint64_t(313464076);
      uint32_t uint32ExpectedOutput28To32 = uint32_t(2415128118);
      uint160_t uint160ExpectedOutput5To25 = uint160_t("459205820946237488389499242237511570682479951378");
      REQUIRE(uint256Output == uint256ExpectedOutput);
      REQUIRE(uint64Ouput12To20 == uint64ExpectedOutput12To20);
      REQUIRE(uint64Ouput20To28 == uint64ExpectedOutput20To28);
      REQUIRE(uint64Output24To28 == uint64ExpectedOutput24To28);
      REQUIRE(uint32Output28To32 == uint32ExpectedOutput28To32);
      REQUIRE(uint160Output5To25 == uint160ExpectedOutput5To25);
    }

    SECTION("toLower Test") {
      std::string inputStr = "ABCDEF";
      std::string outputStr = inputStr;
      Utils::toLower(outputStr);
      std::string expectedOutputStr = "abcdef";
      REQUIRE_THAT(outputStr, Equals(expectedOutputStr));
    }

    SECTION("toUpper Test") {
      std::string inputStr = "abcdef";
      std::string outputStr = inputStr;
      Utils::toUpper(outputStr);
      std::string expectedOutputStr = "ABCDEF";
      REQUIRE_THAT(outputStr, Equals(expectedOutputStr));
    }
  }
}
