#include "ymodem_common.h"

#include <stdio.h>

static bool waitStart(YModem* modem)
{
	uint8_t	 byte	   = 0;
	uint32_t start	   = modem->Time();
	uint32_t timeDelta = 0;
	while (byte != C && timeDelta < TIMEOUT_MS)
	{
		modem->Read(&byte, 1);
		timeDelta = modem->Time() - start;
	}

	return byte == C;
}

static uint8_t waitAck(YModem* modem)
{
	uint8_t	 byte	   = 0;
	uint32_t start	   = modem->Time();
	uint32_t timeDelta = 0;
	while (!byte && timeDelta < TIMEOUT_MS)
	{
		modem->Read(&byte, 1);
		timeDelta = modem->Time() - start;
	}

	return byte;
}

static void sendPacket(YModem* modem, uint8_t* buff, uint16_t packetSize, uint8_t blockNum)
{
	uint8_t byte = packetSize == SMALL_PACKET_BYTES ? SOH : STX;
	modem->Write(&byte, 1); // Send packet header
	byte = blockNum;
	modem->Write(&byte, 1); // Send block number
	byte = ~blockNum;
	modem->Write(&byte, 1); // Send block number

	modem->Write(buff, packetSize); // Send Data

	uint16_t crc = CRC16_Calculate(buff, packetSize);
	crc			 = htobe16(crc);
	modem->Write((uint8_t*)&crc, 2); // Send CRC
}

static void sendDone(YModem* modem, uint8_t* buff, uint8_t blockNum)
{
	memset(buff, 0, SMALL_PACKET_BYTES);
	sendPacket(modem, buff, SMALL_PACKET_BYTES, blockNum);
}

static void sendFileName(YModem* modem, char* fileName, uint32_t fileSize, uint8_t* buff, uint8_t blockNum)
{
	uint8_t fileNameLength = strlen(fileName);
	memset(buff, 0, SMALL_PACKET_BYTES);
	memcpy(buff, fileName, fileNameLength);

	sprintf((char*)&buff[fileNameLength + 1], "%d", fileSize);
	sendPacket(modem, buff, SMALL_PACKET_BYTES, blockNum);
}

static uint16_t sendData(YModem* modem, char* fileName, FileRead readFunc, uint8_t* buff, uint32_t offset, uint32_t remainingData, uint8_t blockNum)
{
	uint32_t maxRead   = MIN(remainingData, LARGE_PACKET_BYTES);
	uint32_t readBytes = readFunc(fileName, buff, offset, maxRead);
	sendPacket(modem, buff, readBytes > SMALL_PACKET_BYTES ? LARGE_PACKET_BYTES : SMALL_PACKET_BYTES, blockNum);
	return readBytes;
}

static void sendFileEnd(YModem* modem)
{
	uint8_t byte = EOT;
	modem->Write(&byte, 1);
}

YModemReturn YModem_Transmit(YModem* modem, char* fileNames[], uint32_t sizes[], uint8_t numFiles, FileRead readFunc)
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
	uint32_t	 remainingData = 0;
	uint8_t		 retriesLeft   = MAX_RETRIES;
	YModemReturn returnValue   = SUCC;
	uint8_t		 blockNum	   = 0;
	uint8_t		 fileIndex	   = 0;
	uint16_t	 sentbytes;

	while (state != END)
	{
		switch (state)
		{
			case START:
				if (waitStart(modem))
				{
					blockNum	= 0;
					retriesLeft = MAX_RETRIES;
					state		= FILENAME;
				}
				else if (!retriesLeft--)
				{
					returnValue = TIMEOUT;
					state		= END;
				}
				break;

			case FILENAME:
				if (fileIndex < numFiles)
					sendFileName(modem, fileNames[fileIndex], sizes[fileIndex], buff, blockNum);
				else
					sendDone(modem, buff, blockNum);

				switch (waitAck(modem))
				{
					case ACK:
						waitStart(modem);
						blockNum++;
						remainingData = sizes[fileIndex];
						retriesLeft	  = MAX_RETRIES;
						state		  = fileIndex < numFiles ? DATA : END;
						break;
					case CAN:
						state = CANCLED;
						break;
					default: //NAK or timeout
						if (!retriesLeft--)
						{
							returnValue = TIMEOUT;
							state		= END;
						}
						break;
				}
				break;

			case DATA:
				sentbytes = sendData(modem, fileNames[fileIndex], readFunc, buff, sizes[fileIndex] - remainingData, remainingData, blockNum);
				switch (waitAck(modem))
				{
					case ACK:
						remainingData -= sentbytes;
						blockNum++;
						state		= remainingData ? DATA : FILEDONE;
						retriesLeft = MAX_RETRIES;
						break;
					case CAN:
						state = CANCLED;
						break;
					default: //NAK or timeout
						if (!retriesLeft--)
						{
							returnValue = TIMEOUT;
							state		= END;
						}
						break;
				}
				break;

			case FILEDONE:
				sendFileEnd(modem);
				switch (waitAck(modem))
				{
					case ACK:
						fileIndex++;
						state		= START;
						retriesLeft = MAX_RETRIES;
						break;
					case CAN:
						state = CANCLED;
						break;
					default: //NAK or timeout
						if (!retriesLeft--)
						{
							returnValue = TIMEOUT;
							state		= END;
						}
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
