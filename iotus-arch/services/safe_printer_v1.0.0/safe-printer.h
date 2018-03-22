#include "safe_printer.h"

#ifndef THIS_LOG_FILE_NAME_DESCRITOR
  #error THIS_LOG_FILE_NAME_DESCRITOR not defined for PRINT_SAFE service.
#endif

#if DEBUG == IOTUS_PRINT_IMMEDIATELY
  #define SAFE_PRINT_BUF(str,size)           safe_printer_copy_to_bufring(safe_printer_immediately_put,str,size)
  #define SAFE_PRINT(str)                    printf(str)
  #define SAFE_PRINTF_CLEAN(str, ...)        printfLogSafe(SAFE_PRINT_CLEAN,safe_printer_immediately_put, THIS_LOG_FILE_NAME_DESCRITOR, __LINE__, str, ##__VA_ARGS__)
  #define SAFE_PRINTF_LOG_INFO(str, ...)     printfLogSafe(SAFE_PRINT_LOG_TYPE_INFO,safe_printer_immediately_put, THIS_LOG_FILE_NAME_DESCRITOR, __LINE__, str, ##__VA_ARGS__)
  #define SAFE_PRINTF_LOG_WARNING(str, ...)  printfLogSafe(SAFE_PRINT_LOG_TYPE_WARNING,safe_printer_immediately_put, THIS_LOG_FILE_NAME_DESCRITOR, __LINE__, str, ##__VA_ARGS__)
  #define SAFE_PRINTF_LOG_ERROR(str, ...)    printfLogSafe(SAFE_PRINT_LOG_TYPE_ERROR,safe_printer_immediately_put, THIS_LOG_FILE_NAME_DESCRITOR, __LINE__, str, ##__VA_ARGS__)
#elif DEBUG == IOTUS_PRINT_SAFE /* DEBUG */
  #define SAFE_PRINT_BUF(str,size)           safe_printer_copy_to_bufring(ringbuf_put,str,size)
  #define SAFE_PRINT(str)                    safe_printer_copy_to_bufring(ringbuf_put,str,strlen(str))
  #define SAFE_PRINTF_CLEAN(str, ...)        printfLogSafe(SAFE_PRINT_CLEAN,ringbuf_put, THIS_LOG_FILE_NAME_DESCRITOR, __LINE__, str, ##__VA_ARGS__)
  #define SAFE_PRINTF_LOG_INFO(str, ...)     printfLogSafe(SAFE_PRINT_LOG_TYPE_INFO,ringbuf_put, THIS_LOG_FILE_NAME_DESCRITOR, __LINE__, str, ##__VA_ARGS__)
  #define SAFE_PRINTF_LOG_WARNING(str, ...)  printfLogSafe(SAFE_PRINT_LOG_TYPE_WARNING,ringbuf_put, THIS_LOG_FILE_NAME_DESCRITOR, __LINE__, str, ##__VA_ARGS__)
  #define SAFE_PRINTF_LOG_ERROR(str, ...)    printfLogSafe(SAFE_PRINT_LOG_TYPE_ERROR,ringbuf_put, THIS_LOG_FILE_NAME_DESCRITOR, __LINE__, str, ##__VA_ARGS__)
#elif DEBUG == IOTUS_PRINT_SIMPLE /* DEBUG */
  #define SAFE_PRINT_BUF(str,size)           safe_printer_copy_to_bufring(ringbuf_put,str,size)
  #define SAFE_PRINT(str)                    safe_printer_copy_to_bufring(ringbuf_put,str,strlen(str))
  #define SAFE_PRINTF_CLEAN(str,...)         safe_printer_copy_to_bufring(ringbuf_put,str,strlen(str))
  #define SAFE_PRINTF_LOG_INFO(str, ...)     safe_printer_copy_to_bufring(ringbuf_put,str,strlen(str))
  #define SAFE_PRINTF_LOG_WARNING(str, ...)  
  #define SAFE_PRINTF_LOG_ERROR(str, ...)    
#else
  #define SAFE_PRINT_BUF(str,size)           
  #define SAFE_PRINT(str)                    
  #define SAFE_PRINTF_CLEAN(str, ...)        
  #define SAFE_PRINTF_LOG_INFO(str, ...)     
  #define SAFE_PRINTF_LOG_WARNING(str, ...)  
  #define SAFE_PRINTF_LOG_ERROR(str, ...)    
#endif /* DEBUG */