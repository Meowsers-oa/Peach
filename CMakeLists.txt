cmake_minimum_required(VERSION 3.20)
project(peach LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)

add_subdirectory(external)
add_subdirectory(Peach)
add_subdirectory(sandbox)
