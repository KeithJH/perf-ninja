cmake_minimum_required(VERSION 3.9)

project(lab)

#include(CheckIPOSupported)
#check_ipo_supported()
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)
