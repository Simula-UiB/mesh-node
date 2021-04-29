# Example application for ping
If the node receives a message that isn't broadcast it will reply to the sender with the same message.

## Build and flash to the nrf5340 application core

```
> source /path/to/zephyr/zephyr-env.sh
> west build -p
> west flash
```
