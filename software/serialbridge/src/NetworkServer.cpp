/**
 * @file         TCPServer.cpp
 * @date         01.04.2023
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



static tcp::endpoint createEndpoint(const std::string & address, uint16_t port)
{
    if (address != System::ALL_INTERFACES)
    {
        return tcp::endpoint(address::from_string(address), port);
    }
    else
    {
        return tcp::endpoint(tcp::v4(), port);
    }
}



struct NetworkServer_Private
{
    using TcpConnection = NetworkConnection<tcp::socket>;

    io_service &           m_ioService;
    tcp::endpoint          m_endPoint;
    tcp::acceptor          m_acceptor;

    INetworkHandler*       m_handler = nullptr;

    std::shared_ptr<TcpConnection> m_connection;


    NetworkServer_Private(const std::string & address, uint16_t port, const std::string & sslCert)
        :   m_ioService(System::IOService()),
            m_endPoint(createEndpoint(address, port)),
            m_acceptor(m_ioService, m_endPoint)
    {        
        m_acceptor.listen();

        this->startAccepting();
    }


    void startAccepting()
    {
        m_acceptor.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    m_connection = std::make_shared<TcpConnection>(std::move(socket), m_handler);
                    m_connection->start();
                }

                this->startAccepting();
            });
    }




    void sendChar(const char msg)
    {
        if (nullptr != m_connection)
        {
            m_connection->sendChar(msg);
        }
        
    }

    void sendText(const std::string & msg)
    {
        if (nullptr != m_connection)
        {
            m_connection->sendText(msg);
        }
    }


    void close(boost::system::error_code ec)
    {
        if (nullptr != m_connection)
        {
            m_connection->close(ec);
            m_connection.reset();
        }
    }


   

};




///////////////////////////


NetworkServer::NetworkServer(const std::string& address, uint16_t port, const std::string & sslCert)
{
    try
    {
        m_private = std::shared_ptr<NetworkServer_Private>(new NetworkServer_Private(address, port, sslCert));

    }
    catch (...)
    {
        throw "Failed to create TCP Server!";
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
        m_private->m_ioService.post(boost::bind(&NetworkServer_Private::sendChar, m_private.get(), cMsg));
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
        m_private->m_ioService.post(boost::bind(&NetworkServer_Private::sendText,
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
        m_private->m_ioService.post(boost::bind(&NetworkServer_Private::close,
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
	return m_private->m_connection != nullptr;
}




