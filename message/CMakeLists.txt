zephyr_library()

zephyr_include_directories(
    include
    ../include
    )
zephyr_library_sources(src/message.c)
