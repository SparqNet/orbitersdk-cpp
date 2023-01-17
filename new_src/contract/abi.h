#ifndef ABI_H
#define ABI_H

#include <string>

#include "../libs/json.hpp"

#include "../utils/utils.h"

/**
 * Namespace for Solidity ABI-related operations.
 */
namespace ABI {
  /**
   * Enum for the types of Solidity variables.
   * Equivalency is as follows:
   * - uint256 = uint256 (Solidity) = uint256_t (C++)
   * - uint256Arr = uint256[] (Solidity) = std::vector<uint256_t> (C++)
   * - address = address (Solidity) = Address (C++)
   * - addressArr = address[] (Solidity) = std::vector<Address> (C++)
   * - bool = bool (Solidity) = bool (C++)
   * - boolArr = bool[] (Solidity) = vector<bool> (C++)
   * - bytes = bytes (Solidity) = std::string (C++)
   * - bytesArr = bytes[] (Solidity) = std::vector<std::string> (C++)
   * - string = string (Solidity) = std::string (C++)
   * - stringArr = string[] (Solidity) = std::vector<std::string> (C++)
   */
  enum Types {
    uint256, uint256Arr, address, addressArr, boolean, booleanArr,
    bytes, bytesArr, string, stringArr
  };

  /// Class that encodes and packs native data types into Solidity ABI strings.
  class Encoder {
    private:
      std::string data; ///< Encoded Solidity ABI string.

      /**
       * Encode a function header into Solidity ABI format.
       * Requires the full function header, no spaces between args
       * (e.g. `func(arg1,arg2)`). The function will SHA3-hash the header
       * and return the first 4 hex bytes of the hash (aka the "functor").
       * @param func The function header to encode.
       * @return The encoded functor hex string.
       */
      std::string encodeFunction(std::string func);

      /**
       * Encode a 256-bit unsigned integer into Solidity ABI format.
       * @param num The 256-bit unsigned integer to encode.
       * @return The encoded uint256 hex string, padded 32 hex bytes to the LEFT.
       */
      std::string encodeUint256(uint256_t num);

      /**
       * Encode a 20-byte address into Solidity ABI format.
       * @param add The 20-byte address to encode.
       * @return The encoded address hex string, padded 32 bytes to the LEFT.
       */
      std::string encodeAddress(Address add);

      /**
       * Encode a boolean into Solidity ABI format.
       * @param b The boolean to encode.
       * @return The encoded boolean hex string, padded 32 bytes to the LEFT.
       */
      std::string encodeBool(bool b);

      /**
       * Encode a raw bytes or UTF-8 string into Solidity ABI format.
       * Solidity bytes and string types are parsed the exact same way,
       * except string REQUIRES an `utf8ToHex()` conversion.
       * This is done internally using `isHex(strict)` to discern from both types.
       * If `bytes`, APPEND `0x` to the start of the string.
       * If `string`, REMOVE `0x` from the start of the string.
       * @param bytes The raw bytes or UTF-8 string to encode.
       * @return The encoded hex bytes or string,
       *         padded to the nearest multiple of 32 bytes to the RIGHT.
       */
      std::string encodeBytes(std::string bytes);

      /**
       * Encode a 256-bit unsigned integer array into Solidity ABI format.
       * @param numV The 256-bit unsigned integer array to encode.
       * @return The encoded uint256[] hex string, with the proper offsets and lengths.
       */
      std::string encodeUint256Arr(std::vector<uint256_t> numV);

      /**
       * Encode a 20-byte address array into Solidity ABI format.
       * @param addV The 20-byte address array to encode.
       * @return The encoded address[] hex string, with the proper offsets and lengths.
       */
      std::string encodeAddressArr(std::vector<Address> addV);

      /**
       * Encode a boolean array into Solidity ABI format.
       * @param bV The boolean array to encode.
       * @return The encoded bool[] hex string, with the proper offsets and lengths.
       */
      std::string encodeBoolArr(std::vector<bool> bV);

      /**
       * Encode a raw bytes or UTF-8 string array into Solidity ABI format.
       * See `encodeBytes()` for more details.
       * @param bytesV The raw bytes or UTF-8 string array to encode.
       * @return The encoded bytes[] or string[] hex string, with the proper offsets and lengths.
       */
      std::string encodeBytesArr(std::vector<std::string> bytesV);

    public:
      /**
       * Constructor. Automatically encodes the data during construction.
       * @param data A list of variables to encode.
       * @param func (optional) The full function header to encode.
       *             Defaults to an empty string.
       */
      Encoder(std::vector<std::variant<
        uint256_t, std::vector<uint256_t>, Address, std::vector<Address>,
        bool, std::vector<bool>, std::string, std::vector<std::string>
      >> data, std::string func = "");

      const std::string& getData() { return this->data; } ///< Getter for `data`.

      /**
       * Get the length of `data`.
       * @return The total size of the data string.
       */
      size_t getDataLen() { return this->data.length(); }
  };

  /// Class that unpacks and decodes a Solidity ABI string into their original data types.
  class Decoder {
    private:
      /// List with the decoded native data types.
      std::vector<std::variant<
        uint256_t, std::vector<uint256_t>, Address, std::vector<Address>,
        bool, std::vector<bool>, std::string, std::vector<std::string>
      >> data;

      /**
       * Decode a 256-bit unsigned integer from the given Solidity data string.
       * @param data The Solidity data string to decode.
       * @start The index of the string to start decoding from.
       * @return The decoded 256-bit unsigned integer.
       */
      uint256_t decodeUint256(const std::string& data, const uint64_t& start);

      /**
       * Decode a 20-byte address from the given Solidity data string.
       * @param data The Solidity data string to decode.
       * @start The index of the string to start decoding from.
       * @return The decoded 20-byte address.
       */
      Address decodeAddress(const std::string& data, const uint64_t& start);

      /**
       * Decode a boolean from the given Solidity data string.
       * @param data The Solidity data string to decode.
       * @start The index of the string to start decoding from.
       * @return The decoded boolean.
       */
      bool decodeBool(const std::string& data, const uint64_t& start);

      /**
       * Decode a raw bytes or UTF-8 string from the given Solidity data string.
       * Decoding bytes and string in Solidity is done the exact same way,
       * as we are dealing with data as raw bytes anyway.
       * @param data The Solidity data string to decode.
       * @start The index of the string to start decoding from.
       * @return The decoded raw bytes or UTF-8 string.
       */
      std::string decodeBytes(const std::string& data, const uint64_t& start);

      /**
       * Decode a 256-bit unsigned integer array from the given Solidity data string.
       * @param data The Solidity data string to decode.
       * @start The index of the string to start decoding from.
       * @return The decoded 256-bit unsigned integer array.
       */
      std::vector<uint256_t> decodeUint256Arr(const std::string& data, const uint64_t& start);

      /**
       * Decode a 20-byte address array from the given Solidity data string.
       * @param data The Solidity data string to decode.
       * @start The index of the string to start decoding from.
       * @return The decoded 20-byte address array.
       */
      std::vector<Address> decodeAddressArr(const std::string& data, const uint64_t& start);

      /**
       * Decode a boolean array from the given Solidity data string.
       * @param data The Solidity data string to decode.
       * @start The index of the string to start decoding from.
       * @return The decoded boolean array.
       */
      std::vector<bool> decodeBoolArr(const std::string& data, const uint64_t& start);

      /**
       * Decode a raw bytes or UTF-8 string array from the given Solidity data string.
       * See `decodeBytes()` for more details.
       * @param data The Solidity data string to decode.
       * @start The index of the string to start decoding from.
       * @return The decoded raw bytes or UTF-8 string array.
       */
      std::vector<std::string> decodeBytesArr(const std::string& data, const uint64_t& start);

    public:
      /**
       * Constructor. Automatically decodes the data during construction.
       * @param types An ordered list of expected Solidity types to decode.
       * @param bytes The full Solidity ABI string to decode.
       */
      Decoder(const std::vector<ABI::Types>& types, const std::string& bytes);

      /**
       * Get a specific data type from the decoded `data` list.
       * @param index The index of the data type to get.
       * @return The decoded data type.
       */
      template <typename T> T getData(const uint64_t& index) {
        if (index >= data.size()) throw std::out_of_range("Index out of range");
        if (std::holds_alternative<T>(data[index])) return std::get<T>(data[index]);
        throw std::runtime_error("Type mismatch");
      }

      /**
       * Get the size of the `data` list.
       * @return The total number of decoded types.
       */
      size_t getDataSize() { return data.size(); }
  };

  /// Same as %Encoder, but works strictly with JSON objects.
  class JSONEncoder {
    private:
      /**
       * Check if a given type is an array.
       * @param type The type value that will be checked.
       * @return `true` if type is array, `false` otherwise.
       */
      bool typeIsArray(const Types& type);

    public:
      /**
       * List of methods as key/value pairs.
       * Key is the method name, value is an ordered list of the method's parameter types.
       */
      std::map<std::string, std::vector<Types>> methods;

      /**
       * List of functors as key/value pairs.
       * Key is the function name, value is the functor
       * (first 4 hex bytes of the SHA-3 hash of the function header).
       */
      std::map<std::string, std::string> functors;

      /**
       * Constructor.
       * @param interface The full Solidity ABI function data to encode as a JSON object.
       */
      JSONEncoder(const json& interface);

      /**
       * Alternative constructor.
       * @param func The function's name.
       * @param args The function's arguments, with "type" and "value" (or "t" and "v").
       *             e.g. `{{"t", "string"}, {"v", "Hello!%"}}`
       * @return The encoded Solidity ABI function.
       */
      std::string operator()(const std::string& func, const json& args);
  };
};

#endif  // ABI_H