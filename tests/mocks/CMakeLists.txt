add_library(sdv_test_mocks INTERFACE)
target_include_directories(sdv_test_mocks INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sdv_test_mocks INTERFACE GTest::gmock)
