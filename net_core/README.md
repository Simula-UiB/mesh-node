# Custom mesh node for the nrf5340 network core

## Overview

**TODO**

## Compiling

See [COMPILING.md](COMPILING.md).

## Logging

To get log output from the network core, connect the following pins on the nrf5340 PDK with jumper wires:

* RTS to P0.10
* CTS to P0.12
* RxD to P0.25
* TxD to P0.26

Log output from the network will generally go to `/dev/ttyACM1`, and app core to `/dev/ttyACM2`. To open a serial port on linux do:

```
screen /dev/ttyACM1 115200
```

## Documentation for libraries

- [Zephyr](https://docs.zephyrproject.org/latest/index.html)
- [nrfx](https://infocenter.nordicsemi.com/topic/struct_drivers/struct/nrfx_latest.html) (Nordic specific drivers/hardware access layers)

## Structure

C source files are in the [src](src) directory.

C header files are in the [include](include) directory.

## Configuration

[prj.conf](prj.conf) sets Zephyr configuration options.

Application-specific configurations are in [include/common.h](include/common.h)
