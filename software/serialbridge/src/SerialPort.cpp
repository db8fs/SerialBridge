/**
* @file   SerialPort.cpp
* @date   26.03.2023
* @author Falk Schilling (db8fs)
* @copyright GPLv3
*/

#include "SerialPort.h"

struct SerialPort_Private
{
    static constexpr int RX_BUFFER_SIZE = 512;

    bool 	                 m_bActive = true;
    io_service            m_oIOService;
    serial_port            m_oSerialPort;
    char 	                 m_acReadMsg[RX_BUFFER_SIZE] = {0};
    std::deque<char>       m_qcWriteMsg;

    PREADCOMPLETECALLBACK	 m_pReadCompleteCallback = nullptr;
    PWRITECOMPLETECALLBACK m_pWriteCompleteCallback = nullptr;
    void* m_pReadCompleteCallbackObject = nullptr;
    void* m_pWriteCompleteCallbackObject = nullptr;

    std::thread m_thread; //< todo


    SerialPort_Private(const std::string & device, uint32_t baudrate, flow_control_t flowControl)
        :   m_oIOService(),
            m_oSerialPort(m_oIOService, device),
            m_thread([&]() { while (m_bActive) { m_oIOService.run_for(std::chrono::milliseconds(20)); }})
    {
        m_oSerialPort.set_option(serial_port_base::baud_rate(baudrate));
        m_oSerialPort.set_option(serial_port_base::flow_control(flowControl));

    }



    bool StartReading() noexcept
    {
        try
        {
            m_oSerialPort.async_read_some(boost::asio::buffer(m_acReadMsg, RX_BUFFER_SIZE),
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
            boost::asio::async_write(m_oSerialPort,
                boost::asio::buffer(&m_qcWriteMsg.front(), 1),
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
            ExecuteCloseOperation(oError);
        }
        else
        {
            if (m_pReadCompleteCallback)
            {
                m_pReadCompleteCallback(m_pReadCompleteCallbackObject,
                    m_acReadMsg,
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
            ExecuteCloseOperation(oError);
        }
        else
        {
            if (m_pWriteCompleteCallback)
            {
                m_pWriteCompleteCallback(m_pWriteCompleteCallbackObject);
            }

            m_qcWriteMsg.pop_front();

            if (!m_qcWriteMsg.empty())
            {
                if (!StartWriting())
                {
                    //ExecuteCloseOperation(oError); //< todo: not sure if still necessary
                }
            }
        }
    }



    void ExecuteWriteOperation(const char msg)
    {
        bool bWriteInProgress = !m_qcWriteMsg.empty();

        m_qcWriteMsg.push_back(msg);

        if (!bWriteInProgress)
        {
            StartWriting();
        }
    }



    void ExecuteCloseOperation(const boost::system::error_code& oError)
    {
        if (oError == boost::asio::error::operation_aborted)
        {
            std::cerr << "SerialPort Error: " << oError.message() << std::endl;
        }
        else
        {
            m_oSerialPort.close();
            m_bActive = false;
        }
    }

};




///////////////////////////


SerialPort::SerialPort(unsigned int uiBaudRate, flow_control_t eFlowControl, const std::string& strDevice )
	: m_private(new SerialPort_Private(strDevice, uiBaudRate, eFlowControl))
{
	if (m_private->m_oSerialPort.is_open())
	{
		if (!m_private->StartReading())
		{
			throw std::exception("Failed to open serial port!\n");
		}
	}
}



SerialPort::~SerialPort()
{
    m_private->m_thread.join();
}



bool SerialPort::SetReadCompletionCallback(void* pObject,
	PREADCOMPLETECALLBACK pCallback)
{
	if (pCallback)
	{
		m_private->m_pReadCompleteCallback = pCallback;
		m_private->m_pReadCompleteCallbackObject = pObject;
        return true;
	}
	else
	{
		m_private->m_pReadCompleteCallback = NULL;
		m_private->m_pReadCompleteCallbackObject = NULL;
        return false;
	}	
}



bool SerialPort::SetWriteCompletionCallback(void* pObject,
	PWRITECOMPLETECALLBACK pCallback)
{
	if (pCallback)
	{
		m_private->m_pWriteCompleteCallback = pCallback;
		m_private->m_pWriteCompleteCallbackObject = pObject;
        return true;
	}
	else
	{
		m_private->m_pWriteCompleteCallback = NULL;
		m_private->m_pWriteCompleteCallbackObject = NULL;
		return false;
	}
}


bool SerialPort::Write(const char cMsg) noexcept
{
	try
	{
		m_private->m_oIOService.post(boost::bind(&SerialPort_Private::ExecuteWriteOperation,
			m_private.get(),
			cMsg));
	}
	catch (...)
	{
		return false;
	}

	return true;
}



bool SerialPort::Close() noexcept
{
	try
	{
		m_private->m_oIOService.post(boost::bind(&SerialPort_Private::ExecuteCloseOperation,
			m_private.get(),
			boost::system::error_code()));
	}
	catch (...)
	{
		return false;
	}

	return true;
}


bool SerialPort::IsActive() const
{
	return m_private->m_bActive;
}

