project(siderelay)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosforce.cdt)

add_contract( siderelay siderelay siderelay.cpp worker.cpp bios_relay.cpp token_map.cpp )

target_include_directories( siderelay PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( siderelay ${CMAKE_SOURCE_DIR}/../ricardian )