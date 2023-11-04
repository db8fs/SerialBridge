#ifndef ARGUMENTS_H_D0B5E333_DDD7_4F7C_B571_EB8BF014FFB6
#define ARGUMENTS_H_D0B5E333_DDD7_4F7C_B571_EB8BF014FFB6

/**
 * @file		Arguments.h
 * @date        26.03.2023
 * @author		Falk Schilling (db8fs)
 * @copyright	GPLv3
 */

#include <cstdint>
#include <iostream>
#include <string>


/** command line arguments for this application */
struct Arguments
{
  Arguments()
    : strAddress("127.0.0.1"),
      port(23),
      strSSLCert(""),
      strDevice("/dev/ttyUSB0"),
      uiBaudrate(115200),
      useUDP(false)
  {
  }

  std::string strAddress;
  uint16_t      port;
  std::string strSSLCert;
  std::string strDevice;
  uint32_t uiBaudrate;
  bool useUDP;
};

std::ostream &operator<<(std::ostream & oStream, const Arguments & conf);


bool parseArguments(Arguments & config, int argc, char* argv[]);

#endif /* ARGUMENTS_H_D0B5E333_DDD7_4F7C_B571_EB8BF014FFB6 */
