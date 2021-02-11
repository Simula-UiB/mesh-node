cmake_minimum_required(VERSION 3.8.2)

set(BOARD nrf5340pdk_nrf5340_cpuapp)

list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_SOURCE_DIR}/../../mesh_access/)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

target_sources(app PRIVATE
    src/main.c
)
