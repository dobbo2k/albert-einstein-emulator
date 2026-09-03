add_executable(albert WIN32
    main.cpp
    audio.cpp
    files.cpp
    input.cpp
    menu.cpp
    panels.cpp
    screen.cpp
    $<$<BOOL:${WIN32}>:albert.rc>
)

# The .rc names albert.ico by a bare relative path, and rc.exe resolves that
# against its include path rather than against the .rc file. Without this the
# resource compiler cannot find the icon.
target_include_directories(albert PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# rc.exe emits no depfile, so nothing otherwise tells the generator that
# albert.rc reads albert.ico. Without this, regenerating the icon leaves the
# stale .res in place and the build cheerfully relinks the old one -- it says
# "Build OK" and the executable keeps the previous face. Name the dependency.
if(WIN32)
    set_source_files_properties(albert.rc PROPERTIES
        OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/albert.ico")
endif()

target_link_libraries(albert PRIVATE ein_core imgui SDL3::SDL3)
target_compile_definitions(albert PRIVATE EIN_ROMS_DIR="${EIN_ROMS_DIR}")

if(MSVC)
    target_compile_options(albert PRIVATE /W4 /permissive-)
    # WIN32 above suppresses the console; keep main() as the entry point.
    set_target_properties(albert PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

# SDL3 is built as a DLL; put it next to the executable.
add_custom_command(TARGET albert POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:albert>
    VERBATIM
)
