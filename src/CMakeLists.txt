include(${CMAKE_SOURCE_DIR}/cmake/check_features.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/warnings.cmake)

add_library(noticeboard
    ${CMAKE_CURRENT_SOURCE_DIR}/noticeboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/notify_send_backend.cpp
)

target_compile_features(noticeboard PUBLIC c_std_23)
target_compile_features(noticeboard PUBLIC cxx_std_23)
target_have_nullable(noticeboard PUBLIC NB_HAVE_NULLABLE)
target_have_nonnull(noticeboard PUBLIC NB_HAVE_NONNULL)
target_warnings(noticeboard PUBLIC)
