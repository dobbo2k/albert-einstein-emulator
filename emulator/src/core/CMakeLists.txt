# Emulator core: no SDL, no ImGui, no platform dependencies.
# The headless test harness (and the M1 zexdoc/zexall runner) links this alone.
add_library(ein_core STATIC
    md5.cpp
    rom.cpp
    machine.cpp
    z80.cpp
    tms9129.cpp
    z80ctc.cpp
    keyboard.cpp
    ay38910.cpp
    fd1770.cpp
    dskimage.cpp
    z80pio.cpp
    i8251.cpp
    adc0844.cpp
    disasm.cpp
    symbols.cpp
    card80.cpp
)

target_include_directories(ein_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(ein_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(ein_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(ein_core PRIVATE -Wall -Wextra)
endif()
