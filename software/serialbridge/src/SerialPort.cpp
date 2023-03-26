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

    bool 	                 m_bActive;
    io_service            m_oIOService;
    serial_port            m_oSerialPort;
    char 	                 m_acReadMsg[RX_BUFFER_SIZE];
    std::deque<char>       m_qcWriteMsg;
    PREADCOMPLETECALLBACK	 m_pReadCompleteCallback;
    PWRITECOMPLETECALLBACK m_pWriteCompleteCallback;
    void* m_pReadCompleteCallbackObject;
    void* m_pWriteCompleteCallbackObject;

    std::thread m_thread; //< todo


    SerialPort_Private(const std::string & device, uint32_t baudrate, flow_control_t flowControl):
        m_bActive(true),
        m_oIOService(),
        m_oSerialPort(m_oIOService, device),
        m_pReadCompleteCallback(NULL),
        m_pWriteCompleteCallback(NULL),
        m_pReadCompleteCallbackObject(NULL),
        m_pWriteCompleteCallbackObject(NULL),
        m_thread([&]() { while (m_bActive) { m_oIOService.run_for(std::chrono::milliseconds(20)); }})
    {
    }



    bool StartReading()
    {
        bool bRet = true;

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
            bRet = false;
        }

        return bRet;
    }


    bool StartWriting()
    {
        bool bRet = true;

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
            bRet = false;
        }

        return bRet;
    }



    void ReadOperationComplete(const boost::system::error_code& oError,
        size_t nBytesTransferred)
    {
        bool bRet = false;

        while (1)
        {
            if (oError)
            {
                break;
            }

            if (m_pReadCompleteCallback)
            {
                m_pReadCompleteCallback(m_pReadCompleteCallbackObject,
                    m_acReadMsg,
                    nBytesTransferred);
            }

            if (!StartReading())
            {
                break;
            }

            bRet = true;
            break;
        }

        if (!bRet)
        {
            ExecuteCloseOperation(oError);
        }
    }



    void WriteOperationComplete(const boost::system::error_code& oError)
    {
        bool bRet = false;

        while (1)
        {
            if (oError)
            {
                break;
            }

            if (m_pWriteCompleteCallback)
            {
                m_pWriteCompleteCallback(m_pWriteCompleteCallbackObject);
            }

            m_qcWriteMsg.pop_front();

            if (!m_qcWriteMsg.empty())
            {
                StartWriting();
            }

            bRet = true;
            break;
        }

        if (!bRet)
        {
            ExecuteCloseOperation(oError);
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
        while (1)
        {
            if (oError == boost::asio::error::operation_aborted)
            {
                break;
            }

            if (oError)
            {
                std::cerr << "SerialPort Error: " << oError.message() << std::endl;
            }
            else
            {
                m_oSerialPort.close();
                m_bActive = false;
            }
            break;
        }
    }

};




///////////////////////////


SerialPort::SerialPort(	unsigned int uiBaudRate,
				flow_control_t eFlowControl,
				const std::string & strDevice
				)
  : m_private(new SerialPort_Private(strDevice, uiBaudRate, eFlowControl))
{
  bool bThrow = true;

  while(1)
    {
      if(!m_private->m_oSerialPort.is_open())
	{
	  break;
	}

      serial_port_base::flow_control  oFlowControlOption(eFlowControl);
      serial_port_base::baud_rate 	oBaudRateOption(uiBaudRate);

      m_private->m_oSerialPort.set_option(oBaudRateOption);
      m_private->m_oSerialPort.set_option(oFlowControlOption);

      if(!m_private->StartReading())
	{
	  break;
	}

      bThrow = false;
      break;
    }

  if(bThrow)
    {
      throw "Failed to open serial port!\n";
    }
}



SerialPort::~SerialPort()
{
    m_private->m_thread.join();
}



bool SerialPort::SetReadCompletionCallback(void* pObject,
					    PREADCOMPLETECALLBACK pCallback	)
{
  bool bRet = true;

  if(pCallback)
    {
      m_private->m_pReadCompleteCallback 		= pCallback;
      m_private->m_pReadCompleteCallbackObject 	= pObject;
    }
  else
    {
      m_private->m_pReadCompleteCallback 		= NULL;
      m_private->m_pReadCompleteCallbackObject	= NULL;
      bRet = false;
    }

  return bRet;
}



bool SerialPort::SetWriteCompletionCallback(	void* pObject,
						PWRITECOMPLETECALLBACK pCallback )
{
  bool bRet = true;

  if(pCallback)
    {
      m_private->m_pWriteCompleteCallback 		= pCallback;
      m_private->m_pWriteCompleteCallbackObject 	= pObject;
    }
  else
    {
      m_private->m_pWriteCompleteCallback 		= NULL;
      m_private->m_pWriteCompleteCallbackObject 	= NULL;
      bRet = false;
    }

  return bRet;
}


bool SerialPort::Write(const char cMsg)
{
  bool bRet = true;

  try
    {
      m_private->m_oIOService.post(boost::bind(	&SerialPort_Private::ExecuteWriteOperation,
					m_private.get(),
					cMsg));
    }
  catch(...)
    {

      bRet = false;
    }

  return bRet;
}



bool SerialPort::Close()
{
  bool bRet = true;

  try
    {
      m_private->m_oIOService.post(boost::bind(	&SerialPort_Private::ExecuteCloseOperation,
					m_private.get(),
					boost::system::error_code()));
    }
  catch(...)
    {
      bRet = false;
    }

  return bRet;
}


bool SerialPort::IsActive() const 
{ 
    return m_private->m_bActive; 
}

