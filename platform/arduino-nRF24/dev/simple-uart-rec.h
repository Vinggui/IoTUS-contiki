/*
 * simple-uart-rec.h
 *
 *  Created on: 05/07/2016
 *      Author: vinicius
 */

#ifndef PLATFORM_ARDUINO_NRF24_DEV_SIMPLE_UART_REC_H_
#define PLATFORM_ARDUINO_NRF24_DEV_SIMPLE_UART_REC_H_


/**
 * Event posted when a line of input has been received.
 *
 * This event is posted when an entire line of input has been received
 * from the serial port. A data pointer to the incoming line of input
 * is sent together with the event.
 */
//extern process_event_t simple_serial_line_event_message;

int serial_line_simple_input(unsigned char c);
//void init_simple_uart_rec(void);
uint8_t * uartGetBuffer(void);
int hasUartMsgAvailable(void);
void uartFlush(void);

#endif /* PLATFORM_ARDUINO_NRF24_DEV_SIMPLE_UART_REC_H_ */
