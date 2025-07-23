add_library(noticeboard
    ${CMAKE_CURRENT_SOURCE_DIR}/noticeboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/notify_send_backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/null_backend.cpp
)

set(headers
    ${CMAKE_CURRENT_SOURCE_DIR}/backend.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/noticeboard.h
    ${CMAKE_CURRENT_SOURCE_DIR}/noticeboard.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/notify_send_backend.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/null_backend.hpp
)
set_target_properties(noticeboard PROPERTIES PUBLIC_HEADER "${headers}")

target_compile_features(noticeboard PUBLIC c_std_23)
target_compile_features(noticeboard PUBLIC cxx_std_23)
target_have_nullable(noticeboard PUBLIC NB_HAVE_NULLABLE)
target_have_nonnull(noticeboard PUBLIC NB_HAVE_NONNULL)
target_warnings(noticeboard PUBLIC)

install(
    TARGETS noticeboard
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIB_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIB_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)
