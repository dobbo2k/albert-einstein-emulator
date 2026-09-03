# Assemble a self-contained folder and zip it.
#
#   cmake --build build --target dist
#
# The target is called "dist" rather than "package" because CPack claims that
# name, and a clash there produces a confusing failure rather than an error.
#
# What goes in is deliberately the minimum that runs on a machine that has
# never seen this project: the executable, SDL3.dll, and a roms folder beside
# them. Nothing else is needed -- the C runtime is linked statically, and the
# settings and layout files are written on first run rather than shipped, so a
# release does not impose one person's window arrangement on everyone.

set(EIN_DIST_NAME "albert-windows-x64" CACHE STRING "Base name for the release archive")

# ROM images and the disassembly are third-party: the MOS ROM is Tatung's
# firmware and the commented sources are Ric Hohne's. Including them is a
# decision rather than a default, so it is a switch you can see.
option(EIN_DIST_ROMS "Include ROM images and the commented disassembly" ON)

set(_dist_root "${CMAKE_BINARY_DIR}/dist")
set(_dist_dir "${_dist_root}/albert")
set(_mos12 "disassembled/Einstein/__MOS12")

set(_dist_commands
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${_dist_root}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_dist_dir}"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:albert> "${_dist_dir}/"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SDL3::SDL3> "${_dist_dir}/"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/dist/READ-ME-FIRST.txt"
            "${_dist_dir}/"
)

if(EIN_DIST_ROMS)
    # Only the four sources the debugger actually reads, not the whole archive.
    list(APPEND _dist_commands
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_dist_dir}/roms/${_mos12}"
        # Shipped under the names Albert looks for: einstein.rom is the
        # default, and both are .rom so the content scan can find either.
        COMMAND ${CMAKE_COMMAND} -E copy
                "${EIN_ROMS_DIR}/Tatung X-TAL MOS v1.2 (1983)(Tatung).rom"
                "${_dist_dir}/roms/einstein.rom"
        COMMAND ${CMAKE_COMMAND} -E copy
                "${EIN_ROMS_DIR}/einstein1.21.bin"
                "${_dist_dir}/roms/einstein-1.21.rom"
        COMMAND ${CMAKE_COMMAND} -E copy
                "${EIN_ROMS_DIR}/${_mos12}/Mos12.zsm"
                "${EIN_ROMS_DIR}/${_mos12}/UPPERMEM.ZSM"
                "${EIN_ROMS_DIR}/${_mos12}/Einstein.zsm"
                "${EIN_ROMS_DIR}/${_mos12}/MOSEQU.GEN"
                "${_dist_dir}/roms/${_mos12}/"
    )
else()
    list(APPEND _dist_commands
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_dist_dir}/roms"
    )
endif()

list(APPEND _dist_commands
    COMMAND ${CMAKE_COMMAND} -E chdir "${_dist_root}"
            ${CMAKE_COMMAND} -E tar cf "${EIN_DIST_NAME}.zip" --format=zip albert
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "  ${_dist_root}/${EIN_DIST_NAME}.zip"
)

if(EIN_DIST_ROMS)
    list(APPEND _dist_commands
        COMMAND ${CMAKE_COMMAND} -E echo
                "  includes ROM images and disassembly -- both third-party; -DEIN_DIST_ROMS=OFF omits them"
    )
else()
    list(APPEND _dist_commands
        COMMAND ${CMAKE_COMMAND} -E echo "  no ROMs included; the user supplies their own"
    )
endif()

add_custom_target(dist
    ${_dist_commands}
    DEPENDS albert
    VERBATIM
    COMMENT "Assembling ${EIN_DIST_NAME}.zip"
)
