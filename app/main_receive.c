#include "ymodem.h"

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>


int fileWriteBuffer;
int fileReadBuffer;

uint32_t getCurrentTime()
{
	clock_t t		= clock();
	double	seconds = (double)t / (double)CLOCKS_PER_SEC;
	return seconds * 1000;
}

uint8_t interfaceRead(uint8_t* buff, uint32_t maxLen)
{
	return read(fileReadBuffer, buff, maxLen);
}

uint8_t interfaceWrite(uint8_t* buff, uint32_t len)
{
	uint8_t retval= write(fileWriteBuffer, buff, len);
	return retval;
}

uint8_t fileWrite(char* fileName, uint8_t* buff, uint32_t len)
{
	FILE* output = fopen(fileName, "wb");
	if (output == NULL)
		return 0;

	uint8_t retval= fwrite(buff, 1, len, output);
	fclose(output);
	return retval;
}

int main()
{
	fileWriteBuffer = open("/tmp/write", O_WRONLY | O_NONBLOCK);
	fileReadBuffer = open("/tmp/read", O_RDONLY | O_NONBLOCK);
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
