include (GNUInstallDirs)
add_library(noticeboard
    ${CMAKE_CURRENT_SOURCE_DIR}/noticeboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/null_backend.cpp
)

set(noticeboard_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/noticeboard/backend.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/noticeboard/noticeboard.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/noticeboard/noticeboard.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/noticeboard/notify_send_backend.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/noticeboard/null_backend.hpp
)
set_target_properties(noticeboard PROPERTIES PUBLIC_HEADER "${noticeboard_headers}")
target_include_directories(noticeboard PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_features(noticeboard PRIVATE c_std_23)
target_compile_features(noticeboard PRIVATE cxx_std_23)
noticeboard_target_have_nullable(noticeboard PRIVATE NOTICEBOARD_HAVE_NULLABLE)
noticeboard_target_have_nonnull(noticeboard PRIVATE NOTICEBOARD_HAVE_NONNULL)
noticeboard_target_warnings(noticeboard PRIVATE)
noticeboard_target_set_system_options(noticeboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(
    TARGETS noticeboard
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIB_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIB_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)
