#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <iostream>
#include <string>

struct Arguments
{
  Arguments()
    : strAddress("127.0.0.1"),
      strPort("23"),
      strSSLCert(""),
      strDevice("/dev/ttyUSB0"),
      uiBaudrate(115200)
  {
  }

  std::string strAddress;
  std::string strPort;
  std::string strSSLCert;
  std::string strDevice;
  unsigned int uiBaudrate;
};

std::ostream &operator<<(std::ostream & oStream, const Arguments & conf);


bool parseArguments(Arguments & config, int argc, char* argv[]);

#endif // ARGUMENTS_H
