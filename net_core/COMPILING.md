# Compilation instructions

### Setup

Follow the setup guide for the nRF Connect SDK. Either:
- Install through [nRF Connect for Desktop](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_assistant.html). (I didn't succeed with this method, YMMV).
- Install [manually](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html).

You should end up with a directory named `ncs` with subdirectories like `nrf`, `zephyr`, `modules` etc. And the `west` command line tool should be installed.

In `ncs/nrf` checkout version 1.4.0, and update with west:

```
> git fetch
> git checkout v1.4.0
> west update
```

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
