find_package(Doxygen QUIET COMPONENTS dot)

if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/docs/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation — open build/docs/html/index.html"
        VERBATIM
    )
else()
    message(STATUS "Doxygen not found — 'docs' target unavailable")
endif()
