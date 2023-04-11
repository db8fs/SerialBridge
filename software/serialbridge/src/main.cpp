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
#include "TCPServer.h"

const char* HelloString = "SerialBridge\n\r";

/* creates a tcp server socket for bridging serial UART data into a tcp network */
class SerialBridge :	private SerialPort::ISerialHandler,
						private TcpServer::ITcpHandler
{
	Arguments  options;
	SerialPort serialPort;
	TcpServer  tcpServer;

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

	void onTcpReadComplete(const char* msg, size_t length) final
	{
		if (tcpConnected)
		{
			serialPort.send((uint8_t*)msg, length);
		}
	}


	void onSerialWriteComplete(const char* msg, size_t length) final
	{
	}

	void onTcpClientAccept() final
	{
		tcpConnected = true;

		std::cout << "Client Connect" << std::endl;

		checkReadyness();
	}

	void onTcpClientDisconnect() final
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

#if 0
      pWebserver	  = IRGBLightControl::CreateWebserver();
      pDevice 		  = IRGBLightControl::CreateDevice();
      
      if(!pDevice || !pWebserver )
	{
	  break;
	}
      
      if(!pDevice->Init(options.strDevice.c_str(), options.uiBaudrate))
	{
	  std::cerr 	<< "Could not initialize RGBLightControl device! " << std::endl
			<< "Do you have the permissions for accessing the device?"
			<< std::endl;
	  break;
	}
      
      //      pDevice->SetDeviceMode(pDeviceModeConstant);
      
      if(!pWebserver->Init(options.strHttpAddress.c_str(), options.strHttpPort.c_str(), options.strHttpDocRoot.c_str() ))
	{
	  std::cerr << "Error when initializing webserver!\n";
	  break;
	}
      
      std::cout << "WebInterface: http://" 
		<< options.strHttpAddress << ":" 
		<< options.strHttpPort 
		<< "/index.html" 
		<< std::endl;
      
      pWebserver->RegisterActionCode("setcolor", &OnSetColor);
      pWebserver->RegisterActionCode("play", &OnPlay);
      pWebserver->RegisterActionCode("delete", &OnDelete);
      
      if(!pWebserver->Start())
	{
	  std::cerr << "Error when starting webserver!\n";
	}

      if(!pDevice->Start())
	{
	  std::cerr << "Error when starting the RGBLightControl!" << std::endl;
	  break;
	}

      std::cout << "Running..." << std::endl;
      while(pDevice->isStarted())
	{
	  char cInput;
	  std::cin.get(cInput);

	  if(cInput == 3)
	    {
	      break;
	    }
	}

      std::cout << "Performing shutdown" << std::endl;

      if(!pDevice->Stop())
	{
	  std::cerr << "Error when stopping the RGBLightControl!" << std::endl;
	  break;
	}

      if(!pDevice->Cleanup())
	{
	  std::cerr << "Error when cleaning up RGBLightControl object" << std::endl;
	}

      break;
    }
  
  IRGBLightControl::FreeDevice(pDevice);
  IRGBLightControl::FreeWebserver(pWebserver);
#endif
  return EXIT_SUCCESS;
}

