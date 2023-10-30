/**
 *  @file   main.cpp
 *  @author Falk Schilling (db8fs)
 *  @copyright GPLv3
 *  @remark migrated from SF/Github RGBLightControl repository
 */


#include <csignal>
#include <iostream>
#include <cstdlib>
#include <thread>

#include "Arguments.h"
#include "SerialBridge.h"
#include "System.h"

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

