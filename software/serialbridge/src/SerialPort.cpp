/**
 * @file         SerialPort.cpp
 * @created		 31.03.2011
 * @date         26.03.2023
 * @author       Falk Schilling (db8fs)
 * @copyright    GPLv3
 */

#include <string>
#include <memory>
#include <thread>
#include <chrono>

#include "System.h"
#include "SerialPort.h"

#include <deque>
#include <map>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/filesystem.hpp>


using namespace boost::asio;


static std::map<enum SerialPort::eFlowControl, serial_port_base::flow_control> convertFlowControl = 
{
    std::pair<enum SerialPort::eFlowControl, serial_port_base::flow_control>(SerialPort::eFlowControl::None, serial_port_base::flow_control::none ),
    std::pair<enum SerialPort::eFlowControl, serial_port_base::flow_control>(SerialPort::eFlowControl::Hardware, serial_port_base::flow_control::hardware),
    std::pair<enum SerialPort::eFlowControl, serial_port_base::flow_control>(SerialPort::eFlowControl::Software, serial_port_base::flow_control::software)
};

struct SerialPort_Params
{
    std::string  device;
    uint32_t     baudrate;
    enum SerialPort::eFlowControl flowControl;
    SerialPort::ISerialHandler* handler = nullptr;

    SerialPort_Params(const std::string& device, uint32_t baudrate, enum SerialPort::eFlowControl flowControl)
        : device(device), baudrate(baudrate), flowControl(flowControl)
    {}

};

struct SerialPort_Private
{
    static constexpr size_t RX_BUF_SIZE = 512;

    bool 	               m_active = true;
    io_service &           m_ioService;
    serial_port            m_serialPort;

    std::vector<char>      m_rxBuffer;
    std::deque<char>       m_txBuffer;

    // completion event handlers
    SerialPort::ISerialHandler* & m_handler;


    SerialPort_Private(SerialPort_Params & params)
        : m_ioService(System::IOService()),
          m_serialPort(m_ioService, params.device),
          m_handler(params.handler)
    {
        m_rxBuffer.resize(RX_BUF_SIZE);
        m_serialPort.set_option(serial_port_base::baud_rate(params.baudrate));
        m_serialPort.set_option( convertFlowControl[params.flowControl]);
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
                if (nullptr != m_handler)
                {
                    m_handler->onSerialReadComplete(m_rxBuffer.data(), nBytesReceived);
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
            if (nullptr != m_handler)
            {
                m_handler->onSerialWriteComplete(&m_txBuffer.front(), 1);
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

static bool isSerialPortPresent(const std::string& deviceName)
{

#ifdef WIN32
    {
        constexpr size_t MAX_BUF_LEN = 1000;
        char targetPath[MAX_BUF_LEN+1] = { 0 };

        DWORD result(QueryDosDeviceA(deviceName.c_str(), &targetPath[0], MAX_BUF_LEN));

        return result > 0;
    }
#else
    {
        using namespace boost;
        return (!filesystem::exists(deviceName) ||
            !filesystem::is_regular_file(deviceName));
    }
#endif
}

///////////////////////////


SerialPort::SerialPort(const std::string& device, uint32_t baudRate, enum SerialPort::eFlowControl flowControl)
    :   m_params(new SerialPort_Params(device, baudRate, flowControl)),
        m_private(nullptr)
{
}


SerialPort::SerialPort(const SerialPort& rhs)
    :   m_params(rhs.m_params),
        m_private(rhs.m_private)
{
}


SerialPort::~SerialPort() noexcept
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

SerialPort& SerialPort::operator=(const SerialPort& rhs)
{
    if (this != &rhs)
    {
        this->m_private = rhs.m_private;
    }
    return *this;
}

void SerialPort::awaitConnection(uint16_t waitMs)
{
    if (isSerialPortPresent(m_params->device))
    {
        try
        {
            m_private = std::shared_ptr<SerialPort_Private>(new SerialPort_Private(*m_params));
        }
        catch (...)
        {
            throw "Failed to open serial port!";
        }

        if (nullptr != m_params->handler)
        {
            m_params->handler->onSerialConnected();
        }

        std::cout << "Serial port connected" << std::endl;
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(waitMs));
    }
}

bool SerialPort::start()
{
    if (m_private->m_serialPort.is_open())
    {
        return m_private->StartReading();
    }

    return false;
}


void SerialPort::setHandler(ISerialHandler* const handler)
{
    m_params->handler = handler;
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




