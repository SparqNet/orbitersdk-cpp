# Pre-setup
cmake_minimum_required(VERSION 3.19.0)

include(cmake/cable/bootstrap.cmake)
include(CableBuildInfo)
include(CableBuildType)
include(GNUInstallDirs)
cable_set_build_type(DEFAULT Debug CONFIGURATION_TYPES Debug;Release;RelWithDebInfo)

# Map current configuration to configurations of imported targets.
set(CMAKE_MAP_IMPORTED_CONFIG_Debug Debug)
set(CMAKE_MAP_IMPORTED_CONFIG_RelWithDebInfo Debug)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24+
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

# Project data
project(orbitersdk VERSION 0.2.0 DESCRIPTION "Sparq subnet")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(DEBUG ON CACHE BOOL "Debug mode")
if(DEBUG)
  set(CMAKE_CXX_FLAGS "-O0 -g -fsanitize=address -fno-inline -fno-eliminate-unused-debug-types -fstack-protector -Werror=unused-variable") # Provides faster compile time.
elseif(SONARQUBE_ANALYSIS)
  set(CMAKE_CXX_FLAGS "-O0 -g --coverage")
else()
  set(CMAKE_CXX_FLAGS "-O2 -Werror=unused-variable")
endif()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a") # Always look for static libraries - "ZLIB_USE_STATIC_LIBS" was added in 3.24
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clang-tidy

# Set project version inside the code
configure_file(
  ${CMAKE_SOURCE_DIR}/src/utils/options.h.in
  ${CMAKE_SOURCE_DIR}/src/utils/options.h
  @ONLY
)

# External project data
set(BUILD_TESTS ON CACHE BOOL "Build helper unit testing program")
set(BUILD_DISCOVERY ON CACHE BOOL "Build helper discovery node program")
set(BUILD_AVALANCHEGO OFF CACHE BOOL "Build with AvalancheGo wrapping")
set(BUILD_TOOLS OFF CACHE BOOL "Build tools related to subnet")
set(USE_LINT OFF CACHE BOOL "Run linter on compile (clang-tidy)")
if(USE_LINT)
  set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-header-filter=.;-checks=-*,abseil-*,boost-*,bugprone-*,cert-*,clang-analyzer-*,concurrency-*,cppcoreguidelines-*,hicpp-*,misc-*,modernize-*,performance-*,portability-*,readability-*")
endif()

# Echo CMake vars during config
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "C++ standard is required: ${CMAKE_CXX_STANDARD_REQUIRED}")
message(STATUS "C++ extensions: ${CMAKE_CXX_EXTENSIONS}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Using PIC: ${CMAKE_POSITION_INDEPENDENT_CODE}")
message(STATUS "Find libs with suffix: ${CMAKE_FIND_LIBRARY_SUFFIXES}")
message("Building tests: ${BUILD_TESTS}")
message("Building Discovery Node: ${BUILD_DISCOVERY}")
message("Building AvalancheGo support: ${BUILD_AVALANCHEGO}")
message("Building tools: ${BUILD_TOOLS}")
message("Using lint: ${USE_LINT}")

cable_add_buildinfo_library(PROJECT_NAME orbitersdk)

# System package configs (built-in)
set(Boost_USE_STATIC_LIBS ON)
set(OPENSSL_USE_STATIC_LIBS ON)
set(Protobuf_USE_STATIC_LIBS ON)

# Find system packages (built-in)
find_package(Threads)
find_package(Boost 1.74.0 REQUIRED COMPONENTS chrono filesystem program_options system thread nowide)
find_package(OpenSSL 1.1.1 REQUIRED)
find_package(ZLIB REQUIRED)

# Find system packages (custom)
find_package(CryptoPP 8.2.0 REQUIRED)
find_package(Scrypt REQUIRED)

# Add external modules
include(cmake/ProjectBoostCertify.cmake)  # Boost Certify
include(cmake/ProjectEthash.cmake)        # Ethash
include(cmake/ProjectSecp256k1.cmake)     # Bitcoin core fast implementation
include(cmake/ProjectSpeedb.cmake)        # Speedb (Level/RocksDB drop-in replacement)

# Add catch2 as a library
add_library(catch2
  ${CMAKE_SOURCE_DIR}/src/libs/catch2/catch_amalgamated.hpp
  ${CMAKE_SOURCE_DIR}/src/libs/catch2/catch_amalgamated.cpp
)
target_include_directories(catch2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/catch2)

# Check compiler variable sizes
include(cmake/CheckSizes.cmake)

# Add AvalancheGo wrapper dependencies if compiling it
if(BUILD_AVALANCHEGO)
  find_package(Absl REQUIRED) # Built-in is hardcoded to SHARED, this one to STATIC
  find_package(Cares REQUIRED)
  find_package(Protobuf 3.12 REQUIRED)
  find_package(GRPC REQUIRED)
endif()

# Include directories for headers and libs
include_directories(
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/src/libs"
  "${CMAKE_SOURCE_DIR}/build"
  "${CMAKE_SOURCE_DIR}/build/deps/include"
)

link_directories(
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/src/libs"
  "${CMAKE_SOURCE_DIR}/build"
  "${CMAKE_SOURCE_DIR}/build/deps/lib"
)

# Organize, compile and link orbitersdk libs
add_subdirectory(src/contract)
add_subdirectory(src/core)
add_subdirectory(src/net)
add_subdirectory(src/utils)
add_subdirectory(tests)

# Generate gRPC files if building with support for AvalancheGo.
# Headers/sources are always cleaned at configure so they can be regenerated at build
if(BUILD_AVALANCHEGO)
  file(REMOVE
    "${CMAKE_SOURCE_DIR}/proto/vm.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/aliasreader.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/appsender.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/keystore.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/messenger.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/sharedmemory.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/rpcdb.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/vm.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/aliasreader.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/appsender.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/keystore.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/messenger.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/sharedmemory.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/rpcdb.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/vm.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/aliasreader.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/appsender.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/keystore.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/messenger.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/sharedmemory.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/rpcdb.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/vm.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/aliasreader.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/appsender.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/keystore.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/messenger.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/sharedmemory.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/rpcdb.pb.h"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/aliasreader.grpc.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/aliasreader.grpc.pb.h"
    COMMAND "protoc"
    ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
         --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/aliasreader.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/appsender.grpc.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/appsender.grpc.pb.h"
    COMMAND "protoc"
    ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
         --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/appsender.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/keystore.grpc.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/keystore.grpc.pb.h"
    COMMAND "protoc"
    ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
         --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/keystore.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/messenger.grpc.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/messenger.grpc.pb.h"
    COMMAND "protoc"
    ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
         --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/messenger.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/metrics.grpc.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/metrics.grpc.pb.h"
    COMMAND "protoc"
    ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
         --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/metrics.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/sharedmemory.grpc.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/sharedmemory.grpc.pb.h"
    COMMAND "protoc"
    ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
         --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/sharedmemory.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/rpcdb.grpc.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/rpcdb.grpc.pb.h"
    COMMAND "protoc"
    ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
         --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/rpcdb.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/vm.grpc.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/vm.grpc.pb.h"
    COMMAND "protoc"
    ARGS --grpc_out="${CMAKE_SOURCE_DIR}/proto"
         --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         --experimental_allow_proto3_optional
         "${CMAKE_SOURCE_DIR}/proto/vm.proto"
  )

  # Protobuf PROTOBUF_GENERATE_CPP does NOT work with --experimental_allow_proto3_optional
  # requiring us to go over protobuf files with add_custom_command

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/aliasreader.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/aliasreader.pb.h"
    COMMAND "protoc"
    ARGS --cpp_out="${CMAKE_SOURCE_DIR}/proto"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/aliasreader.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/appsender.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/appsender.pb.h"
    COMMAND "protoc"
    ARGS --cpp_out="${CMAKE_SOURCE_DIR}/proto"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/appsender.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/keystore.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/keystore.pb.h"
    COMMAND "protoc"
    ARGS --cpp_out="${CMAKE_SOURCE_DIR}/proto"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/keystore.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/messenger.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/messenger.pb.h"
    COMMAND "protoc"
    ARGS --cpp_out="${CMAKE_SOURCE_DIR}/proto"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/messenger.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/metrics.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/metrics.pb.h"
    COMMAND "protoc"
    ARGS --cpp_out="${CMAKE_SOURCE_DIR}/proto"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/metrics.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/sharedmemory.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/sharedmemory.pb.h"
    COMMAND "protoc"
    ARGS --cpp_out="${CMAKE_SOURCE_DIR}/proto"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/sharedmemory.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/rpcdb.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/rpcdb.pb.h"
    COMMAND "protoc"
    ARGS --cpp_out="${CMAKE_SOURCE_DIR}/proto"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/rpcdb.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/metrics.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/metrics.pb.h"
    COMMAND "protoc"
    ARGS --cpp_out="${CMAKE_SOURCE_DIR}/proto"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         "${CMAKE_SOURCE_DIR}/proto/metrics.proto"
  )

  add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/proto/vm.pb.cc"
           "${CMAKE_SOURCE_DIR}/proto/vm.pb.h"
    COMMAND "protoc"
    ARGS --cpp_out="${CMAKE_SOURCE_DIR}/proto"
         --proto_path="${CMAKE_SOURCE_DIR}/proto"
         --experimental_allow_proto3_optional
         "${CMAKE_SOURCE_DIR}/proto/vm.proto"
  )

  add_library(ProtoFiles
    "${CMAKE_SOURCE_DIR}/proto/vm.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/aliasreader.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/appsender.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/keystore.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/messenger.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/sharedmemory.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/rpcdb.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/metrics.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/vm.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/aliasreader.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/appsender.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/keystore.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/messenger.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/sharedmemory.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/rpcdb.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/metrics.pb.h"
  )

  # You HAVE to set the file names
  add_library (gen-grpc
    "${CMAKE_SOURCE_DIR}/proto/vm.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/aliasreader.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/appsender.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/keystore.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/messenger.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/sharedmemory.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/rpcdb.grpc.pb.cc"
    "${CMAKE_SOURCE_DIR}/proto/vm.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/aliasreader.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/appsender.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/keystore.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/messenger.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/sharedmemory.grpc.pb.h"
    "${CMAKE_SOURCE_DIR}/proto/rpcdb.grpc.pb.h"
    ${ProtoFiles}
  )

  target_link_libraries(gen-grpc PUBLIC ${Protobuf_LIBRARIES} ${GRPC_LIBRARIES} ${CARES_LIBRARY} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} absl::flags)

  add_library(orbitersdk_lib STATIC
    ${UTILS_HEADERS}
    ${UTILS_SOURCES}
    ${CONTRACT_HEADERS}
    ${CONTRACT_SOURCES}
    ${CORE_HEADERS}
    ${CORE_SOURCES}
    ${NET_HEADERS}
    ${NET_SOURCES}
  )

  add_dependencies(orbitersdk_lib gen-grpc ProtoFiles)

  target_include_directories(orbitersdk_lib PUBLIC ${CMAKE_SOURCE_DIR}/include ${OPENSSL_INCLUDE_DIR})

  target_link_libraries(orbitersdk_lib PRIVATE
    ${CRYPTOPP_LIBRARIES} ${SCRYPT_LIBRARY} Secp256k1 Ethash ${ETHASH_BYPRODUCTS} ${Protobuf_LIBRARIES}
    ${GRPC_LIBRARIES} ${CARES_LIBRARY} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} absl::flags
    Speedb ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES}
  )

  set_target_properties(orbitersdk_lib PROPERTIES COMPILE_FLAGS "-DAVALANCHEGO_COMPATIBLE=1")

  # Compile and link the executable
  add_executable(orbitersdkd "${CMAKE_SOURCE_DIR}/src/main.cpp")

  add_dependencies(orbitersdkd orbitersdk_lib gen-grpc ProtoFiles)
  target_include_directories(orbitersdkd PRIVATE orbitersdk_lib ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(orbitersdkd
    orbitersdk_lib
    ${Protobuf_LIBRARIES} ${GRPC_LIBRARIES} ${CARES_LIBRARY} Speedb
    ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES}
    absl::flags Secp256k1 Ethash ${ETHASH_BYPRODUCTS}
  )

  # Compile and link the ABI generator executable
  add_executable(contractabigenerator "${CMAKE_SOURCE_DIR}/src/main-contract-abi.cpp")

  add_dependencies(contractabigenerator orbitersdk_lib)
  target_include_directories(contractabigenerator PRIVATE orbitersdk_lib ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(contractabigenerator
    orbitersdk_lib Speedb ${SNAPPY_LIBRARY} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} Secp256k1 Ethash ${ETHASH_BYPRODUCTS}
  )

  # TODO: Implement tests for AvalancheGo compilation.
else()
  add_library(orbitersdk_lib STATIC
    ${UTILS_HEADERS}
    ${UTILS_SOURCES}
    ${CONTRACT_HEADERS}
    ${CONTRACT_SOURCES}
    ${CORE_HEADERS}
    ${CORE_SOURCES}
    ${NET_HEADERS}
    ${NET_SOURCES}
  )

  target_include_directories(orbitersdk_lib PRIVATE ${CMAKE_SOURCE_DIR}/include ${OPENSSL_INCLUDE_DIR})

  target_link_libraries(orbitersdk_lib PRIVATE
    ${CRYPTOPP_LIBRARIES} ${SCRYPT_LIBRARY} Secp256k1 Ethash ${ETHASH_BYPRODUCTS}
    Speedb ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES}
  )

  set_target_properties(orbitersdk_lib PROPERTIES COMPILE_FLAGS "-DAVALANCHEGO_COMPATIBLE=0")

  # Compile and link the executable
  add_executable(orbitersdkd "${CMAKE_SOURCE_DIR}/src/main.cpp")

  add_dependencies(orbitersdkd orbitersdk_lib)
  target_include_directories(orbitersdkd PRIVATE orbitersdk_lib ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(orbitersdkd
    orbitersdk_lib Speedb ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} Secp256k1 Ethash ${ETHASH_BYPRODUCTS}
  )

  # Compile and link the ABI generator executable
  add_executable(contractabigenerator "${CMAKE_SOURCE_DIR}/src/main-contract-abi.cpp")

  add_dependencies(contractabigenerator orbitersdk_lib)
  target_include_directories(contractabigenerator PRIVATE orbitersdk_lib ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(contractabigenerator
    orbitersdk_lib Speedb ${SNAPPY_LIBRARY} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} Secp256k1 Ethash ${ETHASH_BYPRODUCTS}
  )

  # Compile and link the ABI generator executable
  add_executable(networkdeployer "${CMAKE_SOURCE_DIR}/src/networkdeployer.cpp")

  add_dependencies(networkdeployer orbitersdk_lib)
  target_include_directories(networkdeployer PRIVATE orbitersdk_lib ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(networkdeployer
    orbitersdk_lib Speedb ${SNAPPY_LIBRARY} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} Secp256k1 Ethash ${ETHASH_BYPRODUCTS}
  )
endif()

# Compile and link the test executable if set to build it
if (BUILD_TESTS)
  add_executable(orbitersdkd-tests ${TESTS_HEADERS} ${TESTS_SOURCES})
  add_dependencies(orbitersdkd-tests orbitersdk_lib)
  target_include_directories(orbitersdkd-tests PRIVATE orbitersdk_lib  ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(orbitersdkd-tests
    orbitersdk_lib Speedb ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} Secp256k1 catch2 Ethash ${ETHASH_BYPRODUCTS}
  )
endif()

# Compile and link the Discovery Node test executable if set to build it
if (BUILD_DISCOVERY)
  add_executable(orbitersdkd-discovery "${CMAKE_SOURCE_DIR}/src/main-discovery.cpp")
  add_dependencies(orbitersdkd-discovery orbitersdk_lib)
  target_include_directories(orbitersdkd-discovery PRIVATE orbitersdk_lib ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(orbitersdkd-discovery
    orbitersdk_lib Speedb ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} Secp256k1 Ethash ${ETHASH_BYPRODUCTS}
  )
endif()

