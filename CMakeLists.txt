cmake_minimum_required(VERSION 3.10)
project(NrfZBMCpp VERSION 1.0.0 LANGUAGES CXX)

set(ZCL_SRC
    src/zcl/zbm_zcl_poll_ctrl_impl.cpp
)

add_library(NrfZBMCpp STATIC ${ZCL_SRC})
# add_library(NrfZBMCpp INTERFACE)

target_include_directories(NrfZBMCpp INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(NrfZBMCpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(NrfZBMCpp PUBLIC -freflection)

# Inherit Zephyr flags, Kconfig defines, and subsystem include paths (including ncs-zigbee)
if(TARGET zephyr_interface)
    target_link_libraries(NrfZBMCpp PRIVATE zephyr_interface)
endif()
