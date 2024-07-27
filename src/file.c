#include "ymodem.h"

YModemFile YModem_CreateFile(char* name, size_t size, YModem_DataTransfer read, YModem_DataTransfer write)
{
	YModemFile file = {.Name = name, .Size = size, .Read = read, .Write = write};
	return file;
}
