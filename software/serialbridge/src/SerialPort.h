#ifndef SERIALPORT_H_D7494F87_CA8B_4CD9_AC65_2567099262DC
#define SERIALPORT_H_D7494F87_CA8B_4CD9_AC65_2567099262DC

/**
 * @file		SerialPort.h
 * @created		31.03.2011 
 * @changed		26.03.2023
 * @author		Falk Schilling (db8fs)
 * @copyright	GPLv3
 */

#include <string>
#include <memory>

/** */
class SerialPort
{
	std::shared_ptr<struct SerialPort_Params>  m_params;
	std::shared_ptr<struct SerialPort_Private> m_private;

public:

	class ISerialHandler
	{
	public:
		virtual ~ISerialHandler() {}

		virtual void onSerialConnected() = 0;
		virtual void onSerialReadComplete(const char* msg, size_t length) = 0;
		virtual void onSerialWriteComplete(const char* msg, size_t length) = 0;
	};


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

	/** waits for the given timespan (ms) for availability of the port */
	void awaitConnection(uint16_t waitMs);

	/** starts the communication */
	bool start();

	/** defines asynchronous read or write completion handlers */
	void setHandler(ISerialHandler* const handler);

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

};


//////////////////////////////////////////////////////////////////////////////

#endif /* SERIALPORT_H_D7494F87_CA8B_4CD9_AC65_2567099262DC */
