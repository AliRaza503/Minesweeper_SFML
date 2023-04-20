cmake_minimum_required(VERSION 3.23)
project(Minesweeper)

set(CMAKE_CXX_STANDARD 11)
set(SFML_INCLUDE_DIR "../SFML-2.5.1/include")
set(SFML_LIBRARY_DIR "../SFML-2.5.1/build/lib")

link_directories(SFML_LIBRARY_DIR)
include_directories(SFML_INCLUDE_DIR)

find_package(SFML 2.5.1 COMPONENTS system window graphics network audio)

add_executable(Minesweeper main.cpp)

target_link_libraries (Minesweeper sfml-graphics sfml-window sfml-system)
