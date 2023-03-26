/**
 * @file         SerialPort.cpp
 * @created		 31.03.2011
 * @date         26.03.2023
 * @author       Falk Schilling (db8fs)
 * @copyright    GPLv3
 */

#include <string>
#include <memory>

#include "SerialPort.h"

#include <deque>
#include <map>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>



using namespace boost::asio;


static std::map<SerialPort::eFlowControl, serial_port_base::flow_control> convertFlowControl = 
{
    std::pair<SerialPort::eFlowControl, serial_port_base::flow_control>( SerialPort::eFlowControl::None, serial_port_base::flow_control::none ),
    std::pair<SerialPort::eFlowControl, serial_port_base::flow_control>(SerialPort::eFlowControl::Hardware, serial_port_base::flow_control::hardware),
    std::pair<SerialPort::eFlowControl, serial_port_base::flow_control>(SerialPort::eFlowControl::Software, serial_port_base::flow_control::software)
};


struct SerialPort_Private
{
    static constexpr size_t RX_BUF_SIZE = 512;

    bool 	               m_active = true;
    io_service             m_ioService;
    serial_port            m_serialPort;

    std::vector<char>    m_rxBuffer;
    std::deque<char>    m_txBuffer;

    // completion event handlers
    SerialPort::fnReadComplete	 m_fnReadComplete = nullptr;
    SerialPort::fnWriteComplete  m_fnWriteComplete = nullptr;


    SerialPort_Private(const std::string & device, uint32_t baudrate, enum SerialPort::eFlowControl flowControl)
        :   m_ioService(),
            m_serialPort(m_ioService, device)
    {
        m_rxBuffer.resize(RX_BUF_SIZE);
        m_serialPort.set_option(serial_port_base::baud_rate(baudrate));
        m_serialPort.set_option( convertFlowControl[flowControl]);
    }



    bool StartReading() noexcept
    {
        try
        {
            m_serialPort.async_read_some(boost::asio::buffer(m_rxBuffer.data(), RX_BUF_SIZE),
                boost::bind(&SerialPort_Private::ReadOperationComplete,
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
            boost::asio::async_write(m_serialPort,
                boost::asio::buffer(&m_txBuffer.front(), 1),
                boost::bind(&SerialPort_Private::WriteOperationComplete,
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
            std::cerr << "SerialPort Error: " << oError.message() << std::endl;
        }
        else
        {
            m_serialPort.close();
            m_active = false;
        }
    }

};




///////////////////////////


SerialPort::SerialPort(const std::string& device, uint32_t baudRate, SerialPort::eFlowControl flowControl)
	: m_private(new SerialPort_Private(device, baudRate, flowControl))
{
	if (m_private->m_serialPort.is_open())
	{
		if (!m_private->StartReading())
		{
			throw std::exception("Failed to open serial port!\n");
		}
	}
}

SerialPort::SerialPort(const SerialPort& rhs)
    : m_private(rhs.m_private)
{
}



SerialPort::~SerialPort() noexcept
{
    try
    {
        m_private.reset();
    }
    catch (...)
    {
    }
}

SerialPort& SerialPort::operator=(const SerialPort& rhs)
{
    if (this != &rhs)
    {
        this->m_private = rhs.m_private;
    }
    return *this;
}



void SerialPort::setCallbacks(SerialPort::fnReadComplete onReadHandler, SerialPort::fnWriteComplete onWriteHandler)
{
    m_private->m_fnReadComplete = onReadHandler;
    m_private->m_fnWriteComplete = onWriteHandler;
}


bool SerialPort::send(const char cMsg) noexcept
{
	try
	{
		m_private->m_ioService.post(boost::bind(&SerialPort_Private::sendChar, m_private.get(), cMsg));
	}
	catch (...)
	{
		return false;
	}

	return true;
}


bool SerialPort::send(const std::string& text)
{
    try
    {
        m_private->m_ioService.post(boost::bind(&SerialPort_Private::sendText,
            m_private.get(),
            text));
    }
    catch (...)
    {
        return false;
    }

    return true;
}



bool SerialPort::send(const uint8_t* const data, size_t length)
{
    try
    {
        m_private->m_ioService.post(boost::bind(&SerialPort_Private::sendBinary,
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


bool SerialPort::close() noexcept
{
	try
	{
		m_private->m_ioService.post(boost::bind(&SerialPort_Private::close,
			m_private.get(),
			boost::system::error_code()));
	}
	catch (...)
	{
		return false;
	}

	return true;
}


bool SerialPort::isActive() const
{
	return m_private->m_active;
}


void SerialPort::update()
{
    if (m_private->m_active)
    {
        m_private->m_ioService.run();
    }
}

void SerialPort::update(uint16_t durationMs)
{
    if (m_private->m_active) 
    { 
        m_private->m_ioService.run_for(std::chrono::milliseconds(durationMs)); 
    }
}


