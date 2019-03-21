project(side.relay)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( side.relay side.relay side.relay.cpp )
target_include_directories( side.relay PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( side.relay ${CMAKE_SOURCE_DIR}/../ricardian )