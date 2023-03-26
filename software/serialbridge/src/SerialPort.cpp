/**
* @file   SerialPort.cpp
* @date   26.03.2023
* @author Falk Schilling (db8fs)
* @copyright GPLv3
*/

#include "SerialPort.h"


CSerialPort::CSerialPort(	io_service & oIOService,
				unsigned int uiBaudRate,
				flow_control_t eFlowControl,
				const std::string & strDevice
				)
  : m_bActive(true),
    m_oIOService(oIOService),
    m_oSerialPort(oIOService, strDevice),
    m_pReadCompleteCallback(NULL),
    m_pWriteCompleteCallback(NULL),
    m_pReadCompleteCallbackObject(NULL),
    m_pWriteCompleteCallbackObject(NULL)
{
  bool bThrow = true;

  while(1)
    {
      if(!m_oSerialPort.is_open())
	{
	  break;
	}

      serial_port_base::flow_control  oFlowControlOption(eFlowControl);
      serial_port_base::baud_rate 	oBaudRateOption(uiBaudRate);

      m_oSerialPort.set_option(oBaudRateOption);
      m_oSerialPort.set_option(oFlowControlOption);

      if(!StartReading())
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



CSerialPort::~CSerialPort()
{
}



bool CSerialPort::SetReadCompletionCallback(void* pObject,
					    PREADCOMPLETECALLBACK pCallback	)
{
  bool bRet = true;

  if(pCallback)
    {
      m_pReadCompleteCallback 		= pCallback;
      m_pReadCompleteCallbackObject 	= pObject;
    }
  else
    {
      m_pReadCompleteCallback 		= NULL;
      m_pReadCompleteCallbackObject	= NULL;
      bRet = false;
    }

  return bRet;
}



bool CSerialPort::SetWriteCompletionCallback(	void* pObject,
						PWRITECOMPLETECALLBACK pCallback )
{
  bool bRet = true;

  if(pCallback)
    {
      m_pWriteCompleteCallback 		= pCallback;
      m_pWriteCompleteCallbackObject 	= pObject;
    }
  else
    {
      m_pWriteCompleteCallback 		= NULL;
      m_pWriteCompleteCallbackObject 	= NULL;
      bRet = false;
    }

  return bRet;
}


bool CSerialPort::Write(const char cMsg)
{
  bool bRet = true;

  try
    {
      m_oIOService.post(boost::bind(	&CSerialPort::ExecuteWriteOperation,
					this,
					cMsg));
    }
  catch(...)
    {

      bRet = false;
    }

  return bRet;
}



bool CSerialPort::Close()
{
  bool bRet = true;

  try
    {
      m_oIOService.post(boost::bind(	&CSerialPort::ExecuteCloseOperation,
					this,
					boost::system::error_code()));
    }
  catch(...)
    {
      bRet = false;
    }

  return bRet;
}



bool CSerialPort::StartReading()
{
  bool bRet = true;

  try
    {
      m_oSerialPort.async_read_some( 	boost::asio::buffer(m_acReadMsg, RX_BUFFER_SIZE),
					boost::bind(&CSerialPort::ReadOperationComplete,
						    this,
						    placeholders::error,
						    placeholders::bytes_transferred
						    )
					);
    }
  catch(...)
    {
      bRet = false;
    }

  return bRet;
}


bool CSerialPort::StartWriting()
{
  bool bRet = true;

  try
    {
      boost::asio::async_write(	m_oSerialPort,
				boost::asio::buffer(&m_qcWriteMsg.front(), 1),
				boost::bind( 	&CSerialPort::WriteOperationComplete,
						this,
						placeholders::error)
				);
    }
  catch(...)
    {
      bRet = false;
    }

  return bRet;
}



void CSerialPort::ReadOperationComplete(const boost::system::error_code& oError,
					size_t nBytesTransferred)
{
  bool bRet = false;

  while(1)
    {
      if(oError)
	{
	  break;
	}

      if(m_pReadCompleteCallback)
	{
	  m_pReadCompleteCallback(m_pReadCompleteCallbackObject,
				  m_acReadMsg,
				  nBytesTransferred);
	}

      if(!StartReading())
	{
	  break;
	}

      bRet = true;
      break;
    }

  if(!bRet)
    {
      ExecuteCloseOperation(oError);
    }
}



void CSerialPort::WriteOperationComplete(const boost::system::error_code& oError)
{
  bool bRet = false;

  while(1)
    {
      if(oError)
	{
	  break;
	}

      if(m_pWriteCompleteCallback)
	{
	  m_pWriteCompleteCallback(m_pWriteCompleteCallbackObject);
	}

      m_qcWriteMsg.pop_front();

      if(!m_qcWriteMsg.empty())
	{
	  StartWriting();
	}

      bRet = true;
      break;
    }

  if(!bRet)
    {
      ExecuteCloseOperation(oError);
    }
}



void CSerialPort::ExecuteWriteOperation(const char msg)
{
  bool bWriteInProgress = !m_qcWriteMsg.empty();

  m_qcWriteMsg.push_back(msg);

  if(!bWriteInProgress)
    {
      StartWriting();
    }
}



void CSerialPort::ExecuteCloseOperation(const boost::system::error_code& oError)
{
  while(1)
    {
      if(oError == boost::asio::error::operation_aborted)
	{
	  break;
	}

      if(oError)
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


