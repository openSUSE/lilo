#ifndef _PPC_BOOT_SERIAL_H_
#define _PPC_BOOT_SERIAL_H_
/* $Id$ */

/*
 * A really private header file for the (dumb) serial driver in arch/ppc/boot
 *
 * Shamelessly taken from include/linux/serialP.h:
 *
 * Copyright (C) 1997 by Theodore Ts'o.
 *
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL)
 */

/*
 * This is our internal structure for each serial port's state.
 *
 * Many fields are paralleled by the structure used by the serial_struct
 * structure.
 *
 * Given that this is how SERIAL_PORT_DFNS are done, and that we need
 * to use a few of their fields, we need to have our own copy of it.
 */
struct serial_state {
	int magic;
	int baud_base;
	unsigned long port;
	int irq;
	int flags;
	int hub6;
	int type;
	int line;
	int revision;		/* Chip revision (950) */
	int xmit_fifo_size;
	int custom_divisor;
	int count;
	unsigned char *iomem_base;
	unsigned short iomem_reg_shift;
	unsigned short close_delay;
	unsigned short closing_wait;	/* time to wait before closing */
	unsigned long icount;
	int io_type;
	void *info;
	void *dev;
};

/* include/linux/serial_reg.h */
#define UART_FCR	2	/* Out: FIFO Control Register */
#define UART_FCR_ENABLE_FIFO	0x01	/* Enable the FIFO */
#define UART_FCR_CLEAR_RCVR	0x02	/* Clear the RCVR FIFO */
#define UART_FCR_CLEAR_XMIT	0x04	/* Clear the XMIT FIFO */
#define UART_FCR_DMA_SELECT	0x08	/* For DMA applications */
/*
 * Note: The FIFO trigger levels are chip specific:
 *	RX:76 = 00  01  10  11	TX:54 = 00  01  10  11
 * PC16550D:	 1   4   8  14		xx  xx  xx  xx
 * TI16C550A:	 1   4   8  14          xx  xx  xx  xx
 * TI16C550C:	 1   4   8  14          xx  xx  xx  xx
 * ST16C550:	 1   4   8  14		xx  xx  xx  xx
 * ST16C650:	 8  16  24  28		16   8  24  30	PORT_16650V2
 * NS16C552:	 1   4   8  14		xx  xx  xx  xx
 * ST16C654:	 8  16  56  60		 8  16  32  56	PORT_16654
 * TI16C750:	 1  16  32  56		xx  xx  xx  xx	PORT_16750
 * TI16C752:	 8  16  56  60		 8  16  32  56
 */
#define UART_FCR_R_TRIG_00	0x00
#define UART_FCR_R_TRIG_01	0x40
#define UART_FCR_R_TRIG_10	0x80
#define UART_FCR_R_TRIG_11	0xc0
#define UART_FCR_T_TRIG_00	0x00
#define UART_FCR_T_TRIG_01	0x10
#define UART_FCR_T_TRIG_10	0x20
#define UART_FCR_T_TRIG_11	0x30

#define UART_FCR_TRIGGER_MASK	0xC0	/* Mask for the FIFO trigger range */
#define UART_FCR_TRIGGER_1	0x00	/* Mask for trigger set at 1 */
#define UART_FCR_TRIGGER_4	0x40	/* Mask for trigger set at 4 */
#define UART_FCR_TRIGGER_8	0x80	/* Mask for trigger set at 8 */
#define UART_FCR_TRIGGER_14	0xC0	/* Mask for trigger set at 14 */
/* 16650 definitions */
#define UART_FCR6_R_TRIGGER_8	0x00	/* Mask for receive trigger set at 1 */
#define UART_FCR6_R_TRIGGER_16	0x40	/* Mask for receive trigger set at 4 */
#define UART_FCR6_R_TRIGGER_24  0x80	/* Mask for receive trigger set at 8 */
#define UART_FCR6_R_TRIGGER_28	0xC0	/* Mask for receive trigger set at 14 */
#define UART_FCR6_T_TRIGGER_16	0x00	/* Mask for transmit trigger set at 16 */
#define UART_FCR6_T_TRIGGER_8	0x10	/* Mask for transmit trigger set at 8 */
#define UART_FCR6_T_TRIGGER_24  0x20	/* Mask for transmit trigger set at 24 */
#define UART_FCR6_T_TRIGGER_30	0x30	/* Mask for transmit trigger set at 30 */
#define UART_FCR7_64BYTE	0x20	/* Go into 64 byte mode (TI16C750) */
#define UART_LCR	3	/* Out: Line Control Register */
/*
 * Note: if the word length is 5 bits (UART_LCR_WLEN5), then setting 
 * UART_LCR_STOP will select 1.5 stop bits, not 2 stop bits.
 */
#define UART_LCR_DLAB		0x80	/* Divisor latch access bit */
#define UART_LCR_SBC		0x40	/* Set break control */
#define UART_LCR_SPAR		0x20	/* Stick parity (?) */
#define UART_LCR_EPAR		0x10	/* Even parity select */
#define UART_LCR_PARITY		0x08	/* Parity Enable */
#define UART_LCR_STOP		0x04	/* Stop bits: 0=1 bit, 1=2 bits */
#define UART_LCR_WLEN5		0x00	/* Wordlength: 5 bits */
#define UART_LCR_WLEN6		0x01	/* Wordlength: 6 bits */
#define UART_LCR_WLEN7		0x02	/* Wordlength: 7 bits */
#define UART_LCR_WLEN8		0x03	/* Wordlength: 8 bits */

#define UART_MCR	4	/* Out: Modem Control Register */
#define UART_MCR_CLKSEL		0x80	/* Divide clock by 4 (TI16C752, EFR[4]=1) */
#define UART_MCR_TCRTLR		0x40	/* Access TCR/TLR (TI16C752, EFR[4]=1) */
#define UART_MCR_XONANY		0x20	/* Enable Xon Any (TI16C752, EFR[4]=1) */
#define UART_MCR_AFE		0x20	/* Enable auto-RTS/CTS (TI16C550C/TI16C750) */
#define UART_MCR_LOOP		0x10	/* Enable loopback test mode */
#define UART_MCR_OUT2		0x08	/* Out2 complement */
#define UART_MCR_OUT1		0x04	/* Out1 complement */
#define UART_MCR_RTS		0x02	/* RTS complement */
#define UART_MCR_DTR		0x01	/* DTR complement */

#define UART_LSR	5	/* In:  Line Status Register */
#define UART_LSR_TEMT		0x40	/* Transmitter empty */
#define UART_LSR_THRE		0x20	/* Transmit-hold-register empty */
#define UART_LSR_BI		0x10	/* Break interrupt indicator */
#define UART_LSR_FE		0x08	/* Frame error indicator */
#define UART_LSR_PE		0x04	/* Parity error indicator */
#define UART_LSR_OE		0x02	/* Overrun error indicator */
#define UART_LSR_DR		0x01	/* Receiver data ready */

#define UART_MSR	6	/* In:  Modem Status Register */
#define UART_MSR_DCD		0x80	/* Data Carrier Detect */
#define UART_MSR_RI		0x40	/* Ring Indicator */
#define UART_MSR_DSR		0x20	/* Data Set Ready */
#define UART_MSR_CTS		0x10	/* Clear to Send */
#define UART_MSR_DDCD		0x08	/* Delta DCD */
#define UART_MSR_TERI		0x04	/* Trailing edge ring indicator */
#define UART_MSR_DDSR		0x02	/* Delta DSR */
#define UART_MSR_DCTS		0x01	/* Delta CTS */
#define UART_MSR_ANY_DELTA	0x0F	/* Any of the delta bits! */

#define UART_SCR	7	/* I/O: Scratch Register */

/*
 * DLAB=1
 */
#define UART_DLL	0	/* Out: Divisor Latch Low */
#define UART_DLM	1	/* Out: Divisor Latch High */

/* include/linux/serial.h */
#define SERIAL_IO_PORT	0
#define SERIAL_IO_MEM	2
#define ASYNC_SKIP_TEST	0x0040	/* Skip UART test during autoconfiguration */
#define ASYNC_BOOT_AUTOCONF	0x10000000	/* Autoconfigure port on bootup */

/* include/asm-ppc/pc_serial.h */
/*
 * This assumes you have a 1.8432 MHz clock for your UART.
 *
 * It'd be nice if someone built a serial card with a 24.576 MHz
 * clock, since the 16550A is capable of handling a top speed of 1.5
 * megabits/second; but this requires the faster clock.
 */
#define BASE_BAUD ( 1843200 / 16 )
#define RS_TABLE_SIZE  4

#define STD_COM_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST)
#define STD_COM4_FLAGS ASYNC_BOOT_AUTOCONF

#define STD_SERIAL_PORT_DEFNS			\
	/* UART CLK   PORT IRQ     FLAGS        */			\
	{ 0, BASE_BAUD, 0x3F8, 4, STD_COM_FLAGS },	/* ttyS0 */	\
	{ 0, BASE_BAUD, 0x2F8, 3, STD_COM_FLAGS },	/* ttyS1 */	\
	{ 0, BASE_BAUD, 0x3E8, 4, STD_COM_FLAGS },	/* ttyS2 */	\
	{ 0, BASE_BAUD, 0x2E8, 3, STD_COM4_FLAGS },	/* ttyS3 */

#define EXTRA_SERIAL_PORT_DEFNS
#define HUB6_SERIAL_PORT_DFNS

#define SERIAL_PORT_DFNS		\
	STD_SERIAL_PORT_DEFNS		\
	EXTRA_SERIAL_PORT_DEFNS		\
	HUB6_SERIAL_PORT_DFNS

extern int serial_tstc(unsigned long com_port);
extern unsigned char serial_getc(unsigned long com_port);
extern void serial_putc(unsigned long com_port, unsigned char c);

extern void outb(int port, unsigned char val);
extern unsigned char inb(int port);

#endif				/* _PPC_BOOT_SERIAL_H_ */
