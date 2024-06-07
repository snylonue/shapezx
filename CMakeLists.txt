cmake_minimum_required(VERSION 3.27)

project(shapezx VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig)
pkg_check_modules(GTKMM_VARS REQUIRED IMPORTED_TARGET gtkmm-4.0)

add_executable(${PROJECT_NAME} src/main.cpp src/core/core.cpp src/core/machine.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::GTKMM_VARS)
# if (WIN32)
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}> COMMAND_EXPAND_LISTS)
# endif()