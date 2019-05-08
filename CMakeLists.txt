cmake_minimum_required(VERSION 3.5)
project(relay_contracts)
include(ExternalProject)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   set(TEST_BUILD_TYPE "Debug")
   set(CMAKE_BUILD_TYPE "Release")
else()
   set(TEST_BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif()

find_package(eosio.cdt)
ExternalProject_Add(
   eos_siderelay_project
   SOURCE_DIR ${CMAKE_SOURCE_DIR}/eos/siderelay/src
   BINARY_DIR ${CMAKE_BINARY_DIR}/eos/siderelay
   CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
   UPDATE_COMMAND ""
   PATCH_COMMAND ""
   TEST_COMMAND ""
   INSTALL_COMMAND ""
   BUILD_ALWAYS 1
)

find_package(forceio.cdt)
ExternalProject_Add(
   forceio_siderelay_project
   SOURCE_DIR ${CMAKE_SOURCE_DIR}/forceio/siderelay/src
   BINARY_DIR ${CMAKE_BINARY_DIR}/forceio/siderelay
   CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${FORCEIO_CDT_ROOT}/lib/cmake/forceio.cdt/EosioWasmToolchain.cmake
   UPDATE_COMMAND ""
   PATCH_COMMAND ""
   TEST_COMMAND ""
   INSTALL_COMMAND ""
   BUILD_ALWAYS 1
)

find_package(eosforce.cdt)
ExternalProject_Add(
   eosforce_siderelay_project
   SOURCE_DIR ${CMAKE_SOURCE_DIR}/eosforce/siderelay/src
   BINARY_DIR ${CMAKE_BINARY_DIR}/eosforce/siderelay
   CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSFORCE_CDT_ROOT}/lib/cmake/eosforce.cdt/EosioWasmToolchain.cmake
   UPDATE_COMMAND ""
   PATCH_COMMAND ""
   TEST_COMMAND ""
   INSTALL_COMMAND ""
   BUILD_ALWAYS 1
)