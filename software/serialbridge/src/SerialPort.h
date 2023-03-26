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

typedef void (*PREADCOMPLETECALLBACK)(	void* pObject,
					const char* pReadMsg,
					size_t nBytesTransferred);

typedef void (*PWRITECOMPLETECALLBACK)(	void* pObject );


using flow_control_t = serial_port_base::flow_control::type;


/** */
class SerialPort
{
	std::unique_ptr<struct SerialPort_Private> const m_private;

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
  bool IsActive() const; 

 private:
  

  bool StartReading();
  bool StartWriting();

  void ReadOperationComplete( const boost::system::error_code& oError,
			      size_t nBytesTransferred);

  void WriteOperationComplete( const boost::system::error_code& oError);

  void ExecuteWriteOperation(const char cMsg);
  void ExecuteCloseOperation(const boost::system::error_code& error);

};


//////////////////////////////////////////////////////////////////////////////

#endif /* CSERIALPORT_H_ */