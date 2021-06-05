#include "ymodem.h"

#include <stdio.h>
#include <stdbool.h>

#define STX 0x02 // 1024 byte data block
#define SOH 0x01 // 128 byte data block
#define EOT 0x04 // End of transfer
#define ACK 0x06 // Acknownledged, continue transfer
#define NAK 0x15 // Not Acknownledged, retry transfer
#define CAN 0x18 // Cancel transmission
#define C	0x43 // Ready to receive data

#define RETRIES 10 // number of attempts
#define TIMEOUT 1000  // ms

#define DATA_SIZE		   1024
#define LARGE_PACKET_BYTES 1024
#define SMALL_PACKET_BYTES 128

typedef struct
{
	uint8_t Type;
	uint8_t BlockNumber;
	uint8_t BlockNumberComplement;
} PacketHeader;

typedef enum
{
	INIT,
	WAIT_FIRST_HEADER,
	WAIT_HEADER,
	WAIT_DATA,
	WAIT_CRC,
	SEND_NACK,
	SEND_ACK

} ReceiveStates;

static bool receiveTimeout(YModem* modem, uint8_t* buff, uint32_t recvLen)
{	
	uint32_t recved= 0;
	uint32_t start= modem->Wait();
	uint32_t timeDelta= 0;
	while (recved != recvLen && timeDelta < TIMEOUT)
		recved= modem->Read(buff, recvLen-recved);
	
	return recved == recvLen;
}

void YModem_Init(YModem* modem, InterfaceRead readFunc, InterfaceWrite writeFunc, Sleep waitFunc)
{
	modem->Read	 = readFunc;
	modem->Write = writeFunc;
	modem->Wait	 = waitFunc;
}

YModemReturn YModem_Receive(YModem* modem, FileWrite writeFunc)
{
	ReceiveStates state= INIT;
	YModemReturn returnValue= OK;

	uint8_t		 retriesRemaining;
	uint8_t		 controlByte;
	uint16_t 	 buffOffset;
	uint32_t	 blockCounter;
	PacketHeader* header;
	uint16_t* CRC;
	uint8_t Data[DATA_SIZE+sizeof(PacketHeader)+sizeof(uint16_t)];
	header= Data;

	while (true)
	{
		switch (state)
		{
			case INIT:
				// Initialize all state variables
				// Send C
				// Move to waiting for first header
				break;
			case WAIT_FIRST_HEADER:
				// sub 1 from retries -> if none left return with timeout
				// Receive header or wait TIMEOUT
				// if timeout return to INIT
				// else goto WAIT_DATA
				break;
			case WAIT_HEADER:
				// sub 1 from retries -> if none left return with timeout
				// Receive header or wait TIMEOUT
				// if timeout goto NACK
				// else goto WAIT_DATA
				break;
			case WAIT_DATA:
				// receive data (size depending on header) or wait TIMEOUT
				// if timeout goto NACK
				// else goto WAIT_CRC
				break;
			case WAIT_CRC:
				// receive CRC or wait TIMEOUT
				// if timeout goto NACK
				// else check block numers
				// 		check CRC
				//		if CRC match -> goto ACK else goto NACK
				break;
			case SEND_NACK:
				// SEND NACK
				// goto WAIT_HEADER
				break;
			case SEND_ACK:
				// SEND ACK
				// RESET retries
				// goto WAIT_HEADER
			
				break;
			default:
				return UNKNOWN_STATE;
				break;
		}
	}
	return returnValue;
}

YModemReturn YModem_Transmit(YModem* modem, FileRead readFunc, uint32_t size) { return OK; }
