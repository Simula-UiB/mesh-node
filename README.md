# Mesh node for the nrf5340

## Overview of intended stack

![overview](overview.png)

Implemented:

* Radio
* IPC

In progress:

* Mesh access layer (See PR #2)
* Packet processing layer
* Coding layer


## Directory structure

* net\_core
    * Contains mesh node running on network core
* mesh\_access
    * Contains zephyr module for accessing mesh node from application layer
* example
    * Contains example applications utilizing mesh node
