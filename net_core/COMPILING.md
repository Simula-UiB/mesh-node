# Compilation instructions

### Setup

Follow the [Zephyr getting started guide](https://docs.zephyrproject.org/latest/getting_started/index.html), to get the `west` tool installed and to have a local copy of zephyr.

We use zephyr version 2.5.0. To select that version, do `git checkout v2.5.0` in the zephyr directory, and then `west update`.

### Compiling and flashing to nrf5340 Development Kit

First,

Connect nrf5340 DK board through the debug USB port (on the short edge).

In a terminal, setup the environment like this (assuming zephyrproject is in the home directory):

```bash
> source ~/zephyrproject/zephyr/zephyr-env.sh
```

Then, run from the net\_core directory:

```bash
> west build -b nrf5340dk_nrf5340_cpunet -p
> west flash
```

Once you have built with the specified the board once, you can just run `west flash` to build and flash in one step.

To see log output from the network core, keep a USB cable connected to the debug port, and run `screen /dev/ttyACM0 115200`. (If you have multiple serial devices connected, it might be ACM3, ACM6, and so on. Each nrf5340 board will show up with three serial ports).
