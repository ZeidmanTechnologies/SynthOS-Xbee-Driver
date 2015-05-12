/**
 * @addtogroup    XBeeTest
 * @{
 * @file
 * @author        Igor Serikov
 * @date          07-29-2014
 *
 * @brief         Augmentation for avr/interrupt.h
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
#include <avr/interrupt.h>

#undef ISR
/**
 * @brief  Synthos does not support variadic macros yet
 */
#define ISR(vector)                                             \
    void vector (void) __attribute__ ((signal,__INTR_ATTRS));   \
    void vector (void)

#undef sei
/**
 * @brief Synthos does not suport correctly yet function-style macros
 *        without arguments
 */
static void inline sei (void) __attribute__ ((always_inline));
static void inline sei (void) {
    __asm__ __volatile__ ("sei" ::: "memory");
}

#undef cli
/**
 * @brief Synthos does not suport correctly yet function-style macros
 *        without arguments
 */
static void inline cli (void) __attribute__ ((always_inline));
static void inline cli (void) {
    __asm__ __volatile__ ("cli" ::: "memory");
}
