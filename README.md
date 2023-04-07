# SerialBridge

## Introduction

### What's serial communication?

Serial ports have played a significant role in the computer industry over the
years, although their importance has diminished in recent times with the rise of
newer technologies such as USB, Ethernet, and Bluetooth.

A serial port is a type of computer interface that allows data to be transferred
between a computer and a device, one bit at a time. This method of communication
is relatively slow compared to modern interfaces, but it is reliable and can be
used over long distances.

In the past, serial ports were commonly used to connect devices such as
printers, scanners, and modems to computers. They were also used for
communication between computers, such as for file transfer and remote access.

## What is a serial bridge?

A serial to WiFi bridge, also known as a serial to wireless converter or a
serial to WiFi adapter, is a device that allows serial devices to communicate
over a wireless network. It essentially creates a wireless connection between a
serial device and a WiFi network, enabling the device to be accessed and
controlled remotely.

To use a serial to WiFi bridge, the device is connected to a serial port on the
serial device (such as a sensor, controller, or other device) and then connected
to a WiFi network. This allows the device to transmit data wirelessly,
eliminating the need for a physical connection between the device and a computer
or other controller.

## What does this this project try to achieve?

This project contains a C++ application that will create a network socket for
a given serialport interface (telnet-like). Other Wifi participants may connect to this
socket and operate on it, just as they would with a traditional serial port - except that 
the communication settings (device (COM9), baudrate (115200)) will be applied by
the serial bridge server.


## Isn't it possible to do the same using linux and minicom or screen?

Sure, feel free to do so and share it! This approach here tries to be customizable for 
your own needs - and you may do so, but be warned, that changes on the software must 
be applied under GPLv3, which shouldn't be a bigger problem these days.


## System

### RaspberryPi/Raspbian

For getting started, simply install your favorite Raspbian image onto the SDCard and get it running to connect to your WiFi or Ethernet network.
This process of how to get there is very well documented at [RaspberryPi.com](https://www.raspberrypi.com/documentation/computers/getting-started.html#setting-up-your-raspberry-pi).

When your network is running, simply install cmake and libboost-dev using the apt package system and build 
the software as described below.


## Software

### Building SerialBridge

#### Required Software 
- Boost installation or artifact for: 
	- asio (serialization)
	- system -
	- program_options 
- CMake 
- Compiler suite (GCC, Clang, MSVC)


#### Start build

	cd <path/to/this_repo> cd software mkdir build cd build cmake ../serialbridge
	make


### Running SerialBridge

#### Configuring the serial port

	USB-UART@96008N1 being mapped to localhost:25 <BUILDDIR>$ ./SerialBridge -d
	/dev/ttyUSB0 -b 9600 -p 25

	Print options: <BUILDDIR>$ ./SerialBridge --help

	Print current config: <BUILDDIR>$ ./SerialBridge --config

##### Example usages

###### Bridging a Waterrower S4 rowing computer into WiFi network

1. Connect USB-UART to bridging computer (could be also RaspberryPi 3 or 0W)
![Forwarding USB-UART via Laptop into Wifi network](/doc/tutorial/usage.jpg)

2. Start the SerialBridge server for the given USB-UART ![HyperTerminal TCP/IP
Connect Dialog](/doc/tutorial/server-start.jpg)

3. Connect your network client to the server ![HyperTerminal TCP/IP Connect
Dialog](/doc/tutorial/hypertrm-connect-dlg.png)

4. Get the welcome message of the SerialBridge server ![HyperTerminal
acknowledged connection to SerialBridge](/doc/tutorial/hypertrm-connected.png)

5. Ready to go: Transceive data! ![HyperTerminal receiving data from the UART
via network](/doc/tutorial/hypertrm-receiving-data.png)
