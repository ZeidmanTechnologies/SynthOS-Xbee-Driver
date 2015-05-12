/**
 * @addtogroup   Atmega328p
 * @{
 * @file
 * @uthor        Igor Serikov
 * @date         08-26-2014
 *
 * @brief        Atmega328p UART driver
 *
 * @copyright
 * Copyright (c) 2014 Zeidman Technologies, Inc.
 * 15565 Swiss Creek Lane, Cupertino California, 95014 
 * All Rights Reserved
 *
 * @copyright
 * Zeidman Technologies gives an unlimited, nonexclusive license to
 * use this code  as long as this header comment section is kept
 * intact in all distributions and all future versions of this file
 * and the routines within it.
 */
#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"

#define UART_PRESCALLER  (unsigned) (((F_CPU / (UART_BAUDRATE * 8UL))) - 1)

/**
 * @brief UART initialization routine
 */
static void uart_init (void) __attribute__ ((constructor));
static void uart_init (void) {
    UCSR0B = _BV (TXEN0) | _BV (RXEN0) | _BV (RXCIE0);
    UCSR0A |= _BV (U2X0);
    UCSR0C = _BV (UCSZ00) | _BV (UCSZ01);
    UBRR0H = (uint8_t) (UART_PRESCALLER >> 8);
    UBRR0L = (uint8_t) UART_PRESCALLER;
}

/** @brief Activates UART transmission by enable "register empty" interrupt */
void uart_transmit (void) {
    UCSR0B |= _BV (UDRIE0);
}

ISR (USART_UDRE_vect) {
    int x = uart_transmit_byte ();
    if (x != -1) {
        UDR0 = (unsigned char) x;
        return;
    }
    UCSR0B &= ~_BV (UDRIE0);
}

ISR (USART_RX_vect) {
    uart_receive_byte (UDR0);
}
