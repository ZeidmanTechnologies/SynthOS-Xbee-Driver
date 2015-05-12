/**
 * @addtogroup    XBeeTest
 * @{
 * @file
 * @author        Igor Serikov
 * @date          08-26-2014
 *
 * @brief         Timer module interface
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
 * --------------------------------------------------------
 * + Register increment time step:  1 / 16000000 * 1024 = 0.000064000 (64us)
 * + Internal clock register span:  0-155
 * + Clock tick time:  0.000064000 * 156 = 0.009984000 (~10ms)
 * + Clock wrap around time:  0.009984000 * 65536 / 60 = 10.905190400 (~11 min)
 */
#define clock_divider 156
#define time_step 0.000064L

extern volatile unsigned clock;

unsigned pclock (void);
unsigned pdiff (unsigned start, unsigned end);
