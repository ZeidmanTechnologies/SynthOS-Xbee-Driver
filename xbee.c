/**
 * @addtogroup    XBee
 * @{
 * @file
 * @author        Igor Serikov
 * @date          08-26-2014
 *
 * @brief         XBee module
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
 *   XBee setup: AP=2
 */

#include <stddef.h>
#include <string.h>

#include "uart.h"
#include "synthos-support.h"
#include "xbee.h"

#define byte3(v) ((unsigned char) ((v) >> 24))
#define byte2(v) ((unsigned char) ((v) >> 16))
#define byte1(v) ((unsigned char) ((v) >>  8))
#define byte0(v) ((unsigned char)  (v)       )
#define make_ulong(v3, v2, v1, v0) (                    \
      ((uint32_t) (unsigned char) (v3) << 24) |    \
      ((uint32_t) (unsigned char) (v2) << 16) |    \
      ((uint32_t) (unsigned char) (v1) <<  8) |    \
      ((uint32_t) (unsigned char) (v0))            \
    )
#define make_ushort(v1, v0) (                           \
      ((uint16_t) (unsigned char) (v1) <<  8) |   \
      ((uint16_t) (unsigned char) (v0))           \
    )

/* XBee frame structures with frame type removed */
typedef union {
    struct {
        unsigned char id;
        char cmd [2];
    }  __attribute__ ((packed)) at_request;
    struct {
        unsigned char id;
        unsigned char addr64 [8];
        unsigned char addr16 [2];
        unsigned char radius;
        unsigned char options;
    }  __attribute__ ((packed)) transmit;
    struct {
        unsigned char status;
    }  __attribute__ ((packed)) status;
    struct {
        unsigned char id;
        char cmd [2];
        unsigned char status;
    }  __attribute__ ((packed)) at_response;
    struct {
        unsigned char id;
        unsigned char addr16 [2];
        unsigned char retries;
        unsigned char delivery;
        unsigned char discovery;
    }  __attribute__ ((packed)) transmit_status;
    struct {
        unsigned char addr64 [8];
        unsigned char addr16 [2];
        unsigned char options;
    }  __attribute__ ((packed)) receive;
} xbee_packet_type;

typedef struct {
    unsigned char type;
    xbee_packet_type header;
} __attribute__ ((packed)) transmitting_packet_type;

typedef enum {
    transmitting_state_idle,
    transmitting_state_frame_mark,
    transmitting_state_length_1,
    transmitting_state_length_2,
    transmitting_state_header,
    transmitting_state_data
} transmitting_state_type;

typedef enum {
    receiving_state_frame_mark,
    receiving_state_length_1,
    receiving_state_length_2,
    receiving_state_frame_type,
    receiving_state_modem_status,
    receiving_state_transmit_status,
    receiving_state_at_response,
    receiving_state_data,
    receiving_state_data_requested
} receiving_state_type;

typedef enum {
    expected_nothing,
    expected_transmit_status,
    expected_at_response,
} expected_type;

volatile int associated;
volatile expected_type expected_response;
volatile transmitting_state_type transmitting_state;
volatile int expected_data;

static unsigned char transmitting_sequence;
static transmitting_packet_type transmitting_packet;
static uint16_t receiving_packet_size;
static uint16_t transmitting_length_header, transmitting_length_data;
static uint16_t receiving_length_header, receiving_length_data;

static volatile unsigned char * transmitting_ptr_data, * receiving_ptr_data;
static volatile xbee_request_type * request;
static volatile xbee_receive_type * receive;
static volatile xbee_packet_type receiving_packet;
static volatile receiving_state_type receiving_state;
static volatile uint16_t receiving_length_read;
static volatile uint16_t transmitting_length_written;
static volatile int transmitting_esc, receiving_esc, receiving_bytes, receive_ok;
static volatile unsigned char transmitting_escaped, transmitting_chk, receiving_chk;
/* Temporary buffer for receving data */
static volatile unsigned char receiving_buffer [XBEE_RECEIVING_BUFFER_SIZE];

void xbee_init (void) __attribute__ ((constructor));
void xbee_init (void) {
    transmitting_sequence = 0xff;
    associated = 0;
    expected_response = expected_nothing;
    expected_data = 0;
    transmitting_state = transmitting_state_idle;
    transmitting_esc = 0;
    receiving_esc = 0;
    receiving_bytes = 0;
    receiving_state = receiving_state_frame_mark;
}

/*
 * Transmitter' state machine:
 *   transmitting_state: frame_mark -> length_1 -> length_2 [ -> header -> data ] -> idle
 *   Setup:
 *     transmitting_length_header (data goes to transmitting_packet);
 *     transmitting_ptr_data, transmitting_length_data.
 */
int uart_transmit_byte (void) {
    unsigned char byte;
    uint16_t len;

    if (transmitting_esc) {
        transmitting_esc = 0;
        return transmitting_escaped ^ 0x20;
    }

    switch (transmitting_state) {
      case transmitting_state_frame_mark:
        transmitting_state = transmitting_state_length_1;
        return 0x7E;
      case transmitting_state_length_1:
        len = transmitting_length_header + transmitting_length_data;
        byte = byte1 (len);
        transmitting_state = transmitting_state_length_2;
        break;
      case transmitting_state_length_2:
        len = transmitting_length_header + transmitting_length_data;
        byte = byte0 (len);
        transmitting_chk = 0;
        transmitting_length_written = 0;
        transmitting_state = transmitting_state_header;
        break;;
      case transmitting_state_header:
        if (transmitting_length_written < transmitting_length_header) {
            byte = ((unsigned char *) &transmitting_packet) [transmitting_length_written];
            transmitting_chk += byte;
            transmitting_length_written ++;
            break;
        }
        transmitting_length_written = 0;
        transmitting_state = transmitting_state_data;
        /* Fall through */
      case transmitting_state_data:
        if (transmitting_length_written < transmitting_length_data) {
            byte = transmitting_ptr_data  [transmitting_length_written];
            transmitting_chk += byte;
            transmitting_length_written ++;
            break;
        }
        byte = 0xff - transmitting_chk;
        transmitting_state = transmitting_state_idle;
        break;
      default:
        return -1;
    }
    if (byte == 0x7E || byte == 0x7D || byte == 0x13 || byte == 0x11) {
        transmitting_escaped = byte;
        transmitting_esc = 1;
        return 0x7D;
    }
    return byte;
}

static void new_sequence (void) {
    if (transmitting_sequence == 0xff)
        transmitting_sequence = 1;
    else
        transmitting_sequence ++;
}

/**
 * @brief  Execute XBee request (see @ref xbee_request_selector_type)
 * @param  req_ptr  structure contatining input and output data
 *                  (see @ref xbee_request_type)
 */
void xbee_request (struct xbee_request * req_ptr) {
    unsigned char b, chk, * bufp1, * bufp2;
    unsigned bufs1, bufs2, len;
    xbee_packet_type opack;

    switch (req_ptr->req) {
      case xbee_request_at:
        request = req_ptr;
        new_sequence ();
        transmitting_packet.type = 0x08;
        transmitting_packet.header.at_request.id = transmitting_sequence;
        transmitting_packet.header.at_request.cmd [0] = request->args.at.cmd [0];
        transmitting_packet.header.at_request.cmd [1] = request->args.at.cmd [1];
        transmitting_length_header = 
            offsetof (transmitting_packet_type, header) + sizeof transmitting_packet.header.at_request;
        transmitting_ptr_data = (unsigned char *) request->args.at.data_ptr;
        transmitting_length_data = request->args.at.data_size;
        expected_response = expected_at_response;
        break;
      case xbee_request_transmit:
        request = req_ptr;
        new_sequence ();
        transmitting_packet.type = 0x10;
        transmitting_packet.header.transmit.id = transmitting_sequence;
        transmitting_packet.header.transmit.addr64 [0] = byte3 (request->args.transmit.addr_hi);
        transmitting_packet.header.transmit.addr64 [1] = byte2 (request->args.transmit.addr_hi);
        transmitting_packet.header.transmit.addr64 [2] = byte1 (request->args.transmit.addr_hi);
        transmitting_packet.header.transmit.addr64 [3] = byte0 (request->args.transmit.addr_hi);
        transmitting_packet.header.transmit.addr64 [4] = byte3 (request->args.transmit.addr_lo);
        transmitting_packet.header.transmit.addr64 [5] = byte2 (request->args.transmit.addr_lo);
        transmitting_packet.header.transmit.addr64 [6] = byte1 (request->args.transmit.addr_lo);
        transmitting_packet.header.transmit.addr64 [7] = byte0 (request->args.transmit.addr_lo);
        transmitting_packet.header.transmit.addr16 [0] = byte1 (request->args.transmit.addr);
        transmitting_packet.header.transmit.addr16 [1] = byte0 (request->args.transmit.addr);
        transmitting_packet.header.transmit.radius = 0;
        transmitting_packet.header.transmit.options = 0;
        transmitting_length_header = 
            offsetof (transmitting_packet_type, header) + sizeof transmitting_packet.header.transmit;
        transmitting_ptr_data = request->args.transmit.data_ptr;
        transmitting_length_data = request->args.transmit.data_size;
        expected_response = expected_transmit_status;
        break;
      default:
        return;
    }

    transmitting_state = transmitting_state_frame_mark;
    uart_transmit ();

    SynthOS_wait (transmitting_state == transmitting_state_idle);

    SynthOS_wait (expected_response == expected_nothing);
}

/*
 * Receiver' state machine:
 *   receiving_state: xxx, got MARK -> length_1 -> length_2 -> frame_type ->
 *     modem_status | transmit_status | at_response | data_requested | state_data -> frame_mark
 *   receiving_bytes != 0 - meta state for processing the header, data and checksum.
 */
void uart_receive_byte (unsigned char byte) {
    /* Drop XON/XOFF */
    if (byte == 0x11 || byte == 0x13)
        return;

    if (byte == 0x7E) {
        receiving_state = receiving_state_length_1;
        receiving_esc = 0;
        receiving_bytes = 0;
        return;
    }

    if (byte == 0x7D) {
        receiving_esc = 1;
        return;
    }
	
    if (receiving_esc) {
        byte ^= 0x20;
        receiving_esc = 0;
    }
	
    if (receiving_bytes) {
        if (receiving_length_read < receiving_length_header) {
            receiving_chk += byte;
            ((unsigned char *) &receiving_packet) [receiving_length_read] = byte;
            receiving_length_read ++;
            return;
        }
        if (receiving_length_read < receiving_length_header + receiving_length_data) {
            receiving_chk += byte;
            receiving_ptr_data [receiving_length_read - receiving_length_header] = byte;
            receiving_length_read ++;
            return;
        }
        if (receiving_length_read < receiving_packet_size) {
            receiving_chk += byte;
            receiving_length_read ++;
            return;
        }
        if (byte != (unsigned char) 0xff - receiving_chk) {
            receiving_state = receiving_state_frame_mark;
            return;
        }
        receiving_bytes = 0;
    }
	
    switch (receiving_state) {
      case receiving_state_length_1:
        receiving_packet_size = (unsigned) byte << 8;
        receiving_state = receiving_state_length_2;
        return;
      case receiving_state_length_2:
        receiving_packet_size |= byte;
        if (receiving_packet_size == 0) {
            receiving_state = receiving_state_frame_mark; /* Wrong frame */
            return;
        }
        receiving_packet_size --; /* 1 byte is used for frame type */
        receiving_state = receiving_state_frame_type;
        return;
      case receiving_state_frame_type:
        switch (byte) {
          case 0x8A: /* Modem status packet */
            if (receiving_packet_size < sizeof receiving_packet.status) {
                receiving_state = receiving_state_frame_mark;
                return;
            }
            receiving_length_header = sizeof receiving_packet.status;
            receiving_length_data = 0;
            receiving_state = receiving_state_modem_status;
            break;
          case 0x8B: /* Transmit status packed */
            if (
              receiving_packet_size < sizeof receiving_packet.transmit_status ||
              expected_response != expected_transmit_status
            ) {
                receiving_state = receiving_state_frame_mark;
                return;
            }
            receiving_length_header = sizeof receiving_packet.transmit_status;
            receiving_length_data = 0;
            receiving_state = receiving_state_transmit_status;
            break;
          case 0x88: /* AT command response packet */
            if (
              receiving_packet_size < sizeof receiving_packet.at_response ||
              expected_response != expected_at_response
            ) {
                receiving_state = receiving_state_frame_mark;
                return;
            }
            receiving_length_header = sizeof receiving_packet.at_response;
            receiving_length_data = receiving_packet_size - sizeof receiving_packet.at_response;
            if (receiving_length_data > request->args.at.buf_size)
                receiving_length_data = request->args.at.buf_size;
            receiving_ptr_data = (unsigned char *) request->args.at.buf_ptr;
            request->args.at.recv_size = receiving_length_data;
            receiving_state = receiving_state_at_response;
            break;
          case 0x90: /* Receive packet */
            if (receiving_packet_size < sizeof receiving_packet.receive) {
                receiving_state = receiving_state_frame_mark;
                return;
            }
            receiving_length_header = sizeof receiving_packet.receive;
            receiving_length_data = receiving_packet_size - sizeof receiving_packet.receive;
            if (!expected_data) {
                if (receiving_length_data > sizeof receiving_buffer)
                    receiving_length_data = sizeof receiving_buffer;
                receiving_ptr_data = receiving_buffer;
                receiving_state = receiving_state_data;
            } else {
                receive->addr = make_ushort (
                  receiving_packet.receive.addr16 [0], receiving_packet.receive.addr16 [1]
                );
                if (receiving_length_data > receive->buf_size)
                    receiving_length_data = receive->buf_size;
                receiving_ptr_data = (unsigned char *) receive->buf_ptr;
                receiving_state = receiving_state_data_requested;
                receive->recv_size = receiving_length_data;
            }
            break;
          default:
            receiving_state = receiving_state_frame_mark;
            return;
        }
        receiving_length_read = 0;
        receiving_chk = byte;
        receiving_bytes = 1;
        return;
      case receiving_state_modem_status:
        switch (receiving_packet.status.status) {
          case 0x02:
            associated = 1;
            break;
          case 0x03:
            associated = 0;
            if (expected_data)
                expected_data = 0;
            break;
        }
        receiving_state = receiving_state_frame_mark;
        return;
      case receiving_state_transmit_status:
        request->args.transmit.status = receiving_packet.transmit_status.delivery;
        expected_response = expected_nothing;
        receiving_state = receiving_state_frame_mark;
        return;
      case receiving_state_at_response:
        if (receiving_packet.at_request.id == transmitting_sequence) {
            request->args.at.status = receiving_packet.at_response.status;
            expected_response = expected_nothing;
        }
        receiving_state = receiving_state_frame_mark;
        return;
      case receiving_state_data_requested:
        receive->addr_hi = make_ulong (
          receiving_packet.receive.addr64 [0], receiving_packet.receive.addr64 [1], 
          receiving_packet.receive.addr64 [2], receiving_packet.receive.addr64 [3]);
        receive->addr_lo = make_ulong (
          receiving_packet.receive.addr64 [4], receiving_packet.receive.addr64 [5], 
          receiving_packet.receive.addr64 [6], receiving_packet.receive.addr64 [7]
        );
        receive_ok = 1;
        expected_data = 0;
        /* Fall through */
      case receiving_state_data:
        receiving_state = receiving_state_frame_mark;
        return;
    }
}

/**
 * @brief  Performe XBee receive operation
 * @param  recv_ptr  structure contatining input and output data
 *                  ( see @ref xbee_receive_type)
 */
int xbee_receive (struct xbee_receive * recv_ptr) {
    int mask;
    uint16_t have;

    /* Avoiding the race condition: check - interrupt resets "associated" - infinite wait */
    mask = get_mask ();

    if (!associated) {
        set_mask (mask);
        return 0;
    }

    receive = recv_ptr;

    receive_ok = 0;

    have = 0;

    if (receiving_state == receiving_state_data) {
        /* We are already in process of receiving a data packet */
        if (receiving_length_read <= sizeof receiving_packet.receive) {
            /* We are receiving the header */
            receiving_length_data = receiving_packet_size - sizeof receiving_packet.receive;
            if (receiving_length_data > receive->buf_size)
                receiving_length_data = receive->buf_size;
            receiving_ptr_data = receive->buf_ptr;
            receive->recv_size = receiving_length_data;
            receiving_state = receiving_state_data_requested;
        } else if (receiving_length_read <= sizeof receiving_packet.receive + receiving_length_data) {
            /* We are receiving the data. No buffer overflow yet. */
            have = receiving_length_read - sizeof receiving_packet.receive;
            if (have >= receive->buf_size) {
                have = receive->buf_size;
                receive->recv_size = have;
                receiving_length_data = 0; /* No more data is needed */
            } else {
                receiving_length_data = receiving_packet_size - sizeof receiving_packet.receive;
                if (receiving_length_data > receive->buf_size)
                    receiving_length_data = receive->buf_size;
                receive->recv_size = receiving_length_data;
                receiving_ptr_data = (unsigned char *) receive->buf_ptr + have;
            }
            receiving_state = receiving_state_data_requested;
        } else {
            /* We are receiving the data. Buffer overflow already happened. */
            have = receiving_length_data;
            if (have > receive->buf_size)
                have = receive->buf_size;
            receive->recv_size = have;
            receiving_state = receiving_state_data_requested;
        }
    }

    expected_data = 1;

    set_mask (mask);

    if (have)
        /* Copy data after restoring interrupts mask */
        memcpy (receive->buf_ptr, receiving_buffer, have);

    SynthOS_wait (!expected_data);

    return receive_ok;
}
