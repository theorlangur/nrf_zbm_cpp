cmake_minimum_required(VERSION 3.10)
project(NrfZBMCpp VERSION 1.0.0 LANGUAGES CXX)

add_library(NrfZBMCpp INTERFACE)

target_include_directories(NrfZBMCpp INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


