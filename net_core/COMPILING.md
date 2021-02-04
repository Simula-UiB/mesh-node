# Compilation instructions

### Setup

Follow the [Zephyr getting started guide](https://docs.zephyrproject.org/latest/getting_started/index.html), to get the `west` tool installed and to have a local copy of zephyr.

Make sure zephyr is on the master branch or at least version >= 2.5.0.

### Compiling and flashing to nrf5340 Development Kit

First,

Connect nrf5340 PDK board through the debug USB port (on the short edge).

In a terminal, setup the environment like this (assuming ncs is in the home directory):

```bash
> source ~/ncs/zephyr/zephyr-env.sh
```

Then, run from the net\_core directory:

```bash
> west build -b nrf5340pdk_nrf5340_cpunet --pristine
> west flash
```

Once you have built with the specified the board once, you can just run `west flash` to build and flash in one step.

To see log output from the board, keep a USB cable connected to the debug port, and run `screen /dev/ttyACM0 115200`. (If you have multiple serial devices connected, it might be ACM1, ACM2, and so on).
