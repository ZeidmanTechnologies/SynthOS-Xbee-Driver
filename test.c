/**
 * @addtogroup    XBeeTest
 * @{
 * @file
 * @author        Igor Serikov
 * @date          08-26-2014
 *
 * @brief         Test code
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
#include <util/delay.h>

#include "xbee.h"
#include "hardware.h"
#include "timer.h"

static unsigned char buf [32];
static xbee_receive_type recv = { buf_ptr: buf, buf_size: sizeof buf };
static xbee_request_type req;

static void do_power_down (void) {
    buzzer_enable ();
    _delay_ms (100);
    buzzer_disable ();
    power_down ();
}

void test () {
    int r;

    req.req = xbee_request_at;
    req.args.at.cmd [0] = 'F';
    req.args.at.cmd [1] = 'R';
    req.args.at.data_size = 0;
    req.args.at.buf_size = 0;
    SynthOS_call (xbee_request (&req));
    if (req.args.at.status != 0)
        do_power_down ();
    for (;;) {
        SynthOS_wait (associated);
        led_enable ();
        for (;;) {
            r = SynthOS_call (xbee_receive (&recv));
            if (!r)
                break;
            req.req = xbee_request_transmit;
            req.args.transmit.addr_hi = recv.addr_hi;
            req.args.transmit.addr_lo = recv.addr_lo;
            req.args.transmit.addr = recv.addr;
            req.args.transmit.data_ptr = buf;
            req.args.transmit.data_size = recv.recv_size;
            SynthOS_call (xbee_request (&req));
        }
        led_disable ();
    }
}
