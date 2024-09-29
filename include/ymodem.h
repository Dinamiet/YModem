#ifndef _YMODEM_H_
#define _YMODEM_H_

/**
 * \file
 * YModem transfer protocol
 *
 * Implements YModem transfer protocol
 */

#include <stddef.h>
#include <stdint.h>

/**
 * Function template to specify how to read data.
 * \param buff Data needs to be read into this buffer depending
 * \param size The maximum number of bytes to read
 * \return The number of bytes read
 */
typedef size_t (*YModem_DataRead)(void* buff, const size_t size);

/**
 * Function template to specify how to write data.
 * \param buff Data written from this buffer
 * \param size The number of bytes to write
 * \return The number of bytes written
 */
typedef size_t (*YModem_DataWrite)(const void* buff, const size_t size);

/**
 * YModem time function template
 * It is used to check for protocol timeouts
 * \return Current system time/counter to be used by YModem
 */
typedef uint32_t (*YModem_Time)();

/**
 * YModem operation return options
 */
typedef enum _YModemReturn_
{
	YMODEM_SUCCESS, /** Operation was successful */
	YMODEM_FAIL,    /** Operation failed */
	YMODEM_TIMEOUT, /** Operation timed out */
	YMODEM_CANCEL,  /** Operation was canceled */
} YModemReturn;

/**
 * File structure required by YModem, can be created using the CreateFile function
 */
typedef struct _YModemFile_
{
	const char*         Name;  /** File name */
	size_t              Size;  /** Size of the file */
	YModem_DataRead     Read;  /** Specification for reading data from the file */
	YModem_DataWrite    Write; /** Specification for writing data to the file */
} YModemFile;

/**
 * YModem
 */
typedef struct _YModem_
{
	YModem_DataRead     Read;  /** Specification for reading data from the YModem enabled interface */
	YModem_DataWrite    Write; /** Specification for writing data to the YModem enabled interface */
	YModem_Time         Time;  /** Specification for time keeping for YModem  */
} YModem;

/**
 * Initializes the YModem Interface
 * \param modem The modem structure to initialize
 * \param read Modem enabled interface read specification
 * \param write Modem enabled interface write specification
 * \param time Moden time keeping specification
 */
void YModem_Init(YModem* modem, const YModem_DataRead read, const YModem_DataWrite write, const YModem_Time time);

/**
 * Creates a new file that may be used by the protocol
 * \param name File name
 * \param size File size. When receiving files, it ensures adequate storage space before starting the transfer. When sending files, it is the file
 * data size to be sent.
 * \param read File data read specification. May be NULL when the file will only be used in receive operations.
 * \param write File data write specification. May be NULL when the file will only be used in transmit operations.
 * \return A File structure ready for use in YModem transfer protocol.
 */
YModemFile YModem_CreateFile(char* name, const size_t size, const YModem_DataRead read, const YModem_DataWrite write);

/**
 * Receive files over YModem protocol
 * \param modem Modem over which to receive files
 * \param files List of files to receive. List should be NULL terminated to indicate the end of the list.
 * \return Operation status
 */
YModemReturn YModem_Receive(const YModem* modem, const YModemFile* files);

/**
 * Transmits files over YModem protocol
 * \param modem Modem over which to transmit files
 * \param files List of files to transmit. List should be NULL terminated to indicate the end of the list.
 * \return Operation status
 */
YModemReturn YModem_Transmit(const YModem* modem, const YModemFile* files);

#endif
