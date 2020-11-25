#ifndef _VESCUART_h
#define _VESCUART_h

#include <Arduino.h>
#include "datatypes.h"
#include "buffer.h"
#include "crc.h"

class VescUart
{
	/** Struct to store the telemetry data returned by the VESC */
	struct dataPackage
	{
		///float avgMotorCurrent;
		float avgInputCurrent;
		///float dutyCycleNow;
		long erpm;
		float inpVoltage;
		///float ampHours;
		///float ampHoursCharged;
		float wattHours;
		float wattHoursCharged;
		///long tachometer;
		long tachometerAbs;
	};

	public:
		VescUart(void);

		///Variabel to hold measurements returned from VESC
		dataPackage data;

        ///Set the serial port for uart communication
		void setSerialPort(HardwareSerial* port);

		bool getVescValues(uint8_t canID);

	private:
		///Variabel to hold the reference to the Serial object to use for UART
		HardwareSerial* serialPort = NULL;

		///Variabel to hold the reference to the Serial object to use for debugging.
        ///Uses the class Stream instead of HarwareSerial
		//Stream* debugPort = NULL;

        ///payload  - The payload as a unit8_t Array with length of int lenPayload
        ///lenPay   - Length of payload
        ///The number of bytes send
		int packSendPayload(uint8_t * payload, int lenPay);

		///payloadReceived  - The received payload as a unit8_t Array
        ///The number of bytes received within the payload
		int receiveUartMessage(uint8_t * payloadReceived);

        ///Verifies the message (CRC-16) and extracts the payload
		///message  - The received UART message
		///lenMes   - The lenght of the message
		///payload  - The final payload ready to extract data from
        ///True if the process was a success
		bool unpackPayload(uint8_t * message, int lenMes, uint8_t * payload);

		///brief      Extracts the data from the received payload
		///param      message  - The payload to extract data from
		///return     True if the process was a success
		bool processReadPacket(uint8_t * message);

};

#endif
