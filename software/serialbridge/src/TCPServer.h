#ifndef TCP_SERVER_H_733511C1_8156_4C83_BB22_81FD2CDB2E6F
#define TCP_SERVER_H_733511C1_8156_4C83_BB22_81FD2CDB2E6F

/**
 * @file		TCPServer.h
 * @created		01.04.2023
 * @author		Falk Schilling (db8fs)
 * @copyright	GPLv3
 */

#include <string>
#include <memory>
#include <boost/system.hpp>

/** */
class TCPServer
{
	std::shared_ptr<struct TCPServer_Private> m_private;

public:

	typedef void (*fnReadComplete)(const char* msg, size_t length);
	typedef void (*fnWriteComplete)(const char msg);
	typedef void (*fnAcceptConnection)(const boost::system::error_code & ec);

	/** creates a tcp server listening on the given socket */
	TCPServer(const std::string& address, uint16_t tcpPort, const std::string & sslCert);

	TCPServer(const TCPServer&);
	~TCPServer() noexcept;

	TCPServer& operator=(const TCPServer&);

	/** defines asynchronous read or write completion handlers */
	void setCallbacks(TCPServer::fnAcceptConnection onAccept, TCPServer::fnReadComplete onReadHandler, TCPServer::fnWriteComplete onWriteHandler);

	/** transmit single character */
	bool send(const char cMsg) noexcept;

	/** transmit text */
	bool send(const std::string& text);

	/** transmit buffer */
	bool send(const uint8_t* const data, size_t length);

	/** closes device */
	bool close() noexcept;

	/** true if still transceiving */
	bool isActive() const;

};


//////////////////////////////////////////////////////////////////////////////

#endif /* TCP_SERVER_H_733511C1_8156_4C83_BB22_81FD2CDB2E6F */
