/**
 * @file         TCPServer.cpp
 * @author       Falk Schilling (db8fs)
 * @copyright    GPLv3
 */

#include <string>
#include <memory>

#include "System.h"
#include "NetworkServer.h"

#include <deque>
#include <map>
#include <iostream>

#include "NetworkConnection.h"

template<class Endpoint> Endpoint getDefaultEndpoint(uint16_t port);
template<> tcp::endpoint getDefaultEndpoint<tcp::endpoint>(uint16_t port) { return tcp::endpoint(tcp::v4(), port); }
template<> udp::endpoint getDefaultEndpoint<udp::endpoint>(uint16_t port) { return udp::endpoint(udp::v4(), port); }

template <class Endpoint>
static Endpoint createEndpoint(const std::string & address, uint16_t port)
{
    if (address != System::ALL_INTERFACES)
    {
        return Endpoint(address::from_string(address), port);
    }
    else
    {
        return getDefaultEndpoint<Endpoint>(port);
    }
}


/** strategy pattern for network server type abstraction */
struct AbstractServer
{
    io_service& m_ioService;
    INetworkHandler* m_handler = nullptr;

    AbstractServer()
        : m_ioService(System::IOService())
    {
    }

    virtual ~AbstractServer() {}

    virtual void sendChar(const char msg) = 0;

    virtual void sendText(const std::string& msg) = 0;

    virtual void close(boost::system::error_code ec) = 0;

    virtual bool isActive() const = 0;
};



/** connection oriented network server implementation */
template <class Endpoint, class Socket, class Acceptor>
struct ConnectionOriented : AbstractServer
{
    Endpoint               m_endPoint;
    Acceptor               m_acceptor;

    std::shared_ptr<NetworkConnection<Socket>> m_connection;

    ConnectionOriented(const std::string & address, uint16_t port, const std::string & sslCert)
        :   m_endPoint(createEndpoint<Endpoint>(address, port)),
            m_acceptor(m_ioService, m_endPoint)
    {        
        m_acceptor.listen();

        this->startAccepting();
    }


    void startAccepting()
    {
        m_acceptor.async_accept(
            [this](boost::system::error_code ec, Socket socket)
            {
                if (!ec)
                {
                    m_connection = std::make_shared<NetworkConnection<Socket>>(std::move(socket), m_handler);
                    m_connection->start();
                }

                this->startAccepting();
            });
    }

    void sendChar(const char msg) final
    {
        if (nullptr != m_connection)
        {
            m_connection->sendChar(msg);
        }

    }

    void sendText(const std::string& msg) final
    {
        if (nullptr != m_connection)
        {
            m_connection->sendText(msg);
        }
    }


    void close(boost::system::error_code ec) final
    {
        if (nullptr != m_connection)
        {
            m_connection->close(ec);
            m_connection.reset();
        }
    }


    bool isActive() const final
    {
        return m_connection != nullptr;
    }

};



///////////

NetworkServer::NetworkServer(const std::string& address, uint16_t port, eTransport protocol, const std::string & sslCert)
{
    try
    {
        switch (protocol)
        {
        case eTransport::TcpV4:
            m_private = std::shared_ptr<AbstractServer>(new ConnectionOriented<tcp::endpoint, tcp::socket, tcp::acceptor>(address, port, sslCert));
            break;
        case eTransport::UdpV4:
            //m_private = std::shared_ptr<AbstractServer>(new NetworkServer_Impl<udp::endpoint, udp::socket, udp::acceptor>(address, port, sslCert));
            break;
        }
        
    }
    catch (...)
    {
        throw "Failed to create Network Server!";
    }
}


NetworkServer::NetworkServer(const NetworkServer& rhs)
    : m_private(rhs.m_private)
{
}



NetworkServer::~NetworkServer() noexcept
{
    try
    {
        this->close();
        m_private.reset();
    }
    catch (...)
    {
    }
}

NetworkServer& NetworkServer::operator=(const NetworkServer& rhs)
{
    if (this != &rhs)
    {
        this->m_private = rhs.m_private;
    }
    return *this;
}


void NetworkServer::setHandler(INetworkHandler* const handler)
{
    m_private->m_handler = handler;
}


bool NetworkServer::send(const char cMsg) noexcept
{
	try
    {
        m_private->m_ioService.post(boost::bind(&AbstractServer::sendChar, m_private.get(), cMsg));
	}
	catch (...)
	{
		return false;
	}

	return true;
}


bool NetworkServer::send(const std::string& text)
{
    try
    {
        m_private->m_ioService.post(boost::bind(&AbstractServer::sendText,
            m_private.get(),
            text));
    }
    catch (...)
    {
        return false;
    }

    return true;
}





bool NetworkServer::close() noexcept
{
	try
    {
        m_private->m_ioService.post(boost::bind(&AbstractServer::close,
			m_private.get(),
			boost::system::error_code()));
	}
	catch (...)
	{
		return false;
	}

	return true;
}


bool NetworkServer::isActive() const
{
    return m_private->isActive();
}




