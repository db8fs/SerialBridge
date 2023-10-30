#ifndef NETWORK_CONNECTION_H_
#define NETWORK_CONNECTION_H_

#include "INetworkHandler.h"

#include <iostream>

#include <string>
#include <memory>

#include <deque>
#include <vector>

#include <boost/bind/bind.hpp>
using namespace boost::placeholders;

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;


template <class T> class NetworkConnection;
template <class T> static bool StartWriting(class NetworkConnection<T> & connection) noexcept;
template <class T> static void WriteOperationComplete(class NetworkConnection<T> & connection, const boost::system::error_code& oError);

//template <class T> static void ReadOperationComplete(class Connection<T> & connection, const boost::system::error_code& oError, size_t nBytesReceived);


/** an established network connection between the server and a connected client */
template <typename SocketType>
class NetworkConnection : public std::enable_shared_from_this<NetworkConnection<SocketType>>
{
public:
    static constexpr size_t RX_BUF_SIZE = 512;

    std::vector<char>      m_rxBuffer; /**< received data from network */
    std::deque<char>       m_txBuffer; /**< data for being transmitted via network */
    SocketType             m_socket;   /**< network communication socket */
    INetworkHandler* &     m_handler;  /**< network event handler */

    NetworkConnection(SocketType socket, INetworkHandler* & handler)
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
            StartWriting<SocketType>(*this);
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
                StartWriting<SocketType>(*this);
            }
        }
    }
};



/** starts network transmission for a chunk of data to be sent */
template <class T>
bool StartWriting(NetworkConnection<T> & connection) noexcept
{
    try
    {
        boost::asio::async_write(connection.m_socket,
                                 boost::asio::buffer(&connection.m_txBuffer.front(), 1),
                                 boost::bind(WriteOperationComplete<T>, boost::ref(connection), placeholders::error)
                                 );
    }
    catch (...)
    {
        return false;
    }

    return true;
}


/** event handler for transmitted data */
template <class T>
void WriteOperationComplete(NetworkConnection<T> & connection, const boost::system::error_code& oError)
{
    if (oError)
    {
        connection.close(oError);
    }
    else
    {
#if 0 // just for debugging of tx
        if (m_fnWriteComplete)
        {
            m_fnWriteComplete(m_txBuffer.front());
        }
#endif

        connection.m_txBuffer.pop_front();

        if (!connection.m_txBuffer.empty())
        {
            if (!StartWriting(connection)) // as soon if smthg was being sent, recheck the tx queue for new data
            {
                //ExecuteCloseOperation(oError); //< todo: not sure if still necessary
            }
        }
    }
}


#endif

