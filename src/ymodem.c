#include "ymodem.h"

#define STX 0x02 // 1024 byte data block
#define SOH 0x01 // 128 byte data block
#define EOT 0x04 // End of transfer
#define ACK 0x06
#define NAK 0x15
#define CAN 0x18 // Cancel transmission
#define C	0x43 // Ready

#define RETRIES		  10
#define SHORT_TIMEOUT 1	 // 1 seconds
#define LONG_TIMEOUT  10 // 10 seconds

#define BIG_PACKET_DATA_SIZE   1024
#define SMALL_PACKET_DATA_SIZE 128

typedef struct
{
	uint8_t	 Type;
	uint8_t	 BlockNumber;
	uint8_t	 BlockNumberComplement;
	uint8_t	 Data[BIG_PACKET_DATA_SIZE];
	uint16_t CRC;
} BigPacket;

typedef struct
{
	uint8_t	 Type;
	uint8_t	 BlockNumber;
	uint8_t	 BlockNumberComplement;
	uint8_t	 Data[SMALL_PACKET_DATA_SIZE];
	uint16_t CRC;
} SmallPacket;

void YModem_Init(YModem* modem, InterfaceRead readFunc, InterfaceWrite writeFunc);
{
	modem->Read	 = readFunc;
	modem->Write = writeFunc;
}

YModemReturn YModem_Receive(YModem* modem, FileWrite writeFunc) { return OK; }

YModemReturn YModem_Transmit(YModem* modem, FileRead readFunc) { return OK; }
