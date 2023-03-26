//////////////////////////////////////////////////////////////////////////////
//! @file 	CSerialPort.h
//! @date	Created on: 31.03.2011
//! @author	Falk Schilling
//////////////////////////////////////////////////////////////////////////////

#ifndef CSERIALPORT_H_
#define CSERIALPORT_H_

//////////////////////////////////////////////////////////////////////////////

typedef void (*fnReadComplete)(void* pObject, const char* pReadMsg, size_t nBytesTransferred);
typedef void (*fnWriteComplete)(void* pObject );


/** */
class SerialPort
{
	std::unique_ptr<struct SerialPort_Private> const m_private;

 public:

	 enum class eFlowControl : uint8_t
	 {
		 None = 0,
		 Hardware = 1,
		 Software = 2
	 };


  SerialPort(	const std::string& device,
				uint32_t baudRate,
				SerialPort::eFlowControl flowControl );

  ~SerialPort() noexcept;

  /** defines asynchronous read completion handler */
  bool setReadCompleteHandler(	void* pObject,
						fnReadComplete pCallback	);

  /** defines asynchronous write completion handler */
  bool setWriteCompleteHandler( void* pObject,
					   fnWriteComplete pCallback );

  bool write(const char cMsg) noexcept;
  bool close() noexcept;
  bool isActive() const; 

};


//////////////////////////////////////////////////////////////////////////////

#endif /* CSERIALPORT_H_ */
