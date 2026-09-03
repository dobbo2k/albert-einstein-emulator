add_executable(test_core test_core.cpp)
target_link_libraries(test_core PRIVATE ein_core)

add_test(NAME m0_acceptance
         COMMAND test_core "${EIN_ROMS_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/disks")

# zexdoc/zexall: the M1 acceptance criterion. These run for a while, so they
# are labelled "slow" -- exclude with: ctest -LE slow
add_executable(zex_runner zex_runner.cpp)
target_link_libraries(zex_runner PRIVATE ein_core)

add_test(NAME zexdoc COMMAND zex_runner "${CMAKE_CURRENT_SOURCE_DIR}/zex/zexdoc.com")
add_test(NAME zexall COMMAND zex_runner "${CMAKE_CURRENT_SOURCE_DIR}/zex/zexall.com")
set_tests_properties(zexdoc zexall PROPERTIES LABELS slow TIMEOUT 3600)

# M6 acceptance: ten emulated minutes of a game's music, checking the sample
# rate never sags and nothing is dropped. Slow, so excluded from the default run.
add_executable(soak_audio soak_audio.cpp)
target_link_libraries(soak_audio PRIVATE ein_core)

add_test(NAME audio_soak
         COMMAND soak_audio
                 "${EIN_ROMS_DIR}/Tatung X-TAL MOS v1.2 (1983)(Tatung).rom"
                 "${CMAKE_CURRENT_SOURCE_DIR}/disks/OHMUMMY.DSK" 10)
set_tests_properties(audio_soak PROPERTIES LABELS slow TIMEOUT 1800)
