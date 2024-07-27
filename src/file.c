#include "ymodem.h"

YModemFile YModem_CreateFile(char* name, size_t size, YModem_LocalRead read, YModem_LocalWrite write)
{
	YModemFile file = {.Name = name, .Size = size, .Read = read, .Write = write};
	return file;
}
