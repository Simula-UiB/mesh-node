# Low latency packet transport for BLE Radio on the nRF5340

Consisting of a packet transport layer running on the network core, accessible from applications running application core, through a simple message passing interface. Packets consist of 12 byte headers, with up to 243 bytes available for the application.
By default the packet transport layer uses the 2Mbit/s BLE physical layer.
The latency associated with sending a message generated at the application layer trough the packet transport layer and over the wireless channel to the application layer ofthe receiving node is less than 1 ms. 
The corresponding latency for Thread or Bluetooth Mesh is about 5 ms; see below.

In addition the packet transport layer provides mesh networking via a network flooding approach, i.e. packets received by a particular node that are addressed to another node are rebroadcasted. Packets are addressed based on a node-specific identifier. This process is transparent to the application. 
The packet transport layer does not provide reliable transport, applications must be tolerant of packet erasures and errors. 

The project is implemented in the [Zephyr RTOS](https://github.com/zephyrproject-rtos/zephyr).

## Block diagram

![overview](overview.png)
