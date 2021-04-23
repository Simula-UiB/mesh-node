# Example application for testing RTT latency

## Build and flash to the nrf5340 application core

```
> source /path/to/zephyr/zephyr-env.sh
> west build -p
> west flash
```

## Test setup
Flash one device as described above and flash a target device with the `ping` example.

In the serial console of the transmitter, use the command `send <mac> <packetsize>` to send a test packet. For example: `send e609b2 10` to send a 10 byte message to a receiver with mac `e609b2`.

In the serial console of the transmitter, the two-way latency will be printed on the format: `rtt: <latency in microseconds>`, for example: `rtt: 976` if the latency was 976μs.

# Automated test script

See [measure\_latency.py](measure_latency.py)