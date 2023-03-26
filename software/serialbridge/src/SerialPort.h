//////////////////////////////////////////////////////////////////////////////
//! @file 	CSerialPort.h
//! @date	Created on: 31.03.2011
//! @author	Falk Schilling
//////////////////////////////////////////////////////////////////////////////

#ifndef CSERIALPORT_H_
#define CSERIALPORT_H_

//////////////////////////////////////////////////////////////////////////////

#include <string>


/** */
class SerialPort
{
	std::shared_ptr<struct SerialPort_Private> m_private;

public:

	typedef void (*fnReadComplete)(const char* msg, size_t length);
	typedef void (*fnWriteComplete)(const char msg);

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

	/** defines asynchronous read or write completion handlers */
	void setCallbacks(SerialPort::fnReadComplete onReadHandler, SerialPort::fnWriteComplete onWriteHandler);

	/** transmit single character */
	bool send(const char cMsg) noexcept;

	/** transmit text */
	bool send(const std::string& text);

	/** transmit buffer */
	bool send(const uint8_t* const data, size_t length);

	/** closes device */
	bool close() noexcept;

	/** true if still transceiving */
	bool isActive() const;

	/** perform transceive operations (blocking) */
	void update();

	/** perform transceive operations for a timeslice given in milliseconds */
	void update(uint16_t durationMs);

};


//////////////////////////////////////////////////////////////////////////////

#endif /* CSERIALPORT_H_ */
