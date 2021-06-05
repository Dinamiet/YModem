#include "ymodem.h"

#define STX 0x02 // 1024 byte data block
#define SOH 0x01 // 128 byte data block
#define EOT 0x04 // End of transfer
#define ACK 0x06 // Acknownledged, continue transfer
#define NAK 0x15 // Not Acknownledged, retry transfer
#define CAN 0x18 // Cancel transmission
#define C	0x43 // Ready to receive data

#define RETRIES 10 // number of attempts
#define TIMEOUT 1  // seconds

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

typedef enum
{

} YModemTransferStates;

void YModem_Init(YModem* modem, InterfaceRead readFunc, InterfaceWrite writeFunc, Sleep waitFunc)
{
	modem->Read	 = readFunc;
	modem->Write = writeFunc;
	modem->Wait	 = waitFunc;
}

YModemReturn YModem_Receive(YModem* modem, FileWrite writeFunc)
{
	uint8_t retriesRemaining = RETRIES;
	uint8_t controlByte;

	return OK;
}

YModemReturn YModem_Transmit(YModem* modem, FileRead readFunc, uint32_t size) { return OK; }
