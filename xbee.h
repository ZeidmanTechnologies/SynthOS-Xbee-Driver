/**
 * @addtogroup    XBee
 * @{
 * @file
 * @author        Igor Serikov
 * @date          08-26-2014
 *
 * @brief         XBee module interface
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
#include <stdint.h>

#ifndef XBEE_RECEIVING_BUFFER_SIZE
#define XBEE_RECEIVING_BUFFER_SIZE 64
#endif

#define xbee_addr_unknown 0xFFFE

/**
 * @brief XBee request types
 *
 * @param  xbee_request_at  execute an AT command
 * @param  xbee_request_transmit  transmit a packet
 */
typedef enum {
    xbee_request_at,
    xbee_request_transmit
} xbee_request_selector_type;

/**
 * @brief  Structure containing input and output parameters for XBee requests
 * @param  [in] req  request type (see @ref xbee_request_selector_type)
 * @param  [in,out] args  request parameters
 * @param  [in,out] args.at  parameters for @c xbee_request_at
 * @param  [in] args.at.cmd  AT command
 * @param  [in] args.at.data_ptr  input data pointer 
 * @param  [in] args.at.data_size  input data size
 * @param  [in] args.at.buf_ptr  output buffer pointer 
 * @param  [in] args.at.buf_size  output buffer size
 * @param  [out] args.at.recv_size  received data size
 * @param  [out] args.at.status  reported status
 * @param  [in,out] args.transmit parameters for @c xbee_request_transmit
 * @param  [in] args.transmit.addr_hi  highest 32 bits of the 64 bit network address of the recepient (SH)
 * @param  [in] args.transmit.addr_lo  lowest 32 bits of the 64 bit network address of the recepient (SL)
 * @param  [in] args.transmit.addr  16 bit address of the recepient (use @a xbee_addr_unknown, if do not know)
 * @param  [in] args.transmit.data_ptr  input data pointer 
 * @param  [in] args.transmit.data_size  input data size
 * @param  [out] args.at.status  reported delivery status
 */
typedef struct xbee_request {
    xbee_request_selector_type req;
    union {
        struct {
            char cmd [2];
            void * data_ptr;
            uint16_t data_size;
            void * buf_ptr;
            uint16_t buf_size;
            uint16_t recv_size;
            unsigned char status;
        } at;
        struct {
            uint32_t addr_hi;
            uint32_t addr_lo;
            uint16_t addr;
            void * data_ptr;
            uint16_t data_size;
            unsigned char status;
        } transmit;
    } args;
} xbee_request_type;

/**
 * @brief  Structure containing input and output parameters for XBee receive operation
 * @param  [out] addr_hi  highest 32 bits of the 64 bit network address of the sender (SH)
 * @param  [out] addr_lo  lowest 32 bits of the 64 bit network address of the sender (SL)
 * @param  [out] addr  16 bit address of the sender upon return
 * @param  [in] buf_ptr  output buffer pointer 
 * @param  [in] buf_size  output buffer size
 * @param  [out] recv_size  received data size
 */
typedef struct xbee_receive {
    uint32_t addr_hi;
    uint32_t addr_lo;
    uint16_t addr;
    void * buf_ptr;
    uint16_t buf_size;
    uint16_t recv_size;
} xbee_receive_type;

/**
 * @brief Association indicator
 *
 * --------------------------------------------------------
 * Value               | Meaning
 * --------------------|-----------------------------------
 * 0 - not associated  | !0 - associated
 */
extern volatile int associated;
