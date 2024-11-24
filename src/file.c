#include "ymodem.h"

YModemFile YModem_CreateFile(char* name, const size_t size, const YModem_DataReadInterface read_interface, const YModem_DataWriteInterface write_interface)
{
	YModemFile file;

	file.Name  = name;
	file.Size  = size;
	file.Read  = read_interface;
	file.Write = write_interface;

	return file;
}
