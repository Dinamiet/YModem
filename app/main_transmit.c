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

uint32_t interfaceRead(uint8_t* buff, uint32_t maxLen)
{
	usleep(1000);
	return read(fileReadBuffer, buff, maxLen);
}

uint32_t interfaceWrite(uint8_t* buff, uint32_t len)
{
	uint32_t retval = write(fileWriteBuffer, buff, len);
	return retval;
}

uint32_t fileRead(char* fileName, uint8_t* buff, uint32_t offset, uint32_t maxLen)
{
	FILE* output = fopen(fileName, "rb");
	if (output == NULL)
		return 0;

	fseek(output, offset, SEEK_SET);

	uint32_t retval = fread(buff, 1, maxLen, output);
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

	char* fileNames[]= {
		"YModem_receive",
		"YModem_transmit"
	};

	uint32_t sizes[]= { 0, 0 };

	FILE* file= fopen(fileNames[0], "rb");
	fseek(file, 0, SEEK_END);
	sizes[0]= ftell(file);
	fclose(file);
	file= fopen(fileNames[1], "rb");
	fseek(file, 0, SEEK_END);
	sizes[1]= ftell(file);
	fclose(file);

	YModem_Transmit(&modem, fileNames, sizes, 2, fileRead);

	close(fileWriteBuffer);
	close(fileReadBuffer);

	return 0;
}
