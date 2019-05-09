include(ExternalProject)
# if no cdt root is given use default path
if(EOSFORCE_CDT_ROOT STREQUAL "" OR NOT EOSFORCE_CDT_ROOT)
   find_package(eosforce.cdt)
endif()

ExternalProject_Add(
   siderelay_project
   SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
   BINARY_DIR ${CMAKE_BINARY_DIR}/siderelay
   CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSFORCE_CDT_ROOT}/lib/cmake/eosforce.cdt/EosioWasmToolchain.cmake
   UPDATE_COMMAND ""
   PATCH_COMMAND ""
   TEST_COMMAND ""
   INSTALL_COMMAND ""
   BUILD_ALWAYS 1
)