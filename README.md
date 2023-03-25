
*Build*
cd software
mkdir build
cd build
cmake ../serialbridge
make


*Run*
USB-UART@96008N1 being mapped to localhost:25
<BUILDDIR>$ ./SerialBridge -d /dev/ttyUSB0 -b 9600 -p 25

Print options:
<BUILDDIR>$ ./SerialBridge --help

Print current config:
<BUILDDIR>$ ./SerialBridge --config
