# noticeboard

[![CMake build and test](https://github.com/dk949/noticeboard/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/dk949/noticeboard/actions/workflows/cmake-multi-platform.yml)

A library for sending system notifications on Linux.

Provides C and C++ bindings.


## Installing

The simplest way to add `noticeboard` to your project is with cmake's
[`FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html),
like so:

```cmake
set(FETCHCONTENT_BASE_DIR "path/to/dependencies") # optionally set dependency source directory
FetchContent_Declare(
    noticeboard
    GIT_REPOSITORY https://github.com/dk949/noticeboard/
    GIT_TAG trunk
)
FetchContent_MakeAvailable(noticeboard)
target_link_libraries(my_target PRIVATE noticeboard)
```

This will download, build and link `noticeboard` with `my_target`.
