
/**
 * @file		Arguments.cpp
 * @date        26.03.2023
 * @author		Falk Schilling (db8fs)
 * @copyright	GPLv3
 */

#include <boost/system/config.hpp>
#include <boost/program_options.hpp>

#include "Arguments.h"

std::ostream &operator<<(std::ostream & oStream, const Arguments & conf)
{
    oStream << "SerialBridge Configuration " << std::endl
            << "------------------------------" << std::endl;
    oStream << "Address: " << conf.strAddress << std::endl;
    oStream << "Port: " << conf.port << std::endl;
    oStream << "Device: " << conf.strDevice << std::endl;
    oStream << "Baudrate: " << conf.uiBaudrate << std::endl;

    return oStream;
}





using namespace boost::program_options;

bool parseArguments(Arguments & config, int argc, char* argv[])
{
    options_description cmdlineOptions("Usage");
    options_description generic("Generic");
    options_description device("Device");
    options_description serverInterface("TCP/IP Server Interface");

    generic.add_options()
            ("help,h", "this description")
            ("config", "prints the current configuration")
            ("version,v", "about this software")
            ;

    device.add_options()
            ("device,d", value< std::string >()->default_value( "/dev/ttyUSB0" ), "path to the serial device")
            ("baudrate,b", value<unsigned int>()->default_value( 115200U ), "sets baudrate for selected device")
            ;

    serverInterface.add_options()
            ("ip,i", value< std::string >()->default_value( "127.0.0.1" ), "Address of the server" )
            ("port,p", value<uint16_t>()->default_value( 23 ), "Tcp port of the server" )
            ("ssl-cert,r", value< std::string >()->default_value( "" ), "ssl cert of the server" )
            ;

    cmdlineOptions.add(generic).add(device).add(serverInterface);

    try
    {
        variables_map vm;
        store(parse_command_line(argc, argv, cmdlineOptions), vm);
        notify(vm);

        // device
        if (vm.count("baudrate"))
        {
            config.uiBaudrate = vm["baudrate"].as<unsigned int>();
        }

        if(vm.count("device"))
        {
            config.strDevice = vm["device"].as< std::string >();
        }

        // webserver
        if(vm.count("ip"))
        {
            config.strAddress = vm["ip"].as< std::string > ();
        }

        if(vm.count("port"))
        {
            config.port = vm["port"].as<uint16_t> ();
        }

        if(vm.count("ssl-cert"))
        {
            config.strSSLCert = vm["ssl-cert"].as< std::string >();
        }


        // generic
        if (vm.count("help"))
        {
            std::cout << cmdlineOptions << "\n";
            return false;
        }

        if(vm.count("version"))
        {
            //std::cout << PACKAGE_STRING << " <" << PACKAGE_URL << "> " << std::endl;
            std::cout << "(c) 2023 by Falk Schilling" << std::endl;
            std::cout << "License GPLv3+: GNU GPL Version 3 or higher <http://gnu.org/licenses/gpl.html>" << std::endl;
            return false;
        }

        if(vm.count("config"))
        {
            std::cout << config;
            return false;
        }
    }
    catch(...)
    {
        std::cout << cmdlineOptions << std::endl;
        return false;
    }

    return true;
}
