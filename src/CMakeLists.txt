file(GLOB_RECURSE SOURCES
    *.h
    *.cpp
)

add_library(CONBus ${SOURCES})

target_include_directories(CONBus
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)