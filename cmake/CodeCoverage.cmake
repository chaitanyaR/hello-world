find_program(GCOV gcov REQUIRED)
find_program(LCOV lcov)
find_program(GENHTML genhtml)

add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
add_link_options(--coverage)

if(LCOV AND GENHTML)
    add_custom_target(coverage
        COMMAND ${LCOV} --capture --directory . --output-file coverage.info
                        --exclude '*/tests/*' --exclude '*/build/*' --exclude '/usr/*'
        COMMAND ${GENHTML} coverage.info --output-directory coverage_report
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating HTML coverage report in build/coverage_report/"
    )
endif()
