# SerialBridge


## Software


### Building SerialBridge

#### Required Software
- Boost installation or artifact for:
	- asio
	- system
	- program_options


#### Start build
	cd <path/to/this_repo>
	cd software
	mkdir build
	cd build
	cmake ../serialbridge
	make


### Running SerialBridge

#### Configuring the serial port

	USB-UART@96008N1 being mapped to localhost:25
	<BUILDDIR>$ ./SerialBridge -d /dev/ttyUSB0 -b 9600 -p 25

	Print options:
	<BUILDDIR>$ ./SerialBridge --help

	Print current config:
	<BUILDDIR>$ ./SerialBridge --config

##### Example usage

![Forwarding USB-UART via Laptop into Wifi network](/doc/tutorial/usage.jpg)

