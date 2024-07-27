#ifndef _YMODEM_H_
#define _YMODEM_H_

#include <stddef.h>
#include <stdint.h>

typedef size_t (*YModem_DataTransfer)(void* buff, size_t size);
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
	const char*         Name;
	size_t              Size;
	YModem_DataTransfer Read;
	YModem_DataTransfer Write;
} YModemFile;

typedef struct _YModem_
{
	YModem_DataTransfer Read;  // Read data from interface for protocol
	YModem_DataTransfer Write; // Write data to interface for protocol
	YModem_Time         Time;
} YModem;

void         YModem_Init(YModem* modem, const YModem_DataTransfer read, const YModem_DataTransfer write, const YModem_Time time);
YModemFile   YModem_CreateFile(char* name, const size_t size, const YModem_DataTransfer read, const YModem_DataTransfer write);
YModemReturn YModem_Receive(const YModem* modem, const YModemFile* files);
YModemReturn YModem_Transmit(const YModem* modem, const YModemFile* files);

#endif
