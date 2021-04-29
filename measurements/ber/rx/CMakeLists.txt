cmake_minimum_required(VERSION 3.8.2)

set(BOARD nrf5340dk_nrf5340_cpunet)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

target_include_directories(app PRIVATE
    include
    ../../../include
)
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
    ${app_sources}
)
