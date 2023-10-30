#include "SerialBridge.h"

#include <iostream>

static const char* HelloString = "SerialBridge\n\r";

SerialBridge::SerialBridge(const Arguments& options)
    : options(options),
    serialPort(options.strDevice, options.uiBaudrate, SerialPort::eFlowControl::None),
    tcpServer(options.strAddress, options.port, options.strSSLCert)
{
    serialPort.setHandler(this);
    tcpServer.setHandler(this);
}

bool SerialBridge::isSerialAvailable() const
{
    return serialConnected;
}

void SerialBridge::waitForSerial(uint16_t waitDelayMs)
{
    serialPort.awaitConnection(waitDelayMs);
}

void SerialBridge::start()
{
    serialPort.start();
}

void SerialBridge::checkReadyness()
{
    if (tcpConnected && serialConnected)
    {
        std::cout << "TCP + Serial ready" << std::endl;
        if (!readySent)
        {
            tcpServer.send(HelloString);
            readySent = true;
        }
    }
}

void SerialBridge::onSerialConnected()
{
    serialConnected = true;
    checkReadyness();
}

void SerialBridge::onSerialReadComplete(const char* msg, size_t length)
{
    if (tcpConnected)
    {
        tcpServer.send(std::string(msg, msg + length));
    }
}

void SerialBridge::onNetworkReadComplete(const char* msg, size_t length)
{
    if (tcpConnected)
    {
        serialPort.send((uint8_t*)msg, length);
    }
}


void SerialBridge::onSerialWriteComplete(const char* msg, size_t length)
{
}

void SerialBridge::onNetworkClientAccept()
{
    tcpConnected = true;

    std::cout << "Client Connect" << std::endl;

    checkReadyness();
}

void SerialBridge::onNetworkClientDisconnect()
{
    tcpConnected = false;
    readySent = false;

    std::cout << "Client Disconnect" << std::endl;
}
