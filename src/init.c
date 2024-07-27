#include "ymodem.h"

void YModem_Init(YModem* modem, const YModem_DataTransfer read, const YModem_DataTransfer write, const YModem_Time time)
{
	modem->Read  = read;
	modem->Write = write;
	modem->Time  = time;
}
