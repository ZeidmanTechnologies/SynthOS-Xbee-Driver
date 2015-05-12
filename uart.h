/**
 * @addtogroup   Atmega328p
 * @{
 * @file
 * @uthor        Igor Serikov
 * @date         08-26-2014
 *
 * @brief        Atmega328p UART driver interface
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
 *
 * Notes
 * -------------------------------------------------------------------
 * We use ring buffers for both sending and receiving. We do not need
 * to disable interrupts while manipulating the buffers. Note that
 * we check the corresponding condition explicitly after SynthOS_wait
 * as there is no guarantee that they are still met when the scheduler
 * gives control back to us.
 */
#ifndef UART_BAUDRATE
#define UART_BAUDRATE             115200
#endif

void uart_transmit (void);

/**
 * @brief  Callback function that provides data to transmit
 *
 * This function is called from interrupt. 
 *
 * @return a byte of data or -1 to stop the transmission.
 */
int uart_transmit_byte (void);

/**
 * @brief  Callback function delivers received data
 *
 * This function is called from interrupt.
 *
 * @param  byte received byte of data
 */
void uart_receive_byte (unsigned char byte);
