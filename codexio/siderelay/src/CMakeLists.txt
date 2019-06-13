project(siderelay)

set(CODEX_WASM_OLD_BEHAVIOR "Off")
find_package(codex.cdt)

add_contract( siderelay siderelay worker.cpp bios_relay.cpp token_map.cpp chain.cpp siderelay.cpp )

target_include_directories( siderelay PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( siderelay ${CMAKE_SOURCE_DIR}/../ricardian )