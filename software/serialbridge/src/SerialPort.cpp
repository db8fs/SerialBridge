/**
* @file   SerialPort.cpp
* @date   26.03.2023
* @author Falk Schilling (db8fs)
* @copyright GPLv3
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
    static constexpr int RX_BUFFER_SIZE = 512;

    bool 	               m_active = true;
    io_service             m_ioService;
    serial_port            m_serialPort;

    char 	               m_rxBuffer[RX_BUFFER_SIZE] = {0};
    std::deque<char>       m_txBuffer;

    // completion event handlers
    fnReadComplete	 m_fnReadComplete = nullptr;
    fnWriteComplete m_fnWriteComplete = nullptr;
    void* m_objReadComplete = nullptr;
    void* m_objWriteComplete = nullptr;


    std::thread m_thread; //< todo


    SerialPort_Private(const std::string & device, uint32_t baudrate, enum SerialPort::eFlowControl flowControl)
        :   m_ioService(),
            m_serialPort(m_ioService, device),
            m_thread([&]() { while (m_active) { m_ioService.run_for(std::chrono::milliseconds(20)); }})
    {
        m_serialPort.set_option(serial_port_base::baud_rate(baudrate));
        m_serialPort.set_option( convertFlowControl[flowControl]);
    }



    bool StartReading() noexcept
    {
        try
        {
            m_serialPort.async_read_some(boost::asio::buffer(m_rxBuffer, RX_BUFFER_SIZE),
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



    void ReadOperationComplete(const boost::system::error_code& oError,
        size_t nBytesTransferred)
    {
        if (oError)
        {
            close(oError);
        }
        else
        {
            if (m_fnReadComplete)
            {
                m_fnReadComplete(m_objReadComplete,
                    m_rxBuffer,
                    nBytesTransferred);
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
                m_fnWriteComplete(m_objWriteComplete);
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



    void write(const char msg)
    {
        bool bWriteInProgress = !m_txBuffer.empty();

        m_txBuffer.push_back(msg);

        if (!bWriteInProgress)
        {
            StartWriting();
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



SerialPort::~SerialPort() noexcept
{
    try
    {
        m_private->m_thread.join();
    }
    catch (...)
    {
    }
}



bool SerialPort::setReadCompleteHandler(void* pObject,
	fnReadComplete pCallback)
{
	if (pCallback)
	{
		m_private->m_fnReadComplete = pCallback;
		m_private->m_objReadComplete = pObject;
        return true;
	}
	else
	{
		m_private->m_fnReadComplete = NULL;
		m_private->m_objReadComplete = NULL;
        return false;
	}	
}



bool SerialPort::setWriteCompleteHandler(void* pObject,
	fnWriteComplete pCallback)
{
	if (pCallback)
	{
		m_private->m_fnWriteComplete = pCallback;
		m_private->m_objWriteComplete = pObject;
        return true;
	}
	else
	{
		m_private->m_fnWriteComplete = NULL;
		m_private->m_objWriteComplete = NULL;
		return false;
	}
}


bool SerialPort::write(const char cMsg) noexcept
{
	try
	{
		m_private->m_ioService.post(boost::bind(&SerialPort_Private::write,
			m_private.get(),
			cMsg));
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

