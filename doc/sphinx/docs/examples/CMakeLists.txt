cmake_minimum_required(VERSION 3.20)
project(getting_started_example LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find pagmo and the plugins package installed by this project.
find_package(pagmo REQUIRED)
find_package(pagmo_plugins_nonfree REQUIRED)

add_executable(getting_started getting_started.cpp)

# Link the pagmo_plugins_nonfree exported target and pagmo
target_link_libraries(getting_started PRIVATE Pagmo_plugins_nonfree::pagmo_plugins_nonfree Pagmo::pagmo)
