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
using namespace boost::placeholders;

template <class T> class NetworkConnection;
template <class T> static bool StartWriting(class NetworkConnection<T> & connection) noexcept;
template <class T> static void WriteOperationComplete(class NetworkConnection<T> & connection, const boost::system::error_code& oError);

//template <class T> static void ReadOperationComplete(class Connection<T> & connection, const boost::system::error_code& oError, size_t nBytesReceived);


template <typename SocketType>
class NetworkConnection : public std::enable_shared_from_this<NetworkConnection<SocketType>>
{
public:
    static constexpr size_t RX_BUF_SIZE = 512;

    std::vector<char>      m_rxBuffer;
    std::deque<char>       m_txBuffer;

    SocketType    m_socket;

    TcpServer::INetworkHandler* &   m_handler;

    NetworkConnection(SocketType socket, TcpServer::INetworkHandler* & handler)
        : m_socket(std::move(socket)), m_handler(handler)
    {
        m_rxBuffer.resize(RX_BUF_SIZE);
    }



    void start()
    {
        if (nullptr != m_handler)
        {
            m_handler->onNetworkClientAccept();
        }

        read();
    }

    void read()
    {
        auto self(std::enable_shared_from_this<NetworkConnection<SocketType>>::shared_from_this());

        m_socket.async_read_some(boost::asio::buffer(m_rxBuffer.data(), m_rxBuffer.size()),
            [this, self](boost::system::error_code error, std::size_t length)
            {
                if (error)
                {
                    if (boost::asio::error::eof == error ||
                        boost::asio::error::connection_reset)
                    {
                        if (nullptr != m_handler)
                        {
                            m_handler->onNetworkClientDisconnect();
                        }
                    }
                }
                else
                {
                    if (nullptr != m_handler)
                    {
                        m_handler->onNetworkReadComplete(m_rxBuffer.data(), length);
                    }

                    read();
                }
            });
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
        }
    }



    void sendChar(const char msg)
    {
        bool bWriteInProgress = !m_txBuffer.empty();

        m_txBuffer.push_back(msg);

        if (!bWriteInProgress)
        {
            StartWriting<tcp::socket>(*this);
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
                StartWriting<tcp::socket>(*this);
            }
        }
    }


};



template <class T>
bool StartWriting(NetworkConnection<T> & connection) noexcept
{
    try
    {
        boost::asio::async_write(connection.m_socket,
            boost::asio::buffer(&connection.m_txBuffer.front(), 1),
                                 boost::bind(WriteOperationComplete<tcp::socket>, boost::ref(connection), placeholders::error)
        );
    }
    catch (...)
    {
        return false;
    }

    return true;
}





template <class T>
void WriteOperationComplete(NetworkConnection<T> & connection, const boost::system::error_code& oError)
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



struct TcpServer_Private
{
    using TcpConnection = NetworkConnection<tcp::socket>;

    io_service &           m_ioService;
    tcp::endpoint          m_endPoint;
    tcp::acceptor          m_acceptor;

    TcpServer::INetworkHandler*  m_handler = nullptr;

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




