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

1. Connect USB-UART to bridging computer (could be also RaspberryPi)
![Forwarding USB-UART via Laptop into Wifi network](/doc/tutorial/usage.jpg)

2. Start the SerialBridge server for the given USB-UART
![HyperTerminal TCP/IP Connect Dialog](/doc/tutorial/server-start.jpg)
	
3. Connect your network client to the server
![HyperTerminal TCP/IP Connect Dialog](/doc/tutorial/hypertrm-connect-dlg.png)

4. Get the welcome message of the SerialBridge server
![HyperTerminal acknowledged connection to SerialBridge](/doc/tutorial/hypertrm-connected.png)

5. Ready to go: Transceive data!
![HyperTerminal receiving data from the UART via network](/doc/tutorial/hypertrm-receiving-data.png)
