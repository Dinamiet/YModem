#include "ymodem.h"

#include <stdbool.h>
#include <stdio.h>

#define STX 0x02 // 1024 byte data block
#define SOH 0x01 // 128 byte data block
#define EOT 0x04 // End of transfer
#define ACK 0x06 // Acknownledged, continue transfer
#define NAK 0x15 // Not Acknownledged, retry transfer
#define CAN 0x18 // Cancel transmission
#define C	0x43 // Ready to receive data

#define RETRIES 10	 // number of attempts
#define TIMEOUT 1000 // ms

#define DATA_SIZE		   1024
#define FILE_NAME		   32
#define LARGE_PACKET_BYTES 1024
#define SMALL_PACKET_BYTES 128

#define HEADER_SIZE sizeof(PacketHeader)
#define CRC_SIZE	2

typedef struct
{
	uint8_t Type;
	uint8_t BlockNumber;
	uint8_t BlockNumberComplement;
} PacketHeader;

typedef enum
{
	FILENAME,
	DATA,
	DONE
} TransferState;

typedef enum
{
	INIT,
	WAIT_HEADER,
	WAIT_DATA,
	WAIT_CRC,
	SEND_NACK,
	SEND_ACK
} ReceiveStates;

static bool receiveTimeout(YModem* modem, uint8_t* buff, uint32_t recvLen)
{
	uint32_t recved	   = 0;
	uint32_t start	   = modem->Time();
	uint32_t timeDelta = 0;
	while (recved != recvLen && timeDelta < TIMEOUT)
	{
		recved	  = modem->Read(buff, recvLen - recved);
		timeDelta = modem->Time() - start;
	}

	return recved == recvLen;
}

void YModem_Init(YModem* modem, InterfaceRead readFunc, InterfaceWrite writeFunc, Timestamp timeFunc)
{
	modem->Read	 = readFunc;
	modem->Write = writeFunc;
	modem->Time	 = timeFunc;
}

YModemReturn YModem_Receive(YModem* modem, FileWrite writeFunc)
{
	ReceiveStates state		  = INIT;
	YModemReturn  returnValue = OK;

	TransferState transfer;
	char*		  fileName[FILE_NAME];
	uint32_t	  fileSize = 0;
	uint8_t		  retriesRemaining;
	uint8_t		  controlByte;
	uint32_t	  blockCounter;
	uint16_t	  dataLen = 0;
	uint8_t		  buffer[DATA_SIZE + sizeof(PacketHeader) + sizeof(uint16_t)];
	PacketHeader* header = buffer;
	uint8_t*	  data	 = &buffer[HEADER_SIZE];
	uint16_t*	  crc;

	retriesRemaining = RETRIES;
	while (true)
	{
		switch (state)
		{
			case INIT:
				controlByte	 = C;
				blockCounter = 0;
				transfer = FILENAME;
				modem->Write(&controlByte, 1);
				state = WAIT_HEADER;
				break;

			case WAIT_HEADER:
				header->Type = C;
				if (receiveTimeout(modem, header, HEADER_SIZE))
				{
					dataLen = header->Type == STX ? LARGE_PACKET_BYTES : SMALL_PACKET_BYTES;
					state	= WAIT_DATA;
				}
				else if (header->Type == EOT) // transfer done
					state = INIT;			  // start next transfer
				else if (header->Type == CAN) // transfer cancled
					return CANCLED;
				else if (retriesRemaining-- == 0) // receive timeout - there is no more retries left
					return TIMEOUT;
				else												 // receive timeout - retry
					state = transfer == FILENAME ? INIT : SEND_NACK; // if filename is was not yet received - continue to init else send NACK

				break;

			case WAIT_DATA:
				if (receiveTimeout(modem, data, dataLen))
				{
					crc	  = &data[dataLen];
					state = WAIT_CRC;
				}
				else
					state = SEND_NACK;
				break;

			case WAIT_CRC:
				if (receiveTimeout(modem, crc, CRC_SIZE))
				{
					// TODO: Check CRC and block counters - if fail send NACK
					state = SEND_ACK;
				}
				else
					state = SEND_NACK;

				break;

			case SEND_NACK:
				controlByte = NAK;
				modem->Write(&controlByte, 1);
				state = WAIT_HEADER;
				break;

			case SEND_ACK:
				if (transfer == FILENAME)
				{
					// TODO: capture filename and size
					transfer= DATA;
				}
				else if (transfer == DATA)
				{
					// TODO: Write data to file
				}
				controlByte = ACK;
				modem->Write(&controlByte, 1);
				retriesRemaining = RETRIES;
				state			 = WAIT_HEADER;
				break;

			default:
				return UNKNOWN_STATE;
				break;
		}
	}
	return returnValue;
}

YModemReturn YModem_Transmit(YModem* modem, FileRead readFunc, uint32_t size) { return OK; }
