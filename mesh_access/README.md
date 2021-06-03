# Zephyr module for mesh node access layer

This module implements communication with the mesh node running on the network core, and is intended to be used by applications running on the application core of the nrf5340.

## Usage

See the [app\_core\_demo](../examples/app_core_demo) example for reference. 

1. Add the mesh\_access directory to the ZEPHYR\_EXTRA\_MODULES list in CMakeLists.txt
1. `#import <mesh.h>` in your application
1. Implement the `mesh_receive` callback function in your application, which will be called when receiving a message
1. Use the `mesh_send` or `mesh_send_broadcast` functions to send mesh messages.
1. Set `CONFIG_MESH_ACCESS_MODULE=y` and `CONFIG_HEAP_MEM_POOL_SIZE` in your prj.conf
