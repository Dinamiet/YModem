#ifndef __YMODEM_H__
#define __YMODEM_H__

#include <stdint.h>

typedef uint8_t (*InterfaceRead)(uint8_t *buff, uint32_t maxLen);
typedef uint8_t (*InterfaceWrite)(uint8_t *buff, uint32_t len);
typedef uint8_t (*FileRead)(char* fileName, uint8_t *buff, uint32_t maxLen);
typedef uint8_t (*FileWrite)(char* fileName, uint8_t *buff, uint32_t len);
typedef void (*Sleep)(uint16_t timeout);

typedef enum {
	OK,
	TIMEOUT,
	CANCLED
} YModemReturn;

typedef struct
{
	InterfaceRead Read; // Read data from interface for protocol
	InterfaceWrite Write; // Write data to interface for protocol
	Sleep		   Wait;
} YModem;

void YModem_Init(YModem* modem, InterfaceRead readFunc, InterfaceWrite writeFunc, Sleep waitFunc);
YModemReturn YModem_Receive(YModem* modem, FileWrite writeFunc);
YModemReturn YModem_Transmit(YModem* modem, FileRead readFunc, uint32_t size);

#endif
