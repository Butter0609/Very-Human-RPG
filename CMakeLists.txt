cmake_minimum_required(VERSION 3.16)
project(MyGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

add_executable(MyGame src/main.cpp)
target_link_libraries(MyGame PRIVATE SDL3::SDL3)