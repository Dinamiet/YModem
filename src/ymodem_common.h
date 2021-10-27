#include "crc16.h"
#include "ymodem.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define STX 0x02 // 1024 byte data block
#define SOH 0x01 // 128 byte data block
#define EOT 0x04 // End of transfer
#define ACK 0x06 // Acknownledged, continue transfer
#define NAK 0x15 // Not Acknownledged, retry transfer
#define CAN 0x18 // Cancel transmission
#define C	0x43 // Ready to receive data

#define MAX_RETRIES 10	 // number of attempts
#define TIMEOUT_MS	1000 // ms

#define DATA_SIZE		   1024
#define FILE_NAME		   32
#define LARGE_PACKET_BYTES 1024
#define SMALL_PACKET_BYTES 128

#define MIN(x, y) (x > y ? y : x)

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define BIG_ENDIAN_16(x)	__builtin_bswap16(x)
#define LITTLE_ENDIAN_16(x) (x)
#define BIG_ENDIAN_32(x)	__builtin_bswap32(x)
#define LITTLE_ENDIAN_32(x) (x)
#else
#define BIG_ENDIAN_16(x)	(x)
#define LITTLE_ENDIAN_16(x) __builtin_bswap16(x)
#define BIG_ENDIAN_32(x)	(x)
#define LITTLE_ENDIAN_32(x) __builtin_bswap32(x)
#endif
