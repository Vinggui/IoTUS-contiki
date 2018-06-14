
/**
 * \defgroup decription...
 *
 * This...
 *
 * @{
 */

/*
 * safe_printer.c
 *
 *  Created on: Feb 27, 2018
 *      Author: vinicius
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global-parameters.h"
#include "platform-conf.h"
#include "safe_printer.h"
#include "ringbuf.h"
#include "timestamp.h"

#define DEBUG IOTUS_DONT_PRINT//IOTUS_PRINT_IMMEDIATELY
#define THIS_LOG_FILE_NAME_DESCRITOR "safe_printer"
#include "safe-printer.h"

/* Set the size of this buffer, by power of 2 */
static uint8_t gBuffer[IOTUS_SAFE_PRINTER_BUFFER_SIZE];
static struct ringbuf gOutput;

/*---------------------------------------------------------------------*/
int
safe_printer_immediately_put(struct ringbuf *r, uint8_t c)
{
  //ignore r...
  putchar(c);
  return 1;
}
/*---------------------------------------------------------------------*/
void
safe_printer_copy_to_bufring(void *output, const char *input, uint8_t size)
{
  int (* output_func)(struct ringbuf *r, uint8_t c) = output;
  uint8_t i;
  for(i=0;i<size;i++){
    output_func(&gOutput, input[i]);
  }
}
/*---------------------------------------------------------------------*/

/**
 * Print the requested message into a log format, with timestamp and other info
 * Obs: Source filename is truncated to 20 bytes most
 * \param all Arguments are used by macros. Basically it works like the printf function.
 */
void
vprintfLogSafe(safe_printer_log_type logType, int (* output)(struct ringbuf *r, uint8_t c), const char *sourceFileName, uint16_t lineNumber, const char *format, va_list variadicArguments)
{
  char subbuffer[IOTUS_SUB_BUF_SIZE];
  if(SAFE_PRINT_CLEAN != logType) {
    sprintf(subbuffer,"%u",timestamp_elapsed(&iotus_time_zero)/100);
    safe_printer_copy_to_bufring(output, subbuffer,strlen(subbuffer));
    output(&gOutput, '-');
    if(SAFE_PRINT_LOG_TYPE_INFO == logType) {
      output(&gOutput, 'I');
      output(&gOutput, 'N');
      output(&gOutput, 'F');
    } else if (SAFE_PRINT_LOG_TYPE_WARNING == logType) {
      output(&gOutput, 'W');
      output(&gOutput, 'R');
      output(&gOutput, 'N');
    } else if (SAFE_PRINT_LOG_TYPE_ERROR == logType) {
      output(&gOutput, 'E');
      output(&gOutput, 'R');
      output(&gOutput, 'R');
    }

    output(&gOutput, '[');

    /* Source filename is truncated to 20 bytes most */
    sprintf(subbuffer,"%s",sourceFileName);
    safe_printer_copy_to_bufring(output, subbuffer,strlen(subbuffer));
    output(&gOutput, ':');
    sprintf(subbuffer,"%d",lineNumber);
    safe_printer_copy_to_bufring(output, subbuffer,strlen(subbuffer));
    output(&gOutput, ']');
    output(&gOutput, ' ');
  }

  vsprintf(subbuffer, format, variadicArguments);
  safe_printer_copy_to_bufring(output, subbuffer,strlen(subbuffer));

  if(SAFE_PRINT_CLEAN == logType)
    return;
  output(&gOutput,'\n');
}

/*---------------------------------------------------------------------*/

/**
 * Print the requested message into a log format, with timestamp and other info
 * \param all Arguments are used by macros. Basically it works like the printf function.
 */
void
printfLogSafe(safe_printer_log_type logType, int (* output)(struct ringbuf *r, uint8_t c), const char *sourceFileName, uint16_t lineNumber, const char *format, ...)
{
  va_list variadicArguments;
  va_start(variadicArguments, format);
  vprintfLogSafe(logType, output, sourceFileName, lineNumber, format, variadicArguments);
  va_end(variadicArguments);
}
/*---------------------------------------------------------------------*/

/*
 * Default function required from IoTUS, to initialize, run and finish this service
 */
void
iotus_signal_handler_safe_printer(iotus_service_signal signal, void *data)
{
  if(IOTUS_START_SERVICE == signal) {
    SAFE_PRINT("\tService safe printer\n");
    ringbuf_init(&gOutput, gBuffer, IOTUS_SAFE_PRINTER_BUFFER_SIZE);
    timestamp_mark(&iotus_time_zero,0);
  } else if (IOTUS_RUN_SERVICE == signal){
    uint8_t i;
    for(i=0;i<ringbuf_elements(&gOutput);i++){
      putchar(ringbuf_get(&gOutput));
    }
  }/* else if (IOTUS_END_SERVICE == signal){

  }*/
}
/*---------------------------------------------------------------------*/