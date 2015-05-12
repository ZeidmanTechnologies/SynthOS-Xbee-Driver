/**
 * @addtogroup    XBeeTest
 * @{
 * @file
 * @author        Igor Serikov
 * @date          08-26-2014
 *
 * @brief         Timer module
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

#include "timer.h"

volatile unsigned clock;

static void timer_init (void) __attribute__ ((constructor));
static void timer_init (void) {
    clock = 0;

    TCCR2A = _BV (WGM21);
    TCCR2B = _BV (CS22) | _BV (CS21) | _BV (CS20);
    OCR2A = clock_divider;

    TIMSK2 |= _BV (OCIE2A);
}

/* Timer interrupt */
ISR (TIMER2_COMPA_vect) {
    clock ++;
}

/**
 * @brief  Reports 64us resolution clock.
 *
 * Wrap around time: 0.009984000 * 256 / 60 = 2.555904000 (~2.5 sec).
 * Use with pdiff.
 *
 * @return  2 byte value: high:low, low: 0-155
 */
unsigned pclock (void) {
    uint8_t sreg = SREG;
    unsigned r;

    cli ();

    r = TCNT2;

    /*
     * The sampled value of TCNT2 is reliable
     * only if there was no wrap around because
     * we do not know when we took the sample:
     * before or after the wrap around.
     * If the wrap around did happen, 
     * a good estimation would be clock+1:0
     * since the time passed after it is really
     * small.
     */

    if ((TIFR2 & _BV (OCF2A)) == 0)
        /* No wrap around */
        r |= clock << 8;
    else
        /* Wrap around */
        r = (clock + 1) << 8;

    SREG = sreg;
    return r;
}

/**
 * @brief Calculates time period
 * @param  start  start of period
 * @param  end    end of period
 * @return  number of time units (64us each)
 */
unsigned pdiff (unsigned start, unsigned end) {
    unsigned char h1 = (unsigned char) (start >> 8);
    unsigned char l1 = (unsigned char) start;
    unsigned char h2 = (unsigned char) (end >> 8);
    unsigned char l2 = (unsigned char) end;
    unsigned char dh = h2 - h1;
    unsigned char dl = l2 - l1;
    if (l2 < l1) {
        dl += clock_divider;
        dh --;
    }
    return (unsigned) dh * clock_divider + dl;
}
