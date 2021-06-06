#include "ymodem.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STX 0x02 // 1024 byte data block
#define SOH 0x01 // 128 byte data block
#define EOT 0x04 // End of transfer
#define ACK 0x06 // Acknownledged, continue transfer
#define NAK 0x15 // Not Acknownledged, retry transfer
#define CAN 0x18 // Cancel transmission
#define C	0x43 // Ready to receive data

#define MAX_RETRIES 10	 // number of attempts
#define TIMEOUT_MS	1000 // ms

#define DATA_SIZE		   1024
#define FILE_NAME		   32
#define LARGE_PACKET_BYTES 1024
#define SMALL_PACKET_BYTES 128

#define HEADER_SIZE sizeof(PacketHeader)
#define CRC_SIZE	2

#define MIN(x, y) (x > y ? y : x)

static bool receiveTimeout(YModem* modem, uint8_t* buff, uint32_t recvLen)
{
	uint16_t recved	   = 0;
	uint32_t start	   = modem->Time();
	uint32_t timeDelta = 0;
	while (recved != recvLen && timeDelta < TIMEOUT_MS)
	{
		recved += modem->Read(&buff[recved], recvLen - recved);
		timeDelta = modem->Time() - start;
	}

	return recved == recvLen;
}

static YModemReturn receivePacket(YModem* modem, uint8_t* buff, uint8_t blockNum, uint16_t* dataSize)
{
	uint16_t crc;
	uint8_t	 blockNumComplement = ~blockNum;

	// Handle Frame Control
	if (!receiveTimeout(modem, buff, 1))
	{
		return TIMEOUT;
	}
	switch (buff[0])
	{
		case CAN:
			receiveTimeout(modem, buff, DATA_SIZE); // flush to ensure buffers are empty
			return CANCLE;
			break;

		case STX:
			*dataSize = LARGE_PACKET_BYTES;
			break;

		case SOH:
			*dataSize = SMALL_PACKET_BYTES;
			break;

		default: // Unknown control byte
			receiveTimeout(modem, buff, DATA_SIZE); // flush to ensure buffers are empty
			return TIMEOUT;
			break;
	}

	// Handle block numbers
	if (!receiveTimeout(modem, buff, 2))
	{
		return TIMEOUT;
	}
	if (buff[0] != blockNum && buff[1] != blockNumComplement)
	{
		receiveTimeout(modem, buff, DATA_SIZE); // flush to ensure buffers are empty
		return FAIL;
	}

	// Handle data block
	if (!receiveTimeout(modem, buff, *dataSize))
	{
		return TIMEOUT;
	}

	// Handle CRC
	if (!receiveTimeout(modem, (uint8_t*)&crc, 2))
	{
		return TIMEOUT;
	}
	uint16_t calculatedCRC = crc ^ crc; // TODO: Calculate actual CRC
	if (calculatedCRC)
	{
		receiveTimeout(modem, buff, DATA_SIZE); // flush to ensure buffers are empty
		return FAIL;
	}

	return SUCC;
}

static YModemReturn receiveFileName(YModem* modem, char* fileName, uint16_t* fileSize, uint8_t* buff, uint8_t blockNum)
{
	uint16_t	 dataSize	 = 0;
	YModemReturn returnValue = receivePacket(modem, buff, blockNum, &dataSize);

	if (returnValue == SUCC)
	{
		strcpy(fileName, (char*)buff);
		char* sizeString = (char*)buff + strlen(fileName) + 1;
		*fileSize		 = atoi(sizeString);
	}

	return returnValue;
}

static YModemReturn receiveData(YModem* modem, char* fileName, FileWrite writeFunc, uint16_t* remainingData, uint8_t* buff, uint8_t blockNum)
{
	uint16_t	 dataSize	 = 0;
	YModemReturn returnValue = receivePacket(modem, buff, blockNum, &dataSize);

	if (returnValue == SUCC)
	{
		dataSize = MIN(dataSize, *remainingData);
		writeFunc(fileName, buff, dataSize);
		*remainingData -= dataSize;
	}

	return returnValue;
}

static YModemReturn receiveFileEnd(YModem* modem, uint8_t* buff)
{
	if (!receiveTimeout(modem, buff, 1))
	{
		return TIMEOUT;
	}

	if (buff[0] == EOT)
	{
		return SUCC;
	}

	receiveTimeout(modem, buff, DATA_SIZE); // flush to ensure buffers are empty
	return FAIL;
}

void YModem_Init(YModem* modem, InterfaceRead readFunc, InterfaceWrite writeFunc, Timestamp timeFunc)
{
	modem->Read	 = readFunc;
	modem->Write = writeFunc;
	modem->Time	 = timeFunc;
}

YModemReturn YModem_Receive(YModem* modem, FileWrite writeFunc)
{
	enum
	{
		START,
		FILENAME,
		DATA,
		FILEDONE,
		CANCLED,
		END
	} state = START;
	uint8_t		 buff[DATA_SIZE];
	char		 fileName[FILE_NAME];
	uint16_t	 remainingData = 0;
	uint8_t		 retriesLeft   = MAX_RETRIES;
	YModemReturn returnValue   = SUCC;
	uint8_t		 blockNum	   = 0;
	uint8_t		 controlByte   = 0;

	while (state != END)
	{
		switch (state)
		{
			case START:
				blockNum	= 0;
				controlByte = C;
				modem->Write(&controlByte, 1);
				state = FILENAME;
				break;

			case FILENAME:
				switch (receiveFileName(modem, fileName, &remainingData, buff, blockNum))
				{
					case SUCC:
						blockNum++;
						retriesLeft = MAX_RETRIES;
						controlByte = ACK;
						modem->Write(&controlByte, 1);

						state = strlen(fileName) ? DATA : END;
						if (state == DATA)
						{
							controlByte = C;
							modem->Write(&controlByte, 1);
						}
						break;

					case FAIL: // no break
					case TIMEOUT:
						if (retriesLeft--)
							state = START;
						else
						{
							returnValue = TIMEOUT;
							state		= END;
						}
						controlByte = NAK;
						modem->Write(&controlByte, 1);
						break;

					case CANCLE:
						state = CANCLED;
						break;
				}
				break;

			case DATA:
				switch (receiveData(modem, fileName, writeFunc, &remainingData, buff, blockNum))
				{
					case SUCC:
						blockNum++;
						retriesLeft = MAX_RETRIES;
						controlByte = ACK;
						modem->Write(&controlByte, 1);

						state = remainingData ? DATA : FILEDONE;
						break;

					case FAIL: // no break
					case TIMEOUT:
						if (!retriesLeft--)
						{
							returnValue = TIMEOUT;
							state		= END;
						}
						controlByte = NAK;
						modem->Write(&controlByte, 1);
						break;

					case CANCLE:
						state = CANCLED;
						break;
				}
				break;

			case FILEDONE:
				switch (receiveFileEnd(modem, buff))
				{
					case SUCC:
						blockNum++;
						retriesLeft = MAX_RETRIES;
						controlByte = ACK;
						modem->Write(&controlByte, 1);

						state = START;
						break;

					case FAIL: // no break
					case TIMEOUT:
						if (!retriesLeft--)
						{
							returnValue = TIMEOUT;
							state		= END;
						}
						controlByte = NAK;
						modem->Write(&controlByte, 1);
						break;

					case CANCLE:
						state = CANCLED;
						break;
				}
				break;

			case CANCLED:
				returnValue = CANCLE;
				state		= END;
				break;

			case END:
				break;
		}
	}
	return returnValue;
}

YModemReturn YModem_Transmit(YModem* modem, FileRead readFunc, uint32_t size)
{
	(void)modem;
	(void)readFunc;
	(void)size;
	return SUCC;
}
