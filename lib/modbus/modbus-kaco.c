/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "modbus-private.h"

#include "modbus-kaco.h"
#include "modbus-kaco-private.h"

/* Define the slave ID of the remote device to talk in master mode or set the
 * internal slave ID in slave mode */
static int _modbus_set_slave(modbus_t *ctx, int slave)
{
    /* Broadcast address is 0 (MODBUS_BROADCAST_ADDRESS) */
    if (slave >= 0 && slave <= 247) {
        ctx->slave = slave;
    } else {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

/* Builds a KACO request header */
static int _modbus_kaco_build_request_basis(modbus_t *ctx, int function,
                                           int addr, int nb,
                                           uint8_t *req)
{
    assert(ctx->slave != -1);

    char msg[3] = {0};
    sprintf(msg, "%d", addr); 
    req[0] = 0x23; //'#'
    req[1] = msg[0];
    req[2] = msg[1];
    req[3] = 0x30; // '0'
    req[4] = 0xd;  // 'CR'

    return _MODBUS_KACO_PRESET_REQ_LENGTH;
}

uint8_t sum8_calculate(uint8_t *buf, unsigned int len)
{
    unsigned int sum = 0;
    for (unsigned int i = 0; i < len ; i++) {
        sum += buf[i];
    }
	return sum;
}

unsigned short crc16_x25(unsigned char *mem, unsigned int len)
{
    unsigned short crc = 0x0000;

    crc = ~crc;
    crc &= 0xffff;

    while (len--)
    {
        crc ^= *mem++;
        for (unsigned char k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ 0x8408 : crc >> 1;
    }

    crc ^= 0xFFFF;
    return crc;
}

static int _modbus_kaco_send_msg_pre(uint8_t *req, int req_length)
{
    return req_length;
}

static ssize_t _modbus_kaco_send(modbus_t *ctx, const uint8_t *req, int req_length)
{
    return write(ctx->s, req, req_length);
}

static ssize_t _modbus_kaco_recv(modbus_t *ctx, uint8_t *rsp, int rsp_length)
{
    return read(ctx->s, rsp, rsp_length);
}

static int _modbus_kaco_flush(modbus_t *);

/* The check_crc16 function shall return 0 if the message is ignored and the
   message length if the CRC is valid. Otherwise it shall return -1 and set
   errno to EMBBADCRC. */
static int _modbus_kaco_check_integrity(modbus_t *ctx, uint8_t *msg,
                                       const int msg_length)
{
    uint16_t crc_calculated;
    uint16_t crc_received;
    int slave = atoi(msg + 2);

    /* Filter on the Modbus unit identifier (slave) in RTU mode to avoid useless
     * CRC computing. */
    if (slave != ctx->slave && slave != MODBUS_BROADCAST_ADDRESS) {
        if (ctx->debug) {
            fprintf(stderr,
                    "The responding slave %d isn't the requested slave %d\n",
                    slave, ctx->slave);
        }
        errno = EMBBADSLAVE;
        return -1;
    }

    // If data corrupt, program will segmentation fault
    unsigned char preamble[2] = {0x0a, 0x2a};
    if (memcmp(msg, preamble, 2)) {
        crc_calculated = -1;
        crc_received = 0;
    }
    else if (msg_length == 66) {  
       /*   KACO Standard protocol : Series “00”/ “02”
        *   Example inverter reply
        *   ST1   A    S V      I     P    UN     IN     PN   T    E    F WR    ST2
        *   <LF>*030---4-355.9--2.92--1039-239.5--4.12---974--40---3229-«-5000xi<CR>
        */
        crc_calculated = sum8_calculate(msg+1, 56);
        crc_received = msg[57];

    }
    else {
        /*  Generic Protocol
        *   The checksum includes four ASCII digits (HEX) and no longer
        *   functional characters like “LF” and “CR”. The calculation
        *   of the CRC starts at “*” and ends at the last blank after the last
        *   measured value.
        */
        crc_calculated = crc16_x25(msg+1, msg_length-6);

        char str[5] = {0};
        memcpy(str, &msg[msg_length-5], 4);
        char *pEnd;
        crc_received = strtoul (str, &pEnd, 16);
    }

    /* Check CRC of msg */
    if (crc_calculated == crc_received) {
        return msg_length;
    } else {
        if (ctx->debug) {
            fprintf(stderr, "ERROR CRC received 0x%0X != CRC calculated 0x%0X\n",
                    crc_received, crc_calculated);
        }

        if (ctx->error_recovery & MODBUS_ERROR_RECOVERY_PROTOCOL) {
            _modbus_kaco_flush(ctx);
        }
        errno = EMBBADCRC;
        return -1;
    }
}

/* Sets up a serial port for KACO communications */
static int _modbus_kaco_connect(modbus_t *ctx)
{
    struct termios tios;
    speed_t speed;
    int flags;

    modbus_kaco_t *ctx_kaco = ctx->backend_data;

    if (ctx->debug) {
        printf("Opening %s at %d bauds (%c, %d, %d)\n",
               ctx_kaco->device, ctx_kaco->baud, ctx_kaco->parity,
               ctx_kaco->data_bit, ctx_kaco->stop_bit);
    }

    /* The O_NOCTTY flag tells UNIX that this program doesn't want
       to be the "controlling terminal" for that port. If you
       don't specify this then any input (such as keyboard abort
       signals and so forth) will affect your process

       Timeouts are ignored in canonical input mode or when the
       NDELAY option is set on the file via open or fcntl */
    flags = O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif

    ctx->s = open(ctx_kaco->device, flags);
    if (ctx->s == -1) {
        if (ctx->debug) {
            fprintf(stderr, "ERROR Can't open the device %s (%s)\n",
                    ctx_kaco->device, strerror(errno));
        }
        return -1;
    }

    /* Save */
    tcgetattr(ctx->s, &ctx_kaco->old_tios);

    memset(&tios, 0, sizeof(struct termios));

    /* C_ISPEED     Input baud (new interface)
       C_OSPEED     Output baud (new interface)
    */
    switch (ctx_kaco->baud) {
    case 110:
        speed = B110;
        break;
    case 300:
        speed = B300;
        break;
    case 600:
        speed = B600;
        break;
    case 1200:
        speed = B1200;
        break;
    case 2400:
        speed = B2400;
        break;
    case 4800:
        speed = B4800;
        break;
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
#ifdef B57600
    case 57600:
        speed = B57600;
        break;
#endif
#ifdef B115200
    case 115200:
        speed = B115200;
        break;
#endif
#ifdef B230400
    case 230400:
        speed = B230400;
        break;
#endif
#ifdef B460800
    case 460800:
        speed = B460800;
        break;
#endif
#ifdef B500000
    case 500000:
        speed = B500000;
        break;
#endif
#ifdef B576000
    case 576000:
        speed = B576000;
        break;
#endif
#ifdef B921600
    case 921600:
        speed = B921600;
        break;
#endif
#ifdef B1000000
    case 1000000:
        speed = B1000000;
        break;
#endif
#ifdef B1152000
   case 1152000:
        speed = B1152000;
        break;
#endif
#ifdef B1500000
    case 1500000:
        speed = B1500000;
        break;
#endif
#ifdef B2500000
    case 2500000:
        speed = B2500000;
        break;
#endif
#ifdef B3000000
    case 3000000:
        speed = B3000000;
        break;
#endif
#ifdef B3500000
    case 3500000:
        speed = B3500000;
        break;
#endif
#ifdef B4000000
    case 4000000:
        speed = B4000000;
        break;
#endif
    default:
        speed = B9600;
        if (ctx->debug) {
            fprintf(stderr,
                    "WARNING Unknown baud rate %d for %s (B9600 used)\n",
                    ctx_kaco->baud, ctx_kaco->device);
        }
    }

    /* Set the baud rate */
    if ((cfsetispeed(&tios, speed) < 0) ||
        (cfsetospeed(&tios, speed) < 0)) {
        close(ctx->s);
        ctx->s = -1;
        return -1;
    }

    /* C_CFLAG      Control options
       CLOCAL       Local line - do not change "owner" of port
       CREAD        Enable receiver
    */
    tios.c_cflag |= (CREAD | CLOCAL);
    /* CSIZE, HUPCL, CRTSCTS (hardware flow control) */

    /* Set data bits (5, 6, 7, 8 bits)
       CSIZE        Bit mask for data bits
    */
    tios.c_cflag &= ~CSIZE;
    switch (ctx_kaco->data_bit) {
    case 5:
        tios.c_cflag |= CS5;
        break;
    case 6:
        tios.c_cflag |= CS6;
        break;
    case 7:
        tios.c_cflag |= CS7;
        break;
    case 8:
    default:
        tios.c_cflag |= CS8;
        break;
    }

    /* Stop bit (1 or 2) */
    if (ctx_kaco->stop_bit == 1)
        tios.c_cflag &=~ CSTOPB;
    else /* 2 */
        tios.c_cflag |= CSTOPB;

    /* PARENB       Enable parity bit
       PARODD       Use odd parity instead of even */
    if (ctx_kaco->parity == 'N') {
        /* None */
        tios.c_cflag &=~ PARENB;
    } else if (ctx_kaco->parity == 'E') {
        /* Even */
        tios.c_cflag |= PARENB;
        tios.c_cflag &=~ PARODD;
    } else {
        /* Odd */
        tios.c_cflag |= PARENB;
        tios.c_cflag |= PARODD;
    }

    /* Read the man page of termios if you need more information. */

    /* This field isn't used on POSIX systems
       tios.c_line = 0;
    */

    /* C_LFLAG      Line options

       ISIG Enable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals
       ICANON       Enable canonical input (else raw)
       XCASE        Map uppercase \lowercase (obsolete)
       ECHO Enable echoing of input characters
       ECHOE        Echo erase character as BS-SP-BS
       ECHOK        Echo NL after kill character
       ECHONL       Echo NL
       NOFLSH       Disable flushing of input buffers after
       interrupt or quit characters
       IEXTEN       Enable extended functions
       ECHOCTL      Echo control characters as ^char and delete as ~?
       ECHOPRT      Echo erased character as character erased
       ECHOKE       BS-SP-BS entire line on line kill
       FLUSHO       Output being flushed
       PENDIN       Retype pending input at next read or input char
       TOSTOP       Send SIGTTOU for background output

       Canonical input is line-oriented. Input characters are put
       into a buffer which can be edited interactively by the user
       until a CR (carriage return) or LF (line feed) character is
       received.

       Raw input is unprocessed. Input characters are passed
       through exactly as they are received, when they are
       received. Generally you'll deselect the ICANON, ECHO,
       ECHOE, and ISIG options when using raw input
    */

    /* Raw input */
    tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /* C_IFLAG      Input options

       Constant     Description
       INPCK        Enable parity check
       IGNPAR       Ignore parity errors
       PARMRK       Mark parity errors
       ISTRIP       Strip parity bits
       IXON Enable software flow control (outgoing)
       IXOFF        Enable software flow control (incoming)
       IXANY        Allow any character to start flow again
       IGNBRK       Ignore break condition
       BRKINT       Send a SIGINT when a break condition is detected
       INLCR        Map NL to CR
       IGNCR        Ignore CR
       ICRNL        Map CR to NL
       IUCLC        Map uppercase to lowercase
       IMAXBEL      Echo BEL on input line too long
    */
    if (ctx_kaco->parity == 'N') {
        /* None */
        tios.c_iflag &= ~INPCK;
    } else {
        tios.c_iflag |= INPCK;
    }

    /* Software flow control is disabled */
    tios.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* C_OFLAG      Output options
       OPOST        Postprocess output (not set = raw output)
       ONLCR        Map NL to CR-NL

       ONCLR ant others needs OPOST to be enabled
    */

    /* Raw output */
    tios.c_oflag &=~ OPOST;

    /* C_CC         Control characters
       VMIN         Minimum number of characters to read
       VTIME        Time to wait for data (tenths of seconds)

       UNIX serial interface drivers provide the ability to
       specify character and packet timeouts. Two elements of the
       c_cc array are used for timeouts: VMIN and VTIME. Timeouts
       are ignored in canonical input mode or when the NDELAY
       option is set on the file via open or fcntl.

       VMIN specifies the minimum number of characters to read. If
       it is set to 0, then the VTIME value specifies the time to
       wait for every character read. Note that this does not mean
       that a read call for N bytes will wait for N characters to
       come in. Rather, the timeout will apply to the first
       character and the read call will return the number of
       characters immediately available (up to the number you
       request).

       If VMIN is non-zero, VTIME specifies the time to wait for
       the first character read. If a character is read within the
       time given, any read will block (wait) until all VMIN
       characters are read. That is, once the first character is
       read, the serial interface driver expects to receive an
       entire packet of characters (VMIN bytes total). If no
       character is read within the time allowed, then the call to
       read returns 0. This method allows you to tell the serial
       driver you need exactly N bytes and any read call will
       return 0 or N bytes. However, the timeout only applies to
       the first character read, so if for some reason the driver
       misses one character inside the N byte packet then the read
       call could block forever waiting for additional input
       characters.

       VTIME specifies the amount of time to wait for incoming
       characters in tenths of seconds. If VTIME is set to 0 (the
       default), reads will block (wait) indefinitely unless the
       NDELAY option is set on the port with open or fcntl.
    */
    /* Unused because we use open with the NDELAY option */
    tios.c_cc[VMIN] = 0;
    tios.c_cc[VTIME] = 0;

    if (tcsetattr(ctx->s, TCSANOW, &tios) < 0) {
        close(ctx->s);
        ctx->s = -1;
        return -1;
    }
    return 0;
}

static void _modbus_kaco_close(modbus_t *ctx)
{
    /* Restore line settings and close file descriptor in RTU mode */
    modbus_kaco_t *ctx_kaco = ctx->backend_data;

    if (ctx->s != -1) {
        tcsetattr(ctx->s, TCSANOW, &ctx_kaco->old_tios);
        close(ctx->s);
        ctx->s = -1;
    }
}

static int _modbus_kaco_flush(modbus_t *ctx)
{
    return tcflush(ctx->s, TCIOFLUSH);
}

static int _modbus_kaco_select(modbus_t *ctx, fd_set *rset,
                              struct timeval *tv, int length_to_read)
{
    int s_rc;
    while ((s_rc = select(ctx->s+1, rset, NULL, NULL, tv)) == -1) {
        if (errno == EINTR) {
            if (ctx->debug) {
                fprintf(stderr, "A non blocked signal was caught\n");
            }
            /* Necessary after an error */
            FD_ZERO(rset);
            FD_SET(ctx->s, rset);
        } else {
            return -1;
        }
    }

    if (s_rc == 0) {
        /* Timeout */
        errno = ETIMEDOUT;
        return -1;
    }
    return s_rc;
}

static void _modbus_kaco_free(modbus_t *ctx) {
    if (ctx->backend_data) {
        free(((modbus_kaco_t *)ctx->backend_data)->device);
        free(ctx->backend_data);
    }

    free(ctx);
}

const modbus_backend_t _modbus_kaco_backend = {
    _MODBUS_BACKEND_TYPE_KACO,
    _MODBUS_KACO_HEADER_LENGTH,
    _MODBUS_KACO_CHECKSUM_LENGTH,
    MODBUS_KACO_MAX_ADU_LENGTH,
    _modbus_set_slave,
    _modbus_kaco_build_request_basis,
    NULL,
    NULL,
    _modbus_kaco_send_msg_pre,
    _modbus_kaco_send,
    NULL,
    _modbus_kaco_recv,
    _modbus_kaco_check_integrity,
    NULL,
    _modbus_kaco_connect,
    _modbus_kaco_close,
    _modbus_kaco_flush,
    _modbus_kaco_select,
    _modbus_kaco_free
};

modbus_t* modbus_new_kaco(const char *device, int baud)
{
    modbus_t *ctx;
    modbus_kaco_t *ctx_kaco;

    /* Check device argument */
    if (device == NULL || *device == 0) {
        fprintf(stderr, "The device string is empty\n");
        errno = EINVAL;
        return NULL;
    }

    /* Check baud argument */
    if (baud == 0) {
        fprintf(stderr, "The baud rate value must not be zero\n");
        errno = EINVAL;
        return NULL;
    }

    ctx = (modbus_t *)malloc(sizeof(modbus_t));
    if (ctx == NULL) {
        return NULL;
    }

    _modbus_init_common(ctx);
    ctx->backend = &_modbus_kaco_backend;
    ctx->backend_data = (modbus_kaco_t *)malloc(sizeof(modbus_kaco_t));
    if (ctx->backend_data == NULL) {
        modbus_free(ctx);
        errno = ENOMEM;
        return NULL;
    }
    ctx_kaco = (modbus_kaco_t *)ctx->backend_data;

    /* Device name and \0 */
    ctx_kaco->device = (char *)malloc((strlen(device) + 1) * sizeof(char));
    if (ctx_kaco->device == NULL) {
        modbus_free(ctx);
        errno = ENOMEM;
        return NULL;
    }
    strcpy(ctx_kaco->device, device);

    char parity = 'N';
    int data_bit = 8, stop_bit = 1;
    ctx_kaco->baud = baud;
    if (parity == 'N' || parity == 'E' || parity == 'O') {
        ctx_kaco->parity = parity;
    } else {
        modbus_free(ctx);
        errno = EINVAL;
        return NULL;
    }
    ctx_kaco->data_bit = data_bit;
    ctx_kaco->stop_bit = stop_bit;

    return ctx;
}
