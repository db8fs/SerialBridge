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
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>



using namespace boost::asio;
using namespace boost::asio::ip;


static void onAcceptConnection(const boost::system::error_code& ec)
{
    if (!ec)
    {
        std::cout << "Connected" << std::endl;
    }

#if 0
    if (m_private->m_socket.is_open())
    {
        if (!m_private->StartReading())
        {
            throw std::exception();
        }
    }
#endif
}



struct TCPServer_Private
{
    static constexpr size_t RX_BUF_SIZE = 512;

    bool 	               m_active = true;
    io_service &           m_ioService;
    tcp::endpoint          m_endpoint;
    tcp::acceptor          m_acceptor;
    tcp::socket            m_socket;

    std::vector<char>      m_rxBuffer;
    std::deque<char>       m_txBuffer;

    // completion event handlers
    TCPServer::fnReadComplete	    m_fnReadComplete = nullptr;
    TCPServer::fnWriteComplete      m_fnWriteComplete = nullptr;
    TCPServer::fnAcceptConnection   m_fnAcceptConnection = nullptr;


    TCPServer_Private(const std::string & address, uint16_t port, const std::string & sslCert)
        :   m_ioService(System::IOService()),
            m_endpoint(ip::address::from_string(address), port),
            m_acceptor(m_ioService, m_endpoint),
            m_socket(m_ioService, m_endpoint.protocol())
    {
        m_rxBuffer.resize(RX_BUF_SIZE);
        
        m_acceptor.listen();
        m_acceptor.async_accept(m_socket, &onAcceptConnection);
    }



    bool StartReading() noexcept
    {
        try
        {
            m_socket.async_read_some(boost::asio::buffer(m_rxBuffer.data(), RX_BUF_SIZE),
                boost::bind(&TCPServer_Private::ReadOperationComplete,
                    this,
                    placeholders::error,
                    placeholders::bytes_transferred
                )
            );
        }
        catch (...)
        {
            return false;
        }

        return true;
    }


    bool StartWriting() noexcept
    {
        try
        {
            boost::asio::async_write(m_socket,
                boost::asio::buffer(&m_txBuffer.front(), 1),
                boost::bind(&TCPServer_Private::WriteOperationComplete,
                    this,
                    placeholders::error)
            );
        }
        catch (...)
        {
            return false;
        }

        return true;
    }



    void ReadOperationComplete(const boost::system::error_code& oError, size_t nBytesReceived)
    {
        if (oError)
        {
            close(oError);
        }
        else
        {
            if (nBytesReceived > 0)
            {
                if (m_fnReadComplete)
                {
                    m_fnReadComplete(m_rxBuffer.data(), nBytesReceived);
                }

                m_rxBuffer.clear();
                m_rxBuffer.resize(RX_BUF_SIZE);
            }

            if (!StartReading())
            {
                //ExecuteCloseOperation(oError); //< todo: not sure if still necessary
            }
        }
        
    }



    void WriteOperationComplete(const boost::system::error_code& oError)
    {
        if (oError)
        {
            close(oError);
        }
        else
        {
            if (m_fnWriteComplete)
            {
                m_fnWriteComplete(m_txBuffer.front());
            }

            m_txBuffer.pop_front();

            if (!m_txBuffer.empty())
            {
                if (!StartWriting())
                {
                    //ExecuteCloseOperation(oError); //< todo: not sure if still necessary
                }
            }
        }
    }



    void sendChar(const char msg)
    {
        bool bWriteInProgress = !m_txBuffer.empty();

        m_txBuffer.push_back(msg);

        if (!bWriteInProgress)
        {
            StartWriting();
        }
    }

    void sendText(const std::string & msg)
    {
        if (!msg.empty())
        {
            bool bWriteInProgress = !m_txBuffer.empty();

            std::copy(msg.begin(), msg.end(), std::back_inserter(m_txBuffer));

            if (!bWriteInProgress)
            {
                StartWriting();
            }
        }
    }

    void sendBinary(const uint8_t* const msg, size_t length)
    {
        if (nullptr != msg)
        {
            bool bWriteInProgress = !m_txBuffer.empty();

            std::copy(msg, msg + length, std::back_inserter(m_txBuffer));

            if (!bWriteInProgress)
            {
                StartWriting();
            }
        }
    }


    void close(const boost::system::error_code& oError)
    {
        if (oError == boost::asio::error::operation_aborted)
        {
            std::cerr << "TCPServer Error: " << oError.message() << std::endl;
        }
        else
        {
            m_socket.shutdown(tcp::socket::shutdown_both);
            m_socket.close();
            m_active = false;
        }
    }

};




///////////////////////////


TCPServer::TCPServer(const std::string& address, uint16_t port, const std::string & sslCert)
{
    try
    {
        m_private = std::shared_ptr<TCPServer_Private>(new TCPServer_Private(address, port, sslCert));

    }
    catch (...)
    {
        throw "Failed to create TCP Server!";
    }
}

TCPServer::TCPServer(const TCPServer& rhs)
    : m_private(rhs.m_private)
{
}



TCPServer::~TCPServer() noexcept
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

TCPServer& TCPServer::operator=(const TCPServer& rhs)
{
    if (this != &rhs)
    {
        this->m_private = rhs.m_private;
    }
    return *this;
}


void TCPServer::setCallbacks(TCPServer::fnAcceptConnection onAccept, TCPServer::fnReadComplete onReadHandler, TCPServer::fnWriteComplete onWriteHandler)
{
    m_private->m_fnReadComplete = onReadHandler;
    m_private->m_fnWriteComplete = onWriteHandler;
    m_private->m_fnAcceptConnection = onAccept;
}


bool TCPServer::send(const char cMsg) noexcept
{
	try
	{
		m_private->m_ioService.post(boost::bind(&TCPServer_Private::sendChar, m_private.get(), cMsg));
	}
	catch (...)
	{
		return false;
	}

	return true;
}


bool TCPServer::send(const std::string& text)
{
    try
    {
        m_private->m_ioService.post(boost::bind(&TCPServer_Private::sendText,
            m_private.get(),
            text));
    }
    catch (...)
    {
        return false;
    }

    return true;
}



bool TCPServer::send(const uint8_t* const data, size_t length)
{
    try
    {
        m_private->m_ioService.post(boost::bind(&TCPServer_Private::sendBinary,
            m_private.get(),
            data,
            length));
    }
    catch (...)
    {
        return false;
    }

    return true;
}


bool TCPServer::close() noexcept
{
	try
	{
		m_private->m_ioService.post(boost::bind(&TCPServer_Private::close,
			m_private.get(),
			boost::system::error_code()));
	}
	catch (...)
	{
		return false;
	}

	return true;
}


bool TCPServer::isActive() const
{
	return m_private->m_active;
}




