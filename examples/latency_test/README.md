# Example application for testing the latency of messages

## Build and flash to the nrf5340 application core

```
> source /path/to/zephyr/zephyr-env.sh
> west build -b nrf5340dk_nrf5340_cpuapp -p
> west flash
```

# Test setup

To use, flash two boards, and connect a GPIO line from pin 28 at the transmitter to pin 29 at the receiver. 

In the serial console of the transmitter, use the command `send <packetsize>` to send a test packet. For example: `send 10` to send a 10 byte message.

In the serial console of the receiver, the one-way latency will be printed on the format: `latency: <latency in microseconds>`, for example: `latency: 1202` if the latency was 1202μs.

# Automated test script

See [measure\_latency.py](measure_latency.py)
