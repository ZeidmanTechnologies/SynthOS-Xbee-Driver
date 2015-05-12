/**
 * @addtogroup    XBeeTest
 * @{
 * @file
 * @author        Igor Serikov
 * @date          07-29-2014
 *
 * @brief         Augmentation for util/delay.h
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

#undef __ATTR_CONST__
/**
 * @brief  Synthos does not support yet grammar like
 *         "__attribute__ ((xxx)) void func (xxx) {".
 */
#define __ATTR_CONST__

#include <util/delay.h>
