function(noticeboard_target_warnings target access)
    macro (no_error warn)
        target_compile_options(${target} ${access} "-W${warn}" "-Wno-error=${warn}")
    endmacro()

    target_compile_options(${target} ${access}
        -Wall
        -Wextra
        -Wshadow
        -Wconversion
        -Werror
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
endfunction()
