#ifndef SYSTEM_H_F827BDB7_196A_434A_A479_074B19BE93BC
#define SYSTEM_H_F827BDB7_196A_434A_A479_074B19BE93BC

/**
 * @file		System.h
 * @created		01.04.2023
 * @author		Falk Schilling (db8fs)
 * @copyright	GPLv3
 */

#include <string>

#include <boost/asio.hpp>


/** singleton access to system services or cross-cutting stuff (maybe logging) */
class System
{
	static struct System_Private m_private;

public:

	/** gets the fundamental asio service for running all IO requests */
	static boost::asio::io_service & IOService();

	/** runs the io service */
	static void run();


	/** the identifier to connect to all sockets */
	static const std::string ALL_INTERFACES;

};

#endif /* SYSTEM_H_F827BDB7_196A_434A_A479_074B19BE93BC */