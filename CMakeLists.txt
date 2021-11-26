cmake_minimum_required(VERSION 3.21)
project(try-ranges)

include(cmake/ProjectDefaults.cmake)

find_package(doctest REQUIRED)
enable_testing()
include(CTest)
include(doctest)

add_subdirectory(src)
