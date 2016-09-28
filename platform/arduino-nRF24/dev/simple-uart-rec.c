/*
 * simple-uart-rec.c
 *
 *  Created on: 4 de jul de 2016
 *      Author: vinicius
 */


#include <stdint.h>
#include <stdio.h>
#include "contiki.h"


#ifdef SERIAL_LINE_CONF_BUFSIZE
#define BUFSIZE SERIAL_LINE_CONF_BUFSIZE
#else /* SERIAL_LINE_CONF_BUFSIZE */
#define BUFSIZE 128
#endif /* SERIAL_LINE_CONF_BUFSIZE */

#if (BUFSIZE & (BUFSIZE - 1)) != 0
#error SERIAL_LINE_CONF_BUFSIZE must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change SERIAL_LINE_CONF_BUFSIZE in contiki-conf.h.
#endif

static uint8_t rxbuf_data[BUFSIZE];
static volatile uint8_t rx_writing_pointer = 0;
static volatile uint8_t rxCounterToPostProcess = 0;

//process_event_t simple_serial_line_event_message;

/*---------------------------------------------------------------------------*/
int
serial_line_simple_input(unsigned char c)
{
    rxbuf_data[rx_writing_pointer]=c;
    if(rx_writing_pointer>=BUFSIZE ||
       rxCounterToPostProcess > BUFSIZE) {
        return 1;
    }
    rx_writing_pointer++;

    if(rxCounterToPostProcess>0) {
        rxCounterToPostProcess--;
    } else if(rx_writing_pointer == 1) {
        rxCounterToPostProcess = c-1;
    }

    return 1;
}

uint8_t * uartGetBuffer(void) {
    return rxbuf_data;
}

int hasUartMsgAvailable(void) {
    int ret = (1 < rx_writing_pointer && 0 == rxCounterToPostProcess);
    rx_writing_pointer = 0;
    rxCounterToPostProcess = 0;
    return ret;
}

void uartFlush(void) {
    rx_writing_pointer = 0;
    rxCounterToPostProcess = 0;
}

/*void init_simple_uart_rec(void) {
    simple_serial_line_event_message = process_alloc_event();
}*/
