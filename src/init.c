#include "ymodem.h"

void YModem_Init(YModem* modem, YModem_DataTransfer read, YModem_DataTransfer write, YModem_Time time)
{
	modem->Read  = read;
	modem->Write = write;
	modem->Time  = time;
}
