#ifndef SERIALBRIDGE_H_
#define SERIALBRIDGE_H_

#include "INetworkHandler.h"

#include "Arguments.h"
#include "SerialPort.h"
#include "NetworkServer.h"

/* creates a tcp server socket for bridging serial UART data into a tcp network */
class SerialBridge :	private SerialPort::ISerialHandler,
                        private INetworkHandler
{
    Arguments  options;
    SerialPort serialPort;
    NetworkServer  tcpServer;

    bool serialConnected = false;
    bool tcpConnected = false;

    bool readySent = false;

    void checkReadyness();

    /* serial event handling */
    void onSerialConnected() final;
    void onSerialReadComplete(const char* msg, size_t length) final;
    void onSerialWriteComplete(const char* msg, size_t length) final;

    /* network event handling*/
    void onNetworkReadComplete(const char* msg, size_t length) final;
    void onNetworkClientAccept() final;
    void onNetworkClientDisconnect() final;

public:
    SerialBridge(const Arguments& options);

    bool isSerialAvailable() const;

    void waitForSerial(uint16_t waitDelayMs);

    void start();
};


#endif
