#include "common.h"
#include "ymodem.h"

#include <assert.h>

static bool         receiveTimeout(const YModem* modem, uint8_t* buff, size_t recvLen);
static YModemReturn receivePacket(const YModem* modem, uint8_t* buff, const uint8_t blockNum, uint16_t* dataSize);
static YModemReturn receiveFileInfo(const YModem* modem, char* fileName, size_t* size, void* buff, const uint8_t blockNum);
static YModemReturn receiveData(const YModem* modem, YModemFile* file, void* buff, const uint8_t blockNum);
static YModemReturn receiveFileEnd(const YModem* modem, uint8_t* buff);

static bool receiveTimeout(const YModem* modem, uint8_t* buff, size_t recvLen)
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

static YModemReturn receivePacket(const YModem* modem, uint8_t* buff, const uint8_t blockNum, uint16_t* dataSize)
{
	uint16_t crc;
	uint8_t  blockNumComplement = ~blockNum;

	// Handle Frame Control
	if (!receiveTimeout(modem, buff, 1))
	{
		return YMODEM_TIMEOUT;
	}
	switch (buff[0])
	{
		case CAN:
			receiveTimeout(modem, buff, DATA_SIZE); // flush to ensure buffers are empty
			return YMODEM_CANCEL;
			break;

		case STX:
			*dataSize = LARGE_PACKET_BYTES;
			break;

		case SOH:
			*dataSize = SMALL_PACKET_BYTES;
			break;

		default:                                    // Unknown control byte
			receiveTimeout(modem, buff, DATA_SIZE); // flush to ensure buffers are empty
			return YMODEM_TIMEOUT;
			break;
	}

	// Handle block numbers
	if (!receiveTimeout(modem, buff, 2))
	{
		return YMODEM_TIMEOUT;
	}
	if (buff[0] != blockNum && buff[1] != blockNumComplement)
	{
		receiveTimeout(modem, buff, DATA_SIZE); // flush to ensure buffers are empty
		return YMODEM_FAIL;
	}

	// Handle data block
	if (!receiveTimeout(modem, buff, *dataSize))
	{
		return YMODEM_TIMEOUT;
	}

	// Handle CRC
	if (!receiveTimeout(modem, (uint8_t*)&crc, 2))
	{
		return YMODEM_TIMEOUT;
	}

	crc = BIG_ENDIAN_16(crc);

	uint16_t calculatedCRC = CRC16(buff, *dataSize, 0);
	if (calculatedCRC ^ crc)
	{
		receiveTimeout(modem, buff, DATA_SIZE); // flush to ensure buffers are empty
		return YMODEM_FAIL;
	}

	return YMODEM_SUCCESS;
}

static YModemReturn receiveFileInfo(const YModem* modem, char* fileName, size_t* size, void* buff, const uint8_t blockNum)
{
	uint16_t     dataSize    = 0;
	YModemReturn returnValue = receivePacket(modem, buff, blockNum, &dataSize);

	if (returnValue == YMODEM_SUCCESS)
	{
		strcpy(fileName, (char*)buff);
		char* sizeString = (char*)buff + strlen(fileName) + 1;
		*size            = (size_t)atoi(sizeString);
	}

	return returnValue;
}

static YModemReturn receiveData(const YModem* modem, YModemFile* file, void* buff, const uint8_t blockNum)
{
	assert(file->Write != NULL);

	uint16_t     dataSize    = 0;
	YModemReturn returnValue = receivePacket(modem, buff, blockNum, &dataSize);

	if (returnValue == YMODEM_SUCCESS)
	{
		dataSize = (uint16_t)MIN(dataSize, file->Size);
		file->Size -= file->Write(buff, dataSize);
	}

	return returnValue;
}

static YModemReturn receiveFileEnd(const YModem* modem, uint8_t* buff)
{
	if (!receiveTimeout(modem, buff, 1))
	{
		return YMODEM_TIMEOUT;
	}

	if (buff[0] == EOT)
	{
		return YMODEM_SUCCESS;
	}

	receiveTimeout(modem, buff, DATA_SIZE); // flush to ensure buffers are empty
	return YMODEM_FAIL;
}

YModemReturn YModem_Receive(const YModem* modem, const YModemFile* files)
{
	assert(modem != NULL);
	assert(files != NULL);

	enum
	{
		START,
		FILENAME,
		DATA,
		FILEDONE,
		CANCELED,
		END
	} state = START;
	uint8_t      buff[DATA_SIZE];
	char         fileName[FILE_NAME];
	uint8_t      retriesLeft = MAX_RETRIES;
	YModemReturn returnValue = YMODEM_SUCCESS;
	uint8_t      blockNum    = 0;
	uint8_t      controlByte = 0;
	YModemFile   currentFile;
	size_t       receivingSize = 0;

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
				switch (receiveFileInfo(modem, fileName, &receivingSize, buff, blockNum))
				{
					case YMODEM_SUCCESS:
						blockNum++;
						retriesLeft = MAX_RETRIES;
						controlByte = ACK;
						modem->Write(&controlByte, 1);

						state = END;
						// Check for valid file and size
						const YModemFile* file = files;
						while (file->Name)
						{
							if (strcmp(fileName, file->Name) == 0)
							{
								if (receivingSize <= file->Size)
								{
									currentFile = *file;
									state       = DATA;
									controlByte = C;
									modem->Write(&controlByte, 1);
									break;
								}
							}
							file++;
						};
						break;

					case YMODEM_FAIL: // no break
					case YMODEM_TIMEOUT:
						if (retriesLeft--)
							state = START;
						else
						{
							returnValue = YMODEM_TIMEOUT;
							state       = END;
						}
						controlByte = NAK;
						modem->Write(&controlByte, 1);
						break;

					case YMODEM_CANCEL:
						state = CANCELED;
						break;
				}
				break;

			case DATA:
				switch (receiveData(modem, &currentFile, buff, blockNum))
				{
					case YMODEM_SUCCESS:
						blockNum++;
						retriesLeft = MAX_RETRIES;
						controlByte = ACK;
						modem->Write(&controlByte, 1);

						state = currentFile.Size ? DATA : FILEDONE;
						break;

					case YMODEM_FAIL: // no break
					case YMODEM_TIMEOUT:
						if (!retriesLeft--)
						{
							returnValue = YMODEM_TIMEOUT;
							state       = END;
						}
						controlByte = NAK;
						modem->Write(&controlByte, 1);
						break;

					case YMODEM_CANCEL:
						state = CANCELED;
						break;
				}
				break;

			case FILEDONE:
				switch (receiveFileEnd(modem, buff))
				{
					case YMODEM_SUCCESS:
						blockNum++;
						retriesLeft = MAX_RETRIES;
						controlByte = ACK;
						modem->Write(&controlByte, 1);

						state = START;
						break;

					case YMODEM_FAIL: // no break
					case YMODEM_TIMEOUT:
						if (!retriesLeft--)
						{
							returnValue = YMODEM_TIMEOUT;
							state       = END;
						}
						controlByte = NAK;
						modem->Write(&controlByte, 1);
						break;

					case YMODEM_CANCEL:
						state = CANCELED;
						break;
				}
				break;

			case CANCELED:
				returnValue = YMODEM_CANCEL;
				state       = END;
				break;

			case END:
				break;
		}
	}
	return returnValue;
}
