cmake_minimum_required(VERSION 3.27)

project(shapezx VERSION 0.1 LANGUAGES CXX)

find_package(fltk CONFIG REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE fltk)