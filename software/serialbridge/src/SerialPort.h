//////////////////////////////////////////////////////////////////////////////
//! @file 	CSerialPort.h
//! @date	Created on: 31.03.2011
//! @author	Falk Schilling
//////////////////////////////////////////////////////////////////////////////

#ifndef CSERIALPORT_H_
#define CSERIALPORT_H_

//////////////////////////////////////////////////////////////////////////////

#include <deque>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

//////////////////////////////////////////////////////////////////////////////

using namespace boost::asio;

//////////////////////////////////////////////////////////////////////////////

typedef void (*PREADCOMPLETECALLBACK)(	void* pObject,
					const char* pReadMsg,
					size_t nBytesTransferred);

typedef void (*PWRITECOMPLETECALLBACK)(	void* pObject );

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//! This class realizes the communication through a serial port interface.
//////////////////////////////////////////////////////////////////////////

class CSerialPort
{
 public:
  typedef serial_port_base::flow_control::type flow_control_t;

 public:
  CSerialPort(io_service& oIOService,
	      unsigned int uiBaudRate,
	      flow_control_t eFlowControl,
	      const std::string& strDevice );

  virtual ~CSerialPort();

  virtual bool SetReadCompletionCallback(	void* pObject,
						PREADCOMPLETECALLBACK pCallback	);
  virtual bool SetWriteCompletionCallback( void* pObject,
					   PWRITECOMPLETECALLBACK pCallback );

  virtual bool Write(const char cMsg);
  virtual bool Close();
  virtual bool IsActive() const { return m_bActive; }

 private:
  static const int RX_BUFFER_SIZE = 512;

  bool StartReading();
  bool StartWriting();

  void ReadOperationComplete( const boost::system::error_code& oError,
			      size_t nBytesTransferred);

  void WriteOperationComplete( const boost::system::error_code& oError);

  void ExecuteWriteOperation(const char cMsg);
  void ExecuteCloseOperation(const boost::system::error_code& error);

 private:
  bool 	                 m_bActive;
  io_service&            m_oIOService;
  serial_port            m_oSerialPort;
  char 	                 m_acReadMsg[RX_BUFFER_SIZE];
  std::deque<char>       m_qcWriteMsg;
  PREADCOMPLETECALLBACK	 m_pReadCompleteCallback;
  PWRITECOMPLETECALLBACK m_pWriteCompleteCallback;
  void*			 m_pReadCompleteCallbackObject;
  void*			 m_pWriteCompleteCallbackObject;
};


//////////////////////////////////////////////////////////////////////////////

#endif /* CSERIALPORT_H_ */
