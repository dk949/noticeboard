include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)

function(noticeboard_target_have_nullable target access definition)
    set(src "
    void *_Nonnull ${definition}(int * _Nullable ptr) {(void)ptr;}
    void * _Nullable global;
    struct Foo { void *_Nullable x; };

    int main(void) {
        (void)${definition}((int *)0);
        struct Foo foo = {};
        (void)foo;
        return 0;
    }")
    check_c_source_compiles("${src}" HAVE_C_NULLABLE_ATTRIBUTE)
    check_cxx_source_compiles("${src}" HAVE_CXX_NULLABLE_ATTRIBUTE)

    if(HAVE_C_NULLABLE_ATTRIBUTE AND HAVE_CXX_NULLABLE_ATTRIBUTE)
        target_compile_definitions(${target} ${access} ${definition})
    endif()
endfunction()




function(noticeboard_target_have_nonnull target access definition)
    set(src "
    void *_Nonnull ${definition}(int * _Nonnull ptr) {(void)ptr;}
    void * _Nonnull global;
    struct Foo { void *_Nonnull x; };

    int main(void) {
        (void)${definition}((int *)0);
        struct Foo foo = {};
        (void)foo;
        return 0;
    }")
    check_c_source_compiles("${src}" HAVE_C_NONNULL_ATTRIBUTE)
    check_cxx_source_compiles("${src}" HAVE_CXX_NONNULL_ATTRIBUTE)

    if(HAVE_C_NONNULL_ATTRIBUTE AND HAVE_CXX_NONNULL_ATTRIBUTE)
        target_compile_definitions(${target} ${access} ${definition})
    endif()
endfunction()
