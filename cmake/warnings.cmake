function(noticeboard_target_warnings target access)
    if (NOT ((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")))
        return()
    endif()
    macro (no_error warn)
        target_compile_options(${target} ${access} "-W${warn}" "-Wno-error=${warn}")
    endmacro()
    macro(clang_warn)
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${target} ${access} ${ARGV})
        endif()
    endmacro()
    macro(gcc_warn)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${target} ${access} ${ARGV})
        endif()
    endmacro()

    target_compile_options(${target} ${access}
        -Wall
        -Wextra
        -Wshadow
        -Wconversion
        -Werror
        -Wcast-align
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wmisleading-indentation
        -Wnon-virtual-dtor
        -Wnull-dereference
        -Woverloaded-virtual
        -Wpedantic
        -Wsign-conversion
        -Wsuggest-override
        -Wswitch-enum
        -Wunused
    )

    no_error(unused-but-set-parameter)
    no_error(unused-but-set-variable)
    no_error(unused-const-variable)
    no_error(unused-function)
    no_error(unused-label)
    no_error(unused-local-typedefs)
    no_error(unused-macros)
    no_error(unused-parameter)
    no_error(unused-variable)

    gcc_warn(
        -Wunused-const-variable=1
        -Wduplicated-branches
        -Wduplicated-cond
        -Wlogical-op
        -Wnoexcept
        -Wuseless-cast
    )
    clang_warn(-Wno-nullability-extension)
endfunction()
