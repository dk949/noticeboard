cmake_minimum_required(VERSION 3.10)
project(noticeboard LANGUAGES C CXX VERSION 0.1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_INSTALL_INCLUDE_DIR include/noticeboard)
set(CMAKE_INSTALL_LIB_DIR lib)

include(cmake/check_features.cmake)
include(cmake/warnings.cmake)
add_subdirectory(src)
