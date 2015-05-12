/**
 * @addtogroup    XBeeTest
 * @{
 * @file
 * @author        Igor Serikov
 * @date          08-26-2014
 *
 * @brief         Generic hardware control module
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

#include "hardware.h"

/**
 * @brief Hardware initialization routine
 */
static void hardware_init (void) __attribute__ ((constructor));
static void hardware_init (void) {
    /* Led pin is set output */
    DDRB |= _BV (DDB5);

    /* Buzzer pin is set to output */
    DDRB |= _BV (DDB3);
}

/** @brief Enables led (pin 13)  */
void led_enable (void) {
    PORTB |= _BV (PORTB5);
}

/** @brief Disables led (pin 13)  */
void led_disable (void) {
    PORTB &= ~_BV (PORTB5);
}

/** @brief Enables buzzer  */
void buzzer_enable (void) {
    PORTB |= _BV (PORTB3);
}

/** @brief Disables buzzer  */
void buzzer_disable (void) {
    PORTB &= ~_BV (PORTB3);
}

/** @brief Shuts system power down  */
void power_down (void) {
    SMCR = _BV (SM1) | _BV (SE);
    __asm__ __volatile__ ("sleep" ::: "memory");
}
