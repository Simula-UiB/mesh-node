# Custom mesh node for the nrf5340 network core

## Compiling

See [COMPILING.md](COMPILING.md).

## Logging

Log output from the network core will generally go to `/dev/ttyACM0`, and application core to `/dev/ttyACM2`. To open a serial port on linux do:

```
screen /dev/ttyACM0 115200
```

## Documentation for libraries

- [Zephyr](https://docs.zephyrproject.org/latest/index.html)
- [nrfx](https://infocenter.nordicsemi.com/topic/struct_drivers/struct/nrfx_latest.html) (Nordic specific drivers/hardware access layers)

## Structure

C source files are in the [src](src) directory.

C header files are in the [include](include) directory.

## Configuration

[prj.conf](prj.conf) sets Zephyr configuration options.

The global log level is set in the common header file: [../include/common.h](../include/common.h)
