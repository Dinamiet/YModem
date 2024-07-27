#include "ymodem.h"

void YModem_Init(YModem* modem, YModem_RemoteRead read, YModem_RemoteWrite write, YModem_Time time)
{
	modem->Read  = read;
	modem->Write = write;
	modem->Time  = time;
}
