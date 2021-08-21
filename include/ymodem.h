#ifndef __YMODEM_H__
#define __YMODEM_H__

#include <stdint.h>
#include <stdint.h>

typedef size_t (*InterfaceRead)(void* buff, size_t size);
typedef size_t (*InterfaceWrite)(void* buff, size_t size);
typedef size_t (*FileRead)(char* fileName, void* buff, size_t offset, size_t size);
typedef size_t (*FileWrite)(char* fileName, void* buff, size_t size);
typedef uint32_t (*Timestamp)();

typedef enum
{
	SUCC,
	FAIL,
	TIMEOUT,
	CANCLE
} YModemReturn;

typedef struct
{
	InterfaceRead  Read;  // Read data from interface for protocol
	InterfaceWrite Write; // Write data to interface for protocol
	Timestamp	   Time;
} YModem;

void		 YModem_Init(YModem* modem, InterfaceRead readFunc, InterfaceWrite writeFunc, Timestamp timeFunc);
YModemReturn YModem_Receive(YModem* modem, FileWrite writeFunc);
YModemReturn YModem_Transmit(YModem* modem, char* fileNames[], size_t sizes[], uint8_t numFiles, FileRead readFunc);

#endif
