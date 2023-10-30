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


/** */
class TcpServer
{
	std::shared_ptr<struct TcpServer_Private> m_private;

public:

    class INetworkHandler
	{
    public:
        virtual ~INetworkHandler() {}

        virtual void onNetworkReadComplete(const char* msg, size_t length) = 0;
        virtual void onNetworkClientAccept() = 0;
        virtual void onNetworkClientDisconnect() = 0;
	};



	/** creates a tcp server listening on the given socket */
	TcpServer(const std::string& address, uint16_t tcpPort, const std::string & sslCert);

	TcpServer(const TcpServer&);
	~TcpServer() noexcept;

	TcpServer& operator=(const TcpServer&);

    /** defines asynchronous read or write completion handlers */
    void setHandler(INetworkHandler* const handler);

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
