//////////////////////////////////////////////////////////////////////////////
//! @file 	CSerialPort.h
//! @date	Created on: 31.03.2011
//! @author	Falk Schilling
//////////////////////////////////////////////////////////////////////////////

#ifndef CSERIALPORT_H_
#define CSERIALPORT_H_

//////////////////////////////////////////////////////////////////////////////


/** */
class SerialPort
{
	std::shared_ptr<struct SerialPort_Private> m_private;

public:

	typedef void (*fnReadComplete)(const char* pReadMsg, size_t nBytesTransferred);
	typedef void (*fnWriteComplete)();

	/** possible handshake settings for serial connections */
	enum class eFlowControl : uint8_t
	{
		None = 0,
		Hardware = 1,
		Software = 2
	};

	/** connects to given serial port device (e.g. \\.\COM1, /dev/ttyUSB0, /dev/cu0) with the given USART parameters (e.g. 115200, NoFlowControl) */
	SerialPort(const std::string& device, uint32_t baudRate, SerialPort::eFlowControl flowControl);

	SerialPort(const SerialPort&);
	~SerialPort() noexcept;

	SerialPort& operator=(const SerialPort&);

	/** defines asynchronous read completion handler */
	bool setReadCompleteHandler(fnReadComplete pCallback);

	/** defines asynchronous write completion handler */
	bool setWriteCompleteHandler(fnWriteComplete pCallback);

	/** tx single character */
	bool write(const char cMsg) noexcept;

	/** closes device */
	bool close() noexcept;

	/** true if still transceiving */
	bool isActive() const;

};


//////////////////////////////////////////////////////////////////////////////

#endif /* CSERIALPORT_H_ */
