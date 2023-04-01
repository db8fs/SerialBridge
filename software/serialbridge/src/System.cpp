/**
 * @file		System.cpp
 * @created		01.04.2023
 * @author		Falk Schilling (db8fs)
 * @copyright	GPLv3
 */


#include "System.h"

#include <boost/asio.hpp>


struct System_Private
{
	boost::asio::io_context ioService;
} 
System::m_private;



boost::asio::io_context& System::IOService()
{
	return m_private.ioService;
}



void System::run()
{
	m_private.ioService.run();
}

