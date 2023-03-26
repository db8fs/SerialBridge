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

class SerialPort
{
 public:
  typedef serial_port_base::flow_control::type flow_control_t;

 public:
  SerialPort(	unsigned int uiBaudRate,
				flow_control_t eFlowControl,
				const std::string& strDevice );

  ~SerialPort();

  /** defines asynchronous read completion handler */
  bool SetReadCompletionCallback(	void* pObject,
						PREADCOMPLETECALLBACK pCallback	);

  /** defines asynchronous write completion handler */
  bool SetWriteCompletionCallback( void* pObject,
					   PWRITECOMPLETECALLBACK pCallback );

  bool Write(const char cMsg);
  bool Close();
  bool IsActive() const { return m_bActive; }

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
  io_service            m_oIOService;
  serial_port            m_oSerialPort;
  char 	                 m_acReadMsg[RX_BUFFER_SIZE];
  std::deque<char>       m_qcWriteMsg;
  PREADCOMPLETECALLBACK	 m_pReadCompleteCallback;
  PWRITECOMPLETECALLBACK m_pWriteCompleteCallback;
  void*			 m_pReadCompleteCallbackObject;
  void*			 m_pWriteCompleteCallbackObject;

  std::thread m_thread;
};


//////////////////////////////////////////////////////////////////////////////

#endif /* CSERIALPORT_H_ */
