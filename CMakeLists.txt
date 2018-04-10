cmake_minimum_required(VERSION 3.10)
project(hw4)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(hw4
        hw4.cc
        Log.cc
        Log.h
        SetupData.cc
        SetupData.h)
