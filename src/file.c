#include "ymodem.h"

YModemFile YModem_CreateFile(char* name, const size_t size, const YModem_DataRead read, const YModem_DataWrite write)
{
	YModemFile file;

	file.Name  = name;
	file.Size  = size;
	file.Read  = read;
	file.Write = write;

	return file;
}
