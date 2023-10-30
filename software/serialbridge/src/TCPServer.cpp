/**
 * @file         TCPServer.cpp
 * @date         01.04.2023
 * @author       Falk Schilling (db8fs)
 * @copyright    GPLv3
 */

#include <string>
#include <memory>

#include "System.h"
#include "TCPServer.h"

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



struct TcpServer_Private
{
    using TcpConnection = NetworkConnection<tcp::socket>;

    io_service &           m_ioService;
    tcp::endpoint          m_endPoint;
    tcp::acceptor          m_acceptor;

    INetworkHandler*       m_handler = nullptr;

    std::shared_ptr<TcpConnection> m_connection;


    TcpServer_Private(const std::string & address, uint16_t port, const std::string & sslCert)
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


TcpServer::TcpServer(const std::string& address, uint16_t port, const std::string & sslCert)
{
    try
    {
        m_private = std::shared_ptr<TcpServer_Private>(new TcpServer_Private(address, port, sslCert));

    }
    catch (...)
    {
        throw "Failed to create TCP Server!";
    }
}


TcpServer::TcpServer(const TcpServer& rhs)
    : m_private(rhs.m_private)
{
}



TcpServer::~TcpServer() noexcept
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

TcpServer& TcpServer::operator=(const TcpServer& rhs)
{
    if (this != &rhs)
    {
        this->m_private = rhs.m_private;
    }
    return *this;
}


void TcpServer::setHandler(INetworkHandler* const handler)
{
    m_private->m_handler = handler;
}


bool TcpServer::send(const char cMsg) noexcept
{
	try
	{
		m_private->m_ioService.post(boost::bind(&TcpServer_Private::sendChar, m_private.get(), cMsg));
	}
	catch (...)
	{
		return false;
	}

	return true;
}


bool TcpServer::send(const std::string& text)
{
    try
    {
        m_private->m_ioService.post(boost::bind(&TcpServer_Private::sendText,
            m_private.get(),
            text));
    }
    catch (...)
    {
        return false;
    }

    return true;
}





bool TcpServer::close() noexcept
{
	try
	{
		m_private->m_ioService.post(boost::bind(&TcpServer_Private::close,
			m_private.get(),
			boost::system::error_code()));
	}
	catch (...)
	{
		return false;
	}

	return true;
}


bool TcpServer::isActive() const
{
	return m_private->m_connection != nullptr;
}




