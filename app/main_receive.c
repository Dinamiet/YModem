#include "ymodem.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int fileWriteBuffer;
int fileReadBuffer;

uint32_t getCurrentTime()
{
	clock_t t		= clock();
	double	seconds = (double)t / (double)CLOCKS_PER_SEC;
	return seconds * 1000;
}

uint16_t interfaceRead(uint8_t* buff, uint16_t maxLen)
{
	usleep(1000);
	return read(fileReadBuffer, buff, maxLen);
}

uint16_t interfaceWrite(uint8_t* buff, uint16_t len)
{
	uint16_t retval = write(fileWriteBuffer, buff, len);
	return retval;
}

uint16_t fileWrite(char* fileName, uint8_t* buff, uint16_t len)
{
	FILE* output = fopen(fileName, "wb");
	if (output == NULL)
		return 0;

	uint16_t retval = fwrite(buff, 1, len, output);
	fclose(output);
	return retval;
}

int main()
{
	fileWriteBuffer = open("/tmp/write", O_WRONLY | O_NONBLOCK);
	fileReadBuffer	= open("/tmp/read", O_RDONLY | O_NONBLOCK);
	if (fileWriteBuffer == 0 || fileReadBuffer == 0)
	{
		return 1;
	}

	YModem modem;
	YModem_Init(&modem, interfaceRead, interfaceWrite, getCurrentTime);

	YModem_Receive(&modem, fileWrite);

	close(fileWriteBuffer);
	close(fileReadBuffer);

	return 0;
}
