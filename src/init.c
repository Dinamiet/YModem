#include "ymodem.h"

#include <assert.h>

void YModem_Init(
		YModem*                         modem,
		const YModem_DataReadInterface  read_interface,
		const YModem_DataWriteInterface write_interface,
		const YModem_TimeInterface      time_interface)
{
	assert(modem != NULL);
	assert(read_interface != NULL);
	assert(write_interface != NULL);
	assert(time_interface != NULL);

	modem->Read  = read_interface;
	modem->Write = write_interface;
	modem->Time  = time_interface;
}
