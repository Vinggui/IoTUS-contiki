/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * safe_printer.h
 *
 *  Created on: Nov 15, 2017
 *      Author: vinicius
 */

#ifndef IOTUS_ARCH_SERVICES_SAFE_PRINTER_SAFE_PRINTER_H_
#define IOTUS_ARCH_SERVICES_SAFE_PRINTER_SAFE_PRINTER_H_

#include "iotus-core.h"
#include "platform-conf.h"
#include "ringbuf.h"

#define IOTUS_DONT_PRINT          0
#define IOTUS_PRINT_IMMEDIATELY   1
#define IOTUS_PRINT_SAFE          2
#define IOTUS_PRINT_SIMPLE        3

#ifdef _WIN32
    #define PATH_SEP            ";"
    #define DIR_SEP             "\\"
#else
    #define PATH_SEP            ":"
    #define DIR_SEP             "/"
#endif

typedef enum LogType {
    SAFE_PRINT_LOG_TYPE_INFO,
    SAFE_PRINT_LOG_TYPE_WARNING,
    SAFE_PRINT_LOG_TYPE_ERROR,
    SAFE_PRINT_CLEAN
} safe_printer_log_type;

#define IOTUS_SAFE_PRINTER_BUFFER_SIZE  128
#define IOTUS_SUB_BUF_SIZE              100

int
safe_printer_immediately_put(struct ringbuf *r, uint8_t c);

void
safe_printer_copy_to_bufring(void *output, const char *input, uint8_t size);

void
vprintfLogSafe(safe_printer_log_type logType, int (* output)(struct ringbuf *r, uint8_t c), const char *sourceFileName, uint16_t lineNumber, const char *format, va_list variadicArguments);

void
printfLogSafe(safe_printer_log_type logType, int (* output)(struct ringbuf *r, uint8_t c), const char *sourceFileName, uint16_t lineNumber, const char *format, ...);

void
iotus_signal_handler_safe_printer(iotus_service_signal signal, void *data);

#endif /* IOTUS_ARCH_SERVICES_SAFE_PRINTER_SAFE_PRINTER_H_ */

/* The following stuff ends the \defgroup block at the beginning of
   the file: */

/** @} */
