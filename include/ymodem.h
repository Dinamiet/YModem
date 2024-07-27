#ifndef _YMODEM_H_
#define _YMODEM_H_

#include <stddef.h>
#include <stdint.h>

typedef size_t (*YModem_RemoteRead)(void* buff, size_t size);
typedef size_t (*YModem_RemoteWrite)(void* buff, size_t size);
typedef size_t (*YModem_LocalRead)(void* buff, size_t size);
typedef size_t (*YModem_LocalWrite)(void* buff, size_t size);
typedef uint32_t (*YModem_Time)();

typedef enum _YModemReturn_
{
	YMODEM_SUCCESS,
	YMODEM_FAIL,
	YMODEM_TIMEOUT,
	YMODEM_CANCEL,
} YModemReturn;

typedef struct _YModemFile_
{
	char*             Name;
	size_t            Size;
	YModem_LocalRead  Read;
	YModem_LocalWrite Write;
} YModemFile;

typedef struct _YModem_
{
	YModem_RemoteRead  Read;  // Read data from interface for protocol
	YModem_RemoteWrite Write; // Write data to interface for protocol
	YModem_Time        Time;
} YModem;

void         YModem_Init(YModem* modem, YModem_RemoteRead read, YModem_RemoteWrite write, YModem_Time time);
YModemFile   YModem_CreateFile(char* name, size_t size, YModem_LocalRead read, YModem_LocalWrite write);
YModemReturn YModem_Receive(YModem* modem, YModemFile* files);
YModemReturn YModem_Transmit(YModem* modem, YModemFile* files);

#endif
