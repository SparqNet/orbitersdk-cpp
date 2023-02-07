#include "abi.h"

std::string ABI::Encoder::encodeFunction(std::string func) {
  return Utils::sha3(func).get().substr(0, 4);
}

std::string ABI::Encoder::encodeUint256(uint256_t num) { return Hash(num).get(); }

std::string ABI::Encoder::encodeAddress(Address add) {
  std::string addStr = add.hex();
  Utils::patchHex(addStr);
  return Utils::hexToBytes(Utils::padLeft(addStr, 64));
}

std::string ABI::Encoder::encodeBool(bool b) {
  return Utils::hexToBytes(Utils::padLeft(((b) ? "1" : "0"), 64));
}

std::string ABI::Encoder::encodeBytes(std::string bytes) {
  std::string strip, off, len, data = "";
  int pad = 0;
  strip = (Utils::isHex(bytes, true)) ? bytes : Utils::utf8ToHex(bytes);  // Bytes or string
  Utils::patchHex(strip);
  off = Utils::padLeft(Utils::uintToHex(32), 64);
  len = Utils::padLeft(Utils::uintToHex(strip.length() / 2), 64);
  do { pad += 64; } while (pad < strip.length());
  data = Utils::padRight(strip, pad);
  return Utils::hexToBytes(off + len + data);
}

std::string ABI::Encoder::encodeUint256Arr(std::vector<uint256_t> numV) {
  std::string arrOff, arrLen, arrData = "";
  arrOff = Utils::padLeft(Utils::uintToHex(32), 64);
  arrLen = Utils::padLeft(Utils::uintToHex(numV.size()), 64);
  for (uint256_t num : numV) arrData += Utils::bytesToHex(encodeUint256(num));
  return Utils::hexToBytes(arrOff + arrLen + arrData);
}

std::string ABI::Encoder::encodeAddressArr(std::vector<Address> addV) {
  std::string arrOff, arrLen, arrData = "";
  arrOff = Utils::padLeft(Utils::uintToHex(32), 64);
  arrLen = Utils::padLeft(Utils::uintToHex(addV.size()), 64);
  for (Address add : addV) arrData += Utils::bytesToHex(encodeAddress(add));
  return Utils::hexToBytes(arrOff + arrLen + arrData);
}

std::string ABI::Encoder::encodeBoolArr(std::vector<bool> bV) {
  std::string arrOff, arrLen, arrData = "";
  arrOff = Utils::padLeft(Utils::uintToHex(32), 64);
  arrLen = Utils::padLeft(Utils::uintToHex(bV.size()), 64);
  for (bool b : bV) arrData += Utils::bytesToHex(encodeBool(b));
  return Utils::hexToBytes(arrOff + arrLen + arrData);
}

std::string ABI::Encoder::encodeBytesArr(std::vector<std::string> bytesV) {
  std::string arrOff, arrLen = "";
  std::vector<std::string> bytesStrip, bytesOff, bytesLen, bytesData = {};
  arrOff = Utils::padLeft(Utils::uintToHex(32), 64);
  arrLen = Utils::padLeft(Utils::uintToHex(bytesV.size()), 64);
  int pads = 0;
  for (int i = 0; i < bytesV.size(); i++) {
    std::string bS, bO, bL, bD = "";
    int p = 0;
    bS = (Utils::isHex(bytesV[i], true)) ? bytesV[i] : Utils::utf8ToHex(bytesV[i]); // Bytes or string
    Utils::patchHex(bS);
    if (bS.length() % 2 != 0) { bS.insert(0, "0"); } // Complete odd bytes ("aaa" = "0aaa")
    bL = Utils::uintToHex(bS.length() / 2); // Get length first so we can get the right offset
    bO = Utils::uintToHex((32 * bytesV.size()) + (32 * i) + (32 * pads)); // (offsets) + (lengths) + (datas)
    do { p += 64; } while (p < bS.length());
    pads += (p / 64);
    bD = Utils::padRight(bS, p);
    bytesStrip.push_back(Utils::padLeft(bS, 64));
    bytesOff.push_back(Utils::padLeft(bO, 64));
    bytesLen.push_back(Utils::padLeft(bL, 64));
    bytesData.push_back(Utils::padRight(bD, 64));
  }
  std::string ret = arrOff + arrLen;
  for (std::string off : bytesOff) ret += off;
  for (int i = 0; i < bytesV.size(); i++) ret += bytesLen[i] + bytesData[i];
  return Utils::hexToBytes(ret);
}

ABI::Encoder::Encoder(std::vector<std::variant<
  uint256_t, std::vector<uint256_t>, Address, std::vector<Address>,
  bool, std::vector<bool>, std::string, std::vector<std::string>
>> data, std::string func) {
  // Handle function ID first if it exists.
  // We have to check the existence of "()", every type inside it,
  // *and* if type positions on both header and data vector are the same
  // (e.g. arg[0] on header is string, arg[0] on data vector has to be string too).
  if (!func.empty()) {
    if (func.find("(") == std::string::npos || func.find(")") == std::string::npos) {
      throw std::runtime_error("Invalid function header");
    }
    std::string funcTmp = func;
    funcTmp.erase(0, funcTmp.find("(") + 1);
    funcTmp.replace(funcTmp.find(")"), 1, ",");
    int pos, posct = 0;
    while ((pos = funcTmp.find(",")) != std::string::npos) {
      std::string funcType = funcTmp.substr(0, pos);
      if (
        funcType != "uint256" && funcType != "address" &&
        funcType != "bool" && funcType != "bytes" &&
        funcType != "string" && funcType != "uint256[]" &&
        funcType != "address[]" && funcType != "bool[]" &&
        funcType != "bytes[]" && funcType != "string[]"
      ) {
        throw std::runtime_error("Invalid function header type");
      }
      if (
        (funcType == "uint256" && !std::holds_alternative<uint256_t>(data[posct])) ||
        (funcType == "address" && !std::holds_alternative<Address>(data[posct])) ||
        (funcType == "bool" && !std::holds_alternative<bool>(data[posct])) ||
        (funcType == "bytes" && !std::holds_alternative<std::string>(data[posct])) ||
        (funcType == "string" && !std::holds_alternative<std::string>(data[posct])) ||
        (funcType == "uint256[]" && !std::holds_alternative<std::vector<uint256_t>>(data[posct])) ||
        (funcType == "address[]" && !std::holds_alternative<std::vector<Address>>(data[posct])) ||
        (funcType == "bool[]" && !std::holds_alternative<std::vector<bool>>(data[posct])) ||
        (funcType == "bytes[]" && !std::holds_alternative<std::vector<std::string>>(data[posct])) ||
        (funcType == "string[]" && !std::holds_alternative<std::vector<std::string>>(data[posct]))
      ) {
        throw std::runtime_error("Header and data types at position " + std::to_string(posct) + " don't match");
      }
      posct++;
      funcTmp.erase(0, pos + 1);
    }
    this->data += encodeFunction(func);
  }

  // Handle each data type and value
  uint64_t nextOffset = 32 * data.size();
  std::string arrToAppend = "";
  for (auto arg : data) {
    if (std::holds_alternative<uint256_t>(arg)) {
      this->data += encodeUint256(std::get<uint256_t>(arg));
    } else if (std::holds_alternative<Address>(arg)) {
      this->data += encodeAddress(std::get<Address>(arg));
    } else if (std::holds_alternative<bool>(arg)) {
      this->data += encodeBool(std::get<bool>(arg));
    } else if (std::holds_alternative<std::string>(arg)) {
      this->data += Utils::hexToBytes(Utils::padLeft(Utils::uintToHex(nextOffset), 64));
      std::string packed = encodeBytes(std::get<std::string>(arg));
      nextOffset += 32 * (packed.length() / 32); // Both offset and packed in bytes
      arrToAppend += packed;
    } else if (std::holds_alternative<std::vector<uint256_t>>(arg)) {
      std::vector<uint256_t> argData = std::get<std::vector<uint256_t>>(arg);
      this->data += Utils::hexToBytes(Utils::padLeft(Utils::uintToHex(nextOffset), 64));
      nextOffset += 32 * argData.size();  // In bytes
      arrToAppend += encodeUint256Arr(argData).substr(32);
    } else if (std::holds_alternative<std::vector<Address>>(arg)) {
      std::vector<Address> argData = std::get<std::vector<Address>>(arg);
      this->data += Utils::hexToBytes(Utils::padLeft(Utils::uintToHex(nextOffset), 64));
      nextOffset += 32 * argData.size();  // In bytes
      arrToAppend += encodeAddressArr(argData).substr(32);
    } else if (std::holds_alternative<std::vector<bool>>(arg)) {
      std::vector<bool> argData = std::get<std::vector<bool>>(arg);
      this->data += Utils::hexToBytes(Utils::padLeft(Utils::uintToHex(nextOffset), 64));
      nextOffset += 32 * argData.size();  // In bytes
      arrToAppend += encodeBoolArr(argData).substr(32);
    } else if (std::holds_alternative<std::vector<std::string>>(arg)) {
      std::vector<std::string> argData = std::get<std::vector<std::string>>(arg);
      this->data += Utils::hexToBytes(Utils::padLeft(Utils::uintToHex(nextOffset), 64));
      std::string packed = encodeBytesArr(argData).substr(32);
      nextOffset += 32 * (packed.length() / 32); // Both offset and packed in bytes
      arrToAppend += packed;
    }
  }
  this->data += arrToAppend;
}

uint256_t ABI::Decoder::decodeUint256(const std::string& data, const uint64_t& start) {
  if (start + 32 > data.size()) throw std::runtime_error("Data too short");
  std::string tmp;
  std::copy(data.begin() + start, data.begin() + start + 32, std::back_inserter(tmp));
  uint256_t ret = Utils::bytesToUint256(tmp);
  return ret;
}

Address ABI::Decoder::decodeAddress(const std::string& data, const uint64_t& start) {
  if (start + 32 > data.size()) throw std::runtime_error("Data too short");
  std::string tmp;
  std::copy(data.begin() + start + 12, data.begin() + start + 32, std::back_inserter(tmp)); // Skip first 12 bytes
  Address ret(tmp, false);
  return ret;
}

bool ABI::Decoder::decodeBool(const std::string& data, const uint64_t& start) {
  if (start + 32 > data.size()) throw std::runtime_error("Data too short");
  return (data[start + 31] == 0x01);  // Bool value ("00"/"01") is at the very end
}

std::string ABI::Decoder::decodeBytes(const std::string& data, const uint64_t& start) {
  // Get bytes offset
  std::string tmp;
  std::copy(data.begin() + start, data.begin() + start + 32, std::back_inserter(tmp));
  uint64_t bytesStart = boost::lexical_cast<uint64_t>(Utils::bytesToUint256(tmp));

  // Get bytes length
  tmp.clear();
  std::copy(data.begin() + bytesStart, data.begin() + bytesStart + 32, std::back_inserter(tmp));
  uint64_t bytesLength = boost::lexical_cast<uint64_t>(Utils::bytesToUint256(tmp));

  // Size sanity check
  if (start + 32 + 32 + bytesLength > data.size()) throw std::runtime_error("Data too short");

  // Get bytes data
  tmp.clear();
  std::copy(
    data.begin() + bytesStart + 32,
    data.begin() + bytesStart + 32 + bytesLength,
    std::back_inserter(tmp)
  );
  return tmp;
}

std::vector<uint256_t> ABI::Decoder::decodeUint256Arr(const std::string& data, const uint64_t& start) {
  // Get array offset
  std::string tmp;
  std::copy(data.begin() + start, data.begin() + start + 32, std::back_inserter(tmp));
  uint64_t arrayStart = boost::lexical_cast<uint64_t>(Utils::bytesToUint256(tmp));

  // Get array length
  tmp.clear();
  std::copy(data.begin() + arrayStart, data.begin() + arrayStart + 32, std::back_inserter(tmp));
  uint64_t arrayLength = boost::lexical_cast<uint64_t>(Utils::bytesToUint256(tmp));

  // Size sanity check
  if (arrayStart + 32 + (arrayLength * 32) > data.size()) throw std::runtime_error("Data too short");

  // Get array data
  std::vector<uint256_t> tmpArr;
  for (uint64_t i = 0; i < arrayLength; i++) {
    tmp.clear();
    std::copy(
      data.begin() + arrayStart + 32 + (i * 32),
      data.begin() + arrayStart + 32 + (i * 32) + 32,
      std::back_inserter(tmp)
    );
    uint256_t value = Utils::bytesToUint256(tmp);
    tmpArr.emplace_back(value);
  }

  return tmpArr;
}

std::vector<Address> ABI::Decoder::decodeAddressArr(const std::string& data, const uint64_t& start) {
  // Get array offset
  std::string tmp;
  std::copy(data.begin() + start, data.begin() + start + 32, std::back_inserter(tmp));
  uint64_t arrayStart = boost::lexical_cast<uint64_t>(Utils::bytesToUint256(tmp));

  // Get array length
  tmp.clear();
  std::copy(data.begin() + arrayStart, data.begin() + arrayStart + 32, std::back_inserter(tmp));
  uint64_t arrayLength = boost::lexical_cast<uint64_t>(Utils::bytesToUint256(tmp));

  // Size sanity check
  if (arrayStart + 32 + (arrayLength * 32) > data.size()) throw std::runtime_error("Data too short");

  // Get array data
  std::vector<Address> tmpArr;
  for (uint64_t i = 0; i < arrayLength; i++) {
    tmp.clear();
    // Don't forget to skip the first 12 bytes of an address!
    std::copy(
      data.begin() + arrayStart + 32 + (i * 32) + 12,
      data.begin() + arrayStart + 32 + (i * 32) + 32,
      std::back_inserter(tmp)
    );
    Address address(tmp, false);
    tmpArr.emplace_back(address);
  }

  return tmpArr;
}

std::vector<bool> ABI::Decoder::decodeBoolArr(const std::string& data, const uint64_t& start) {
  // Get array offset
  std::string tmp;
  std::copy(data.begin() + start, data.begin() + start + 32, std::back_inserter(tmp));
  uint64_t arrayStart = boost::lexical_cast<uint64_t>(Utils::bytesToUint256(tmp));

  // Get array length
  tmp.clear();
  std::copy(data.begin() + arrayStart, data.begin() + arrayStart + 32, std::back_inserter(tmp));
  uint64_t arrayLength = boost::lexical_cast<uint64_t>(Utils::bytesToUint256(tmp));

  // Size sanity check
  if (arrayStart + 32 + (arrayLength * 32) > data.size()) throw std::runtime_error("Data too short");

  // Get array data
  std::vector<bool> tmpArr;
  for (uint64_t i = 0; i < arrayLength; i++) tmpArr.emplace_back(
    (data[arrayStart + 32 + (i * 32) + 31] == 0x01)
  );
  return tmpArr;
}

std::vector<std::string> ABI::Decoder::decodeBytesArr(const std::string& data, const uint64_t& start) {
  // Get array offset
  std::string tmp;
  std::copy(data.begin() + start, data.begin() + start + 32, std::back_inserter(tmp));
  uint64_t arrayStart = boost::lexical_cast<uint64_t>(Utils::bytesToUint256(tmp));

  // Get array length
  tmp.clear();
  std::copy(data.begin() + arrayStart, data.begin() + arrayStart + 32, std::back_inserter(tmp));
  uint64_t arrayLength = boost::lexical_cast<uint64_t>(Utils::bytesToUint256(tmp));

  std::vector<std::string> tmpVec;
  for (uint64_t i = 0; i < arrayLength; ++i) {
    // Get bytes offset
    tmp.clear();
    std::copy(
      data.begin() + arrayStart + 32 + (i * 32),
      data.begin() + arrayStart + 32 + (i * 32) + 32,
      std::back_inserter(tmp)
    );
    uint64_t bytesStart = boost::lexical_cast<uint64_t>(Utils::bytesToUint256(tmp)) + arrayStart + 32;

    // Get bytes length
    tmp.clear();
    std::copy(
      data.begin() + bytesStart,
      data.begin() + bytesStart + 32,
      std::back_inserter(tmp)
    );
    uint64_t bytesLength = boost::lexical_cast<uint64_t>(Utils::bytesToUint256(tmp));

    // Individual size sanity check
    if (bytesStart + 32 + bytesLength > data.size()) throw std::runtime_error("Data too short");

    // Get bytes data
    tmp.clear();
    std::copy(
      data.begin() + bytesStart + 32,
      data.begin() + bytesStart + 32 + bytesLength,
      std::back_inserter(tmp)
    );
    tmpVec.emplace_back(tmp);
  }

  return tmpVec;
}

ABI::Decoder::Decoder(std::vector<ABI::Types> const& types, std::string const& bytes) {
  uint64_t argIdx = 0;
  uint64_t dataIdx = 0;
  while (argIdx < types.size()) {
    if (types[argIdx] == ABI::Types::uint256) {
      this->data.emplace_back(decodeUint256(bytes, dataIdx));
    } else if (types[argIdx] == ABI::Types::uint256Arr) {
      this->data.emplace_back(decodeUint256Arr(bytes, dataIdx));
    } else if (types[argIdx] == ABI::Types::address) {
      this->data.emplace_back(decodeAddress(bytes, dataIdx));
    } else if (types[argIdx] == ABI::Types::addressArr) {
      this->data.emplace_back(decodeAddressArr(bytes, dataIdx));
    } else if (types[argIdx] == ABI::Types::boolean) {
      this->data.emplace_back(decodeBool(bytes, dataIdx));
    } else if (types[argIdx] == ABI::Types::booleanArr) {
      this->data.emplace_back(decodeBoolArr(bytes, dataIdx));
    } else if (types[argIdx] == ABI::Types::string || types[argIdx] == ABI::Types::bytes) {
      this->data.emplace_back(decodeBytes(bytes, dataIdx));
    } else if (types[argIdx] == ABI::Types::stringArr || types[argIdx] == ABI::Types::bytesArr) {
      this->data.emplace_back(decodeBytesArr(bytes, dataIdx));
    }
    dataIdx += 32;
    ++argIdx;
  }
}

bool ABI::JSONEncoder::typeIsArray(const Types& type) {
  return (
    type == ABI::Types::uint256Arr || type == ABI::Types::addressArr ||
    type == ABI::Types::booleanArr || type == ABI::Types::bytesArr ||
    type == ABI::Types::stringArr
  );
}

ABI::JSONEncoder::JSONEncoder(const json& interface) {
  // Parse contract
  for (auto item : interface) {
    if (item["type"].get<std::string>() == "function") {
      std::string funcName = item["name"].get<std::string>();
      std::string funcAll = funcName + "(";
      for (auto arg : item["inputs"]) {
        Types argType;
        std::string argTypeStr = arg["type"].get<std::string>();
        funcAll += argTypeStr + ",";
        if (argTypeStr == "uint256") argType = ABI::Types::uint256;
        else if (argTypeStr == "uint256[]") argType = ABI::Types::uint256Arr;
        else if (argTypeStr == "address") argType = ABI::Types::address;
        else if (argTypeStr == "address[]") argType = ABI::Types::addressArr;
        else if (argTypeStr == "bool") argType = ABI::Types::boolean;
        else if (argTypeStr == "bool[]") argType = ABI::Types::booleanArr;
        else if (argTypeStr == "bytes") argType = ABI::Types::bytes;
        else if (argTypeStr == "bytes[]") argType = ABI::Types::bytesArr;
        else if (argTypeStr == "string") argType = ABI::Types::string;
        else if (argTypeStr == "string[]") argType = ABI::Types::stringArr;
        else {
          // All uints (128, 64, etc.) are encoded the same way
          if (argTypeStr.find("uint") != std::string::npos) {
            argType = (argTypeStr.find("[]") != std::string::npos)
              ? ABI::Types::uint256Arr : ABI::Types::uint256;
          } else if (argTypeStr.find("bytes") != std::string::npos) {
            argType = (argTypeStr.find("[]") != std::string::npos)
              ? ABI::Types::bytesArr : ABI::Types::bytes;
          }
        }
        methods[funcName].push_back(argType);
      }
      funcAll.pop_back(); // Remove last ,
      funcAll += ")";
      functors[funcName] = Utils::sha3(funcAll).hex().substr(0,8);
    }
  }
}

std::string ABI::JSONEncoder::operator()(const std::string& func, const json& args) {
  if (!methods.count(func)) {
    Utils::LogPrint(Log::ABI, __func__, " Error: ABI Functor Not Found");
    throw std::runtime_error(std::string(__func__) + "ABI Functor Not Found");
  }

  if (!args.is_array()) {
    Utils::LogPrint(Log::ABI, __func__, " Error: ABI Invalid JSON Array");
    throw std::runtime_error(std::string(__func__) + "ABI Invalid JSON Array");
  }

  if (args.size() != methods[func].size()) {
    Utils::LogPrint(Log::ABI, __func__, " Error: ABI Invalid Arguments Length");
    throw std::runtime_error(std::string(__func__) + "ABI Invalid Arguments Length");
  }

  // Streamline all types from function into a string
  std::vector<std::string> funcTypes;
  for (Types t : methods[func]) {
    switch (t) {
      case Types::uint256: funcTypes.push_back("uint256"); break;
      case Types::uint256Arr: funcTypes.push_back("uint256[]"); break;
      case Types::address: funcTypes.push_back("address"); break;
      case Types::addressArr: funcTypes.push_back("address[]"); break;
      case Types::boolean: funcTypes.push_back("bool"); break;
      case Types::booleanArr: funcTypes.push_back("bool[]"); break;
      case Types::bytes: funcTypes.push_back("bytes"); break;
      case Types::bytesArr: funcTypes.push_back("bytes[]"); break;
      case Types::string: funcTypes.push_back("string"); break;
      case Types::stringArr: funcTypes.push_back("string[]"); break;
    }
  }

  // Mount the function header and JSON arguments manually,
  // with the proper formatting both need to go through packMulti()
  std::string funcStr = func + "(";
  std::vector<std::variant<
    uint256_t, std::vector<uint256_t>, Address, std::vector<Address>,
    bool, std::vector<bool>, std::string, std::vector<std::string>
  >> argsVec;
  for (int i = 0; i < funcTypes.size(); i++) {
    funcStr += funcTypes[i] + ",";
    if (funcTypes[i] == "uint256") {
      argsVec.push_back(boost::lexical_cast<uint256_t>(args[i].get<std::string>()));
    } else if (funcTypes[i] == "address") {
      argsVec.push_back(Address(args[i].get<std::string>(), true));
    } else if (funcTypes[i] == "bool") {
      argsVec.push_back(args[i].get<std::string>() == "1");
    } else if (funcTypes[i] == "bytes" || funcTypes[i] == "string") {
      argsVec.push_back(args[i].get<std::string>());
    } else {
      json argArr = args[i];
      if (funcTypes[i] == "uint256[]") {
        std::vector<uint256_t> arr;
        for (std::string arg : argArr) arr.push_back(boost::lexical_cast<uint256_t>(arg));
        argsVec.push_back(arr);
      } else if (funcTypes[i] == "address[]") {
        std::vector<Address> arr;
        for (std::string arg : argArr) arr.push_back(Address(arg, true));
        argsVec.push_back(arr);
      } else if (funcTypes[i] == "bool[]") {
        std::vector<bool> arr;
        for (std::string arg : argArr) arr.push_back(arg == "1");
        argsVec.push_back(arr);
      } else if (funcTypes[i] == "bytes[]" || funcTypes[i] == "string[]") {
        std::vector<std::string> arr;
        for (std::string arg : argArr) arr.push_back(arg);
        argsVec.push_back(arr);
      }
    }
  }
  funcStr.pop_back();  // Remove last ","
  funcStr += ")";
  return ABI::Encoder(argsVec, funcStr).get();
}
