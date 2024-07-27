#include "common.h"
#include "ymodem.h"

static bool receiveTimeout(YModem* modem, uint8_t* buff, size_t recvLen)
{
	size_t   recved    = 0;
	uint32_t start     = modem->Time();
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
	uint8_t  blockNumComplement = ~blockNum;

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

		default:                                    // Unknown control byte
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

	crc = BIG_ENDIAN_16(crc);

	uint16_t calculatedCRC = CRC16(buff, *dataSize, 0);
	if (calculatedCRC ^ crc)
	{
		receiveTimeout(modem, buff, DATA_SIZE); // flush to ensure buffers are empty
		return FAIL;
	}

	return SUCC;
}

static YModemReturn receiveFileName(YModem* modem, char* fileName, size_t* fileSize, void* buff, uint8_t blockNum)
{
	uint16_t     dataSize    = 0;
	YModemReturn returnValue = receivePacket(modem, buff, blockNum, &dataSize);

	if (returnValue == SUCC)
	{
		strcpy(fileName, (char*)buff);
		char* sizeString = (char*)buff + strlen(fileName) + 1;
		*fileSize        = (size_t)atoi(sizeString);
	}

	return returnValue;
}

static YModemReturn receiveData(YModem* modem, char* fileName, FileWrite writeFunc, size_t* remainingData, void* buff, uint8_t blockNum)
{
	uint16_t     dataSize    = 0;
	YModemReturn returnValue = receivePacket(modem, buff, blockNum, &dataSize);

	if (returnValue == SUCC)
	{
		dataSize = (uint16_t)MIN(dataSize, *remainingData);
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
	uint8_t      buff[DATA_SIZE];
	char         fileName[FILE_NAME];
	size_t       remainingData = 0;
	uint8_t      retriesLeft   = MAX_RETRIES;
	YModemReturn returnValue   = SUCC;
	uint8_t      blockNum      = 0;
	uint8_t      controlByte   = 0;

	while (state != END)
	{
		switch (state)
		{
			case START:
				blockNum    = 0;
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
							state       = END;
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
							state       = END;
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
							state       = END;
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
				state       = END;
				break;

			case END:
				break;
		}
	}
	return returnValue;
}
