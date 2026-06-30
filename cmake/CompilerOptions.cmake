add_library(sdv_compiler_options INTERFACE)

target_compile_options(sdv_compiler_options INTERFACE
    -Wall -Wextra -Wpedantic -Wshadow
    -Wno-unused-parameter
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2 -DNDEBUG>
)

if(ENABLE_SANITIZERS)
    target_compile_options(sdv_compiler_options INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(sdv_compiler_options INTERFACE
        -fsanitize=address,undefined
    )
endif()
