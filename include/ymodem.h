#ifndef __YMODEM_H__
#define __YMODEM_H__

#include <stdint.h>

typedef uint32_t (*InterfaceRead)(uint8_t* buff, uint32_t maxLen);
typedef uint32_t (*InterfaceWrite)(uint8_t* buff, uint32_t len);
typedef uint32_t (*FileRead)(char* fileName, uint8_t* buff, uint32_t maxLen);
typedef uint32_t (*FileWrite)(char* fileName, uint8_t* buff, uint32_t len);
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
YModemReturn YModem_Transmit(YModem* modem, FileRead readFunc, uint32_t size);

#endif
