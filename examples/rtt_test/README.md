# Example application for testing RTT latency

## Build and flash to the nrf5340 application core

```
> source /path/to/zephyr/zephyr-env.sh
> west build -p
> west flash
```

## Test setup
Flash this device as described above and flash the target node with the `ping` example.