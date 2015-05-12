/**
 *
 * Project:       Arduino (DFRobot rover v2) robot
 * File:          synthos-support.c
 * Author:        Igor Serikov
 * Date:          07-29-2014
 *
 * Purpose:       Interrupts manipulation routines for SynthOS.
 *
 * Copyright (c) 2014 Zeidman Technologies, Inc.
 * 15565 Swiss Creek Lane, Cupertino California, 95014 
 * All Rights Reserved
 *
 * Zeidman Technologies gives an unlimited, nonexclusive license to
 * use this code  as long as this header comment section is kept
 * intact in all distributions and all future versions of this file
 * and the routines within it.
 *
 */
#include <avr/interrupt.h>

#include "synthos-support.h"

/*
 * As SynthOS user manual states, the following functions
 * are needed for a system that uses interrupt.
 */

/*
 * enableAll: a routine that enables all interrupts. This is called by
 * the system only once after all Init tasks have executed.
 */
void enable_ints (void) {
    sei ();
}

/*
 * getMask: a routine that disables all interrupts and returns the
 * previous state of the interrupt system just prior to this call.
 */
int get_mask (void) {
    int mask = SREG;
    cli ();
    return mask;
}

/*
 * setMask: a routine that restores the interrupt system to the state
 * specified in the parameter. The parameter value is normally the value
 * returned by getIntMask.
 */
void set_mask (int mask) {
    SREG = (uint8_t) mask;
}
