#ifndef UTILS_H
#define UTILS_H

#include "../include/web3cpp/devcore/Common.h"

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <sstream>
#include <fstream>
#include <chrono>

namespace Utils {
    
   // u256 to 48 bytes
   dev::bytes u256toBytes(dev::u256 value);
   // 48 bytes to u256
   dev::u256 bytesTou256(dev::bytes value);

   std::string secondsToGoTimeStamp(uint64_t seconds);

   std::string hashToBytes(std::string hash);

   std::string uintToHex(std::string input, bool isPadded = true);
   
   void logToFile(std::string str);

}

#endif // UTILS_H
