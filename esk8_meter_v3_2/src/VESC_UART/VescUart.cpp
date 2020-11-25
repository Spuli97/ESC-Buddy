#include "VescUart.h"
#include <HardwareSerial.h>

VescUart::VescUart(void)
{
    //constructor
    ///float avgMotorCurrent;
    float avgInputCurrent       = 0.0;
    ///float dutyCycleNow;
    long erpm                   = 0;
    float inpVoltage            = 0.0;
    ///float ampHours;
    ///float ampHoursCharged;
    float wattHours             = 0.0;
    float wattHoursCharged      = 0.0;
    ///long tachometer;
    long tachometerAbs          = 0;
}

void VescUart::setSerialPort(HardwareSerial* port)
{
	serialPort = port;
}

int VescUart::receiveUartMessage(uint8_t * payloadReceived)
{
	// Messages <= 255 starts with "2", 2nd byte is length
	// Messages > 255 starts with "3" 2nd and 3rd byte is length combined with 1st >>8 and then &0xFF

	uint16_t counter = 0;
	uint16_t endMessage = 256;
	bool messageRead = false;
	uint8_t messageReceived[256];
	uint16_t lenPayload = 0;

	uint32_t timeout = millis() + 100; // Defining the timestamp for timeout (100ms before timeout)

	while ( millis() < timeout && messageRead == false)
    {
		while (serialPort->available())
		{
			messageReceived[counter++] = serialPort->read();

			if (counter == 2)
            {
				switch (messageReceived[0])
				{
					case 2:
						endMessage = messageReceived[1] + 5; //Payload size + 2 for sice + 3 for SRC and End.
						lenPayload = messageReceived[1];
					break;

					case 3:
						// ToDo: Add Message Handling > 255 (starting with 3)
						//Message is larger than 256 bytes - not supported
					break;

					default:
						//Invalid start bit
					break;
				}
			}

			if (counter >= sizeof(messageReceived))
				break;

			if (counter == endMessage && messageReceived[endMessage - 1] == 3)
            {
				messageReceived[endMessage] = 0;
				//End of message reached
				messageRead = true;
				break; // Exit if end of message is reached, even if there is still more data in the buffer.
			}
		}
	}

	///if(messageRead == false)
		///Timeout condition, do something?

	bool unpacked = false;

	if (messageRead)
    {
		unpacked = unpackPayload(messageReceived, endMessage, payloadReceived);
	}

	if (unpacked)
    {
		// Message was read
		return lenPayload;
	}
	else
	{
		// No Message Read
		return 0;
	}
}


bool VescUart::unpackPayload(uint8_t * message, int lenMes, uint8_t * payload)
{

	uint16_t crcMessage = 0;
	uint16_t crcPayload = 0;

	// Rebuild crc:
	crcMessage = message[lenMes - 3] << 8;
	crcMessage &= 0xFF00;
	crcMessage += message[lenMes - 2];

	// Extract payload:
	memcpy(payload, &message[2], message[1]);
	crcPayload = crc16(payload, message[1]);

	if (crcPayload == crcMessage)
    {
		return true;
	}
	else
    {
		return false;
	}
}


int VescUart::packSendPayload(uint8_t * payload, int lenPay)
{

	uint16_t crcPayload = crc16(payload, lenPay);
	int count = 0;
	uint8_t messageSend[256];

	if (lenPay <= 256)
	{
		messageSend[count++] = 2;
		messageSend[count++] = lenPay;
	}
	else
	{
		messageSend[count++] = 3;
		messageSend[count++] = (uint8_t)(lenPay >> 8);
		messageSend[count++] = (uint8_t)(lenPay & 0xFF);
	}

	memcpy(&messageSend[count], payload, lenPay);

	count += lenPay;
	messageSend[count++] = (uint8_t)(crcPayload >> 8);
	messageSend[count++] = (uint8_t)(crcPayload & 0xFF);
	messageSend[count++] = 3;
	messageSend[count] = '\0';

	// Sending package
	serialPort->write(messageSend, count);

	// Returns number of send bytes
	return count;
}


bool VescUart::processReadPacket(uint8_t * message)
{

	COMM_PACKET_ID packetId;
	int32_t ind = 0;

	packetId = (COMM_PACKET_ID)message[0];
	message++; // Removes the packetId from the actual message (payload)

	switch (packetId)
	{
		case COMM_GET_VALUES: // Structure defined here: https://github.com/vedderb/bldc/blob/43c3bbaf91f5052a35b75c2ff17b5fe99fad94d1/commands.c#L164

			ind = 8; // Skip the first 4+4 bytes (temperature data), avgMotorCurrent
			///data.avgMotorCurrent 	= buffer_get_float32(message, 100.0, &ind);
			data.avgInputCurrent 	= buffer_get_float32(message, 100.0, &ind);
			ind += 10; // Skip the next 8+2 bytes (avg_id, avg_iq), dutyCycleNow
			///data.dutyCycleNow 		= buffer_get_float16(message, 1000.0, &ind);
			data.erpm 				= buffer_get_int32(message, &ind);
			data.inpVoltage 		= buffer_get_float16(message, 10.0, &ind);
			ind += 8; // Skip the next 8 bytes: ampHours, ampHoursCharged
			///data.ampHours 			= buffer_get_float32(message, 10000.0, &ind);
			///data.ampHoursCharged 	= buffer_get_float32(message, 10000.0, &ind);
            data.wattHours 			= buffer_get_float32(message, 10000.0, &ind);
            data.wattHoursCharged 	= buffer_get_float32(message, 10000.0, &ind);
            ind += 4; // Skip the next 4 bytes, tachometer
			///data.tachometer 		= buffer_get_int32(message, &ind);
			data.tachometerAbs 		= buffer_get_int32(message, &ind);

			return true;

		break;

		default:
			return false;
		break;
	}
}

bool VescUart::getVescValues(uint8_t canID)
{
  
  uint8_t payload[256];

  if (canID == 0)
  {
      uint8_t command[1] = { COMM_GET_VALUES };
      packSendPayload(command, 1);
      // delay(1); //needed, otherwise data is not read?
  }
  else
  {
      uint8_t command[3];
      command[0] = COMM_FORWARD_CAN;
      command[1] = canID;
      command[2] = COMM_GET_VALUES;
      packSendPayload(command, 3);
      // delay(1); //needed, otherwise data is not read?
  }


  int lenPayload = receiveUartMessage(payload);

  if (lenPayload > 55)
    {
    bool read = processReadPacket(payload); //returns true if successful
    return read;
  }
  else
  {
    return false;
  }
  
}

