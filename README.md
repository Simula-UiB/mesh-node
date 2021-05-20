# Mesh node for the nrf5340

## Introduction

This is a Zephyr-based implementation of a wireless mesh node for the nRF5340 SoC. Features include:

- < 1ms latency for single hop transmissions
- Routing using network flooding
- Broadcast and unicast messages
- Up to 243 byte packet payloads
- 2Mbit/s Bluetooth LE-based physical layer
- Mesh node and application running on separate CPU cores

This project provides the ability to do ad hoc mesh networking with less overhead than the 
popular mesh implementations. The routing technique is stateless, meaning each node is unaware 
of the current structure of the network. This provides a lot of flexibility, as adding or removing
nodes from the network does not require any updates to be propagated to other nodes. This is a
similar approach to Bluetooth Mesh, however we have much more freedom when it comes to packet size
and transmission intervals, and we achieve better latency performance than the Bluetooth Mesh 
implementations we have tested.

The mesh node is limited in features, which means a lot of control is given to the application.
Packets are delivered on a best-effort basis, and any guarantee of delivery must be handled by 
the application layer. The application layer is provided with functions for sending messages as
well as callback functions for receiving. The relative simplicity of the project also allows for
modifications to be reasonably simple, such as changing the physical layer protocol, porting to
other devices etc.

The project is based on the [Zephyr RTOS](https://github.com/zephyrproject-rtos/zephyr), which 
allows applications to use features such as threads, queues, semaphores, a logging framework, and more.

## Overview of mesh node stack

![overview](overview.png)
