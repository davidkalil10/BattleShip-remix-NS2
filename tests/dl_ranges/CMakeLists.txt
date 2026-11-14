# Headless unit tests of the display list range registry (port/port_dl_ranges.cpp) and of the walk-off rule the
# interpreter applies to its answers (libultraship fast/dl_walk_bounds.h): no ROM, no window, no libultraship build.
# One command: cmake --build <build> --target run_dl_ranges_tests
add_executable(dl_ranges_tests EXCLUDE_FROM_ALL
    test_dl_ranges.cpp
    ${CMAKE_SOURCE_DIR}/port/port_dl_ranges.cpp
)
target_include_directories(dl_ranges_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stub ${CMAKE_SOURCE_DIR}/libultraship/src ${CMAKE_SOURCE_DIR}/port)
set_target_properties(dl_ranges_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

add_custom_target(run_dl_ranges_tests
    COMMAND dl_ranges_tests
    DEPENDS dl_ranges_tests
    COMMENT "Running the display list range registry unit tests"
)
