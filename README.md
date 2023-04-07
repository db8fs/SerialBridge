# SerialBridge

## Introduction

#### What's serial communication?

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

#### What is a serial bridge?

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

#### What does this this project try to achieve?

This project contains a C++ application that will create a network socket for
a given USB serialport interface (telnet-like). Other Wifi participants may connect to this
socket and operate on it, just as they would with a traditional serial port - except that 
the communication settings (device (COM9), baudrate (115200)) will be applied by
the serial bridge server.

#### Can I put this on Arduino? 

Nope, most likely not. Let's make it clear: let's say, you want to use an USB serial interface
and want to stream its data via network. For doing so, your Arduino would have to be the USB host, speaking the
USB protocol to the USB serialport client (usb-host). Then you would also need a specific driver
for the attached serialport device (usb-serial) and a Wifi connection via ESP32.

It's therefore way easier to simply use a RaspberryPi Zero W with an Raspbian image (or custom buildroot)
and use the C++ project below.

#### Did I get it right? I need a RaspberryPi with USB OTG enabled and usb-serial driver?

Exactly, that's how it works. You attach your USB-UART to your Raspberry, start the SerialBridge server 
and you then may connect via WiFi or Ethernet to the server. 

You can then access the USB-UART from the network client computer, and receive or transmit data to it.

#### Are there ways to simulate a physical COM interface for the network client?

Windows or Unix software usually access serial ports via DeviceIO - for the OS
it is like accessing a specific file in the filesystem.

For being able to connect to the serial server (the "SerialBridge") via network,
you will need to have an emulation layer, most likely a virtual interface driver, 
that will fake an COM port on your Windows OS. For the client application it will 
then look, as if the device is physically attached to the computer.

For Windows there are projects available, that can provide a kernel driver for
these purposes. For further details, just have a look at
[Com0Com](https://com0com.sourceforge.net/).

#### Isn't this possible using linux and minicom or screen?

Sure, feel free to do so and share it! This approach here tries to be customizable for 
your own needs - and you may do so, but be warned, that changes on the software must 
be applied under GPLv3, which shouldn't be a bigger problem these days.

#### What's the License for SerialBridge?

As already mentioned above, the source code is licensed under the GNU GPLv3 as
defined [here](https://www.gnu.org/licenses/gpl-3.0.html).


## System

### RaspberryPi/Raspbian

For getting started, simply install your favorite Raspbian image onto the SDCard
and get it running to connect to your WiFi or Ethernet network. This process of
how to get there is very well documented at
[RaspberryPi.com](https://www.raspberrypi.com/documentation/computers/getting-started.html#setting-up-your-raspberry-pi).

When your network is running, simply install cmake and libboost-dev using the
apt package system and build the software as described below.


## Software

### Building SerialBridge

#### Required Software 
- Boost installation or artifact for: 
	- asio (serialization)
	- system -
	- program_options 
- CMake 
- Git
- Compiler suite (GCC, Clang, MSVC)


#### Start build

For Raspian you may want to start building using the following steps:

	$ git clone https://github.com/db8fs/SerialBridge.git
	$ cd SerialBridge/software 
	$ mkdir build 
	$ cd build 
	$ cmake ../serialbridge
	$ make

This should usually run fine, as long as the software dependencies for building are matched. 
The result an existing 'SerialBridge' binary in your build folder.

### Running SerialBridge

#### Configuring the serial port and running the server

	USB-UART@96008N1 being mapped to localhost:25 
	  <BUILDDIR>$ ./SerialBridge -d /dev/ttyUSB0 -b 9600 -p 25

	Print options: 
	  <BUILDDIR>$ ./SerialBridge --help

	Print current config: 
	  <BUILDDIR>$ ./SerialBridge --config


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
