#include "ymodem.h"

void YModem_Init(YModem* modem, InterfaceRead readFunc, InterfaceWrite writeFunc, Timestamp timeFunc)
{
	modem->Read  = readFunc;
	modem->Write = writeFunc;
	modem->Time  = timeFunc;
}
