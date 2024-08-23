#include "ymodem.h"

#include <assert.h>

void YModem_Init(YModem* modem, const YModem_DataTransfer read, const YModem_DataTransfer write, const YModem_Time time)
{
	assert(modem != NULL);
	assert(read != NULL);
	assert(write != NULL);
	assert(time != NULL);

	modem->Read  = read;
	modem->Write = write;
	modem->Time  = time;
}
