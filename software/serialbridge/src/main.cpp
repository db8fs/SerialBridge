/**
 *  @file   main.cpp
 *  @date   25.03.2023
 *  @author Falk Schilling (db8fs)
 *  @copyright GPLv3
 *  @remark migrated from SF/Github RGBLightControl repository
 */


#include <csignal>
#include <iostream>
#include <cstdlib>
#include <thread>

#include "Arguments.h"
#include "SerialPort.h"
#include "System.h"
#include "INetworkHandler.h"
#include "NetworkServer.h"

const char* HelloString = "SerialBridge\n\r";

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

public:
	SerialBridge(const Arguments& options)
		: options(options),
		serialPort(options.strDevice, options.uiBaudrate, SerialPort::eFlowControl::None),
		tcpServer(options.strAddress, options.port, options.strSSLCert)
	{
		serialPort.setHandler(this);
		tcpServer.setHandler(this);
	}

	bool isSerialAvailable() const
	{
		return serialConnected;
	}

	void waitForSerial(uint16_t waitDelayMs)
	{
		serialPort.awaitConnection(waitDelayMs);
	}

	void start()
	{
		serialPort.start();
	}

	void checkReadyness()
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

	void onSerialConnected() final
	{
		serialConnected = true;
		checkReadyness();
	}

	void onSerialReadComplete(const char* msg, size_t length) final
	{
		if (tcpConnected)
		{
			tcpServer.send(std::string(msg, msg + length));
		}
	}

    void onNetworkReadComplete(const char* msg, size_t length) final
	{
		if (tcpConnected)
		{
			serialPort.send((uint8_t*)msg, length);
		}
	}


	void onSerialWriteComplete(const char* msg, size_t length) final
	{
	}

    void onNetworkClientAccept() final
	{
		tcpConnected = true;

		std::cout << "Client Connect" << std::endl;

		checkReadyness();
	}

    void onNetworkClientDisconnect() final
	{
		tcpConnected = false;
		readySent = false;

		std::cout << "Client Disconnect" << std::endl;
	}


};


int main(int argc, char** argv)
{
    Arguments options;

	if (parseArguments(options, argc, argv))
	{
		try
		{
			SerialBridge bridge(options);

			while (!bridge.isSerialAvailable())
			{
				bridge.waitForSerial(4000);
			}

			bridge.start();

			System::run();
		}
		catch (const char* const text)
		{
			std::cout << ">>> FATAL: " << text << std::endl;
			std::cout << "================================" << std::endl;

			std::cout << std::endl << options;
		}
		catch (...)
		{
			std::cout << ">>> Aborting...";
		}
	}

  return EXIT_SUCCESS;
}

