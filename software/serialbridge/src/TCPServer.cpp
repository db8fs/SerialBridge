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

#include <boost/bind.hpp>
#include <memory>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>



using namespace boost::asio;
using namespace boost::asio::ip;


// TODO: accept handling, start reading/writing on socket only if client is connected and if serialdevice is present

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

static bool StartReading(class TCPConnection & connection) noexcept;
static bool StartWriting(class TCPConnection & connection) noexcept;
static void ReadOperationComplete(class TCPConnection & connection, const boost::system::error_code& oError, size_t nBytesReceived);
static void WriteOperationComplete(class TCPConnection & connection, const boost::system::error_code& oError);



class TCPConnection : public std::enable_shared_from_this<TCPConnection>
{
public:
    static constexpr size_t RX_BUF_SIZE = 512;

    std::vector<char>      m_rxBuffer;
    std::deque<char>       m_txBuffer;

    tcp::socket            m_socket;


    TCPConnection(io_service& ioService, tcp::endpoint& endPoint)
        : m_socket(ioService, endPoint.protocol())
    {
        m_rxBuffer.resize(RX_BUF_SIZE);
    }

    tcp::socket& getSocket() { return m_socket; }

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
        }
    }



    void sendChar(const char msg)
    {
        bool bWriteInProgress = !m_txBuffer.empty();

        m_txBuffer.push_back(msg);

        if (!bWriteInProgress)
        {
            StartWriting(*this);
        }
    }


    void sendText(const std::string& msg)
    {
        if (!msg.empty())
        {
            bool bWriteInProgress = !m_txBuffer.empty();

            std::copy(msg.begin(), msg.end(), std::back_inserter(m_txBuffer));

            if (!bWriteInProgress)
            {
                StartWriting(*this);
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
                StartWriting(*this);
            }
        }
    }
};



bool StartReading(TCPConnection & connection) noexcept
{
    try
    {
        connection.m_socket.async_read_some(boost::asio::buffer(connection.m_rxBuffer.data(), connection.RX_BUF_SIZE),
            boost::bind(ReadOperationComplete,
                boost::ref(connection),
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


bool StartWriting(TCPConnection & connection) noexcept
{
    try
    {
        boost::asio::async_write(connection.m_socket,
            boost::asio::buffer(&connection.m_txBuffer.front(), 1),
            boost::bind(WriteOperationComplete,
                boost::ref(connection),
                placeholders::error)
        );
    }
    catch (...)
    {
        return false;
    }

    return true;
}



void ReadOperationComplete(TCPConnection & connection, const boost::system::error_code& oError, size_t nBytesReceived)
{
    if (oError)
    {
        connection.close(oError);
    }
    else
    {
        if (nBytesReceived > 0)
        {
#if 0
            if (m_fnReadComplete)
            {
                m_fnReadComplete(m_rxBuffer.data(), nBytesReceived);
            }
#endif

            connection.m_rxBuffer.clear();
            connection.m_rxBuffer.resize(connection.RX_BUF_SIZE);
        }

        if (!StartReading(connection))
        {
            //ExecuteCloseOperation(oError); //< todo: not sure if still necessary
        }
    }    
}



void WriteOperationComplete(TCPConnection & connection, const boost::system::error_code& oError)
{
    if (oError)
    {
        connection.close(oError);
    }
    else
    {
#if 0
        if (m_fnWriteComplete)
        {
            m_fnWriteComplete(m_txBuffer.front());
        }
#endif

        connection.m_txBuffer.pop_front();

        if (!connection.m_txBuffer.empty())
        {
            if (!StartWriting(connection))
            {
                //ExecuteCloseOperation(oError); //< todo: not sure if still necessary
            }
        }
    }
}






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



struct TCPServer_Private
{
    io_service &           m_ioService;
    tcp::endpoint          m_endPoint;
    tcp::acceptor          m_acceptor;

    std::shared_ptr<TCPConnection> m_connection;

    // completion event handlers
    TCPServer::fnReadComplete	    m_fnReadComplete = nullptr;
    TCPServer::fnWriteComplete      m_fnWriteComplete = nullptr;
    TCPServer::fnAcceptConnection   m_fnAcceptConnection = nullptr;


    TCPServer_Private(const std::string & address, uint16_t port, const std::string & sslCert)
        :   m_ioService(System::IOService()),
            m_endPoint(createEndpoint(address, port)),
            m_acceptor(m_ioService, m_endPoint)
    {        
        m_acceptor.listen();

        this->startAccepting();
    }


    void startAccepting()
    {
        m_connection = std::shared_ptr<TCPConnection>(new TCPConnection( m_ioService, m_endPoint));
        m_acceptor.async_accept(m_connection->getSocket(), boost::bind(&TCPServer_Private::onAccept, this, m_connection, placeholders::error));
    }


    void onAccept(std::shared_ptr<TCPConnection> connection, const boost::system::error_code& error)
    {
        if (!error)
        {
            if (connection != nullptr)
            {
                std::cout << "onAccept" << std::endl;
                StartReading(*connection);
            }
        }

        this->startAccepting();
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

    void sendBinary(const uint8_t* const msg, size_t length)
    {
        if (nullptr != m_connection)
        {
            if (nullptr != msg)
            {
                m_connection->sendBinary(msg, length);
            }
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
	return m_private->m_connection != nullptr;
}




