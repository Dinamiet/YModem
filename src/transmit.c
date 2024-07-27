#include "common.h"
#include "ymodem.h"

#include <stdio.h>

static bool    waitStart(YModem* modem);
static uint8_t waitAck(YModem* modem);
static void    sendPacket(YModem* modem, uint8_t* buff, uint16_t packetSize, uint8_t blockNum);
static void    sendDone(YModem* modem, uint8_t* buff, uint8_t blockNum);
static void    sendFileInfo(YModem* modem, YModemFile* file, void* buff, uint8_t blockNum);
static size_t  sendData(YModem* modem, YModemFile* file, void* buff, uint8_t blockNum);
static void    sendFileEnd(YModem* modem);

static bool waitStart(YModem* modem)
{
	uint8_t  byte      = 0;
	uint32_t start     = modem->Time();
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
	uint8_t  byte      = 0;
	uint32_t start     = modem->Time();
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

	uint16_t crc = CRC16(buff, packetSize, 0);
	crc          = BIG_ENDIAN_16(crc);
	modem->Write((uint8_t*)&crc, 2); // Send CRC
}

static void sendDone(YModem* modem, uint8_t* buff, uint8_t blockNum)
{
	memset(buff, 0, SMALL_PACKET_BYTES);
	sendPacket(modem, buff, SMALL_PACKET_BYTES, blockNum);
}

static void sendFileInfo(YModem* modem, YModemFile* file, void* buff, uint8_t blockNum)
{
	size_t fileNameLength = strlen(file->Name);
	memset(buff, 0, SMALL_PACKET_BYTES);
	memcpy(buff, file->Name, fileNameLength);

	sprintf((char*)buff + fileNameLength + 1, "%d", (int)file->Size);
	sendPacket(modem, buff, SMALL_PACKET_BYTES, blockNum);
}

static size_t sendData(YModem* modem, YModemFile* file, void* buff, uint8_t blockNum)
{
	size_t maxRead   = (size_t)MIN(file->Size, LARGE_PACKET_BYTES);
	size_t readBytes = file->Read(buff, maxRead);
	sendPacket(modem, buff, readBytes > SMALL_PACKET_BYTES ? LARGE_PACKET_BYTES : SMALL_PACKET_BYTES, blockNum);
	return readBytes;
}

static void sendFileEnd(YModem* modem)
{
	uint8_t byte = EOT;
	modem->Write(&byte, 1);
}

YModemReturn YModem_Transmit(YModem* modem, YModemFile* files)
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
	size_t       remainingData = 0;
	uint8_t      retriesLeft   = MAX_RETRIES;
	YModemReturn returnValue   = YMODEM_SUCCESS;
	uint8_t      blockNum      = 0;
	YModemFile*  currentFile   = files;
	size_t       sentbytes;

	while (state != END)
	{
		switch (state)
		{
			case START:
				if (waitStart(modem))
				{
					blockNum    = 0;
					retriesLeft = MAX_RETRIES;
					state       = FILENAME;
				}
				else if (!retriesLeft--)
				{
					returnValue = YMODEM_TIMEOUT;
					state       = END;
				}
				break;

			case FILENAME:
				if (currentFile->Name)
					sendFileInfo(modem, currentFile, buff, blockNum);
				else
				{
					sendDone(modem, buff, blockNum);
					state = END;
					break;
				}

				switch (waitAck(modem))
				{
					case ACK:
						waitStart(modem);
						blockNum++;
						remainingData = currentFile->Size;
						retriesLeft   = MAX_RETRIES;
						state         = currentFile->Name ? DATA : END;
						break;
					case CAN:
						state = CANCLED;
						break;
					default: // NAK or timeout
						if (!retriesLeft--)
						{
							returnValue = YMODEM_TIMEOUT;
							state       = END;
						}
						break;
				}
				break;

			case DATA:
				sentbytes = sendData(modem, currentFile, buff, blockNum);
				switch (waitAck(modem))
				{
					case ACK:
						remainingData -= sentbytes;
						blockNum++;
						state       = remainingData ? DATA : FILEDONE;
						retriesLeft = MAX_RETRIES;
						break;
					case CAN:
						state = CANCLED;
						break;
					default: // NAK or timeout
						if (!retriesLeft--)
						{
							returnValue = YMODEM_TIMEOUT;
							state       = END;
						}
						break;
				}
				break;

			case FILEDONE:
				sendFileEnd(modem);
				switch (waitAck(modem))
				{
					case ACK:
						currentFile++;
						state       = START;
						retriesLeft = MAX_RETRIES;
						break;
					case CAN:
						state = CANCLED;
						break;
					default: // NAK or timeout
						if (!retriesLeft--)
						{
							returnValue = YMODEM_TIMEOUT;
							state       = END;
						}
						break;
				}
				break;

			case CANCLED:
				returnValue = YMODEM_CANCEL;
				state       = END;
				break;

			case END:
				break;
		}
	}

	return returnValue;
}
