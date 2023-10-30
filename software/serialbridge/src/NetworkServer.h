#ifndef NETWORKSERVER_H_733511C1_8156_4C83_BB22_81FD2CDB2E6F
#define NETWORKSERVER_H_733511C1_8156_4C83_BB22_81FD2CDB2E6F

/**
 * @file		NetworkServer.h
 * @author		Falk Schilling (db8fs)
 * @copyright	GPLv3
 */

#include <string>
#include <memory>


/** */
class NetworkServer
{
    std::shared_ptr<struct NetworkServer_Private> m_private;

public:

    /** creates a tcp server listening on the given socket */
    NetworkServer(const std::string& address, uint16_t port, const std::string & sslCert);

    NetworkServer(const NetworkServer&);
    ~NetworkServer() noexcept;

    NetworkServer& operator=(const NetworkServer&);

    /** defines asynchronous read or write completion handlers */
    void setHandler(class INetworkHandler* const handler);

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

#endif /* TCPSERVER_H_733511C1_8156_4C83_BB22_81FD2CDB2E6F */
