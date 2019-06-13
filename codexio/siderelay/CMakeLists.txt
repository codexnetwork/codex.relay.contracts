include(ExternalProject)

cmake_minimum_required(VERSION 3.13)

# if no cdt root is given use default path
if(FORCEIO_CDT_ROOT STREQUAL "" OR NOT FORCEIO_CDT_ROOT)
   find_package(forceio.cdt)
endif()

ExternalProject_Add(
   siderelay_project
   SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
   BINARY_DIR ${CMAKE_BINARY_DIR}/siderelay
   CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${FORCEIO_CDT_ROOT}/lib/cmake/forceio.cdt/EosioWasmToolchain.cmake
   UPDATE_COMMAND ""
   PATCH_COMMAND ""
   TEST_COMMAND ""
   INSTALL_COMMAND ""
   BUILD_ALWAYS 1
)