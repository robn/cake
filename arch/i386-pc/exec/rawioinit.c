/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$

    Desc: Initialize raw IO
    Lang: english

    Note: serial io from "PC-intern" examples
*/

#include <proto/exec.h>
#include <asm/io.h>
#include <aros/debug.h>

#define SER_LCR_8BITS      0x03
#define SER_LCR_1STOPBIT   0x00
#define SER_LCR_NOPARITY   0x00

int ser_UARTType (int);
void ser_FIFOLevel(int, BYTE);
int ser_Init(int, LONG, BYTE);

/*****i***********************************************************************

    NAME */
	AROS_LH0(void, RawIOInit,

/*  LOCATION */
	struct ExecBase *, SysBase, 84, Exec)

/*  FUNCTION
	This is a private function. It initializes raw IO. After you
	have called this function, you can use RawMayGetChar() and
	RawPutChar().

    INPUTS
	None.

    RESULT
	None.

    NOTES
	This function is for very low level debugging only.

    EXAMPLE

    BUGS

    SEE ALSO
	RawPutChar(), RawMayGetChar()

    INTERNALS

    HISTORY

*****************************************************************************/
{
   AROS_LIBFUNC_INIT
   AROS_LIBBASE_EXT_DECL(struct ExecBase *,SysBase)
   if (ser_Init(0x2F8, 9600,SER_LCR_8BITS | SER_LCR_1STOPBIT | SER_LCR_NOPARITY))
	ser_FIFOLevel(0x2F8, 0);
   return;
   AROS_LIBFUNC_EXIT
} /* RawIOInit */

#define SER_TXBUFFER       0x00
#define SER_DIVISOR_LSB    0x00
#define SER_DIVISOR_MSB    0x01
#define SER_IRQ_ENABLE     0x01
#define SER_IRQ_ID         0x02
#define SER_FIFO           0x02
#define SER_2FUNCTION      0x02
#define SER_LINE_CONTROL   0x03
#define SER_MODEM_CONTROL  0x04
#define SER_MODEM_STATUS   0x06
#define SER_SCRATCH        0x07

#define SER_LCR_SETDIVISOR 0x80

#define SER_MCR_DTR        0x01
#define SER_MCR_RTS        0x02
#define SER_MCR_LOOP       0x10

#define SER_FIFO_ENABLE        0x01
#define SER_FIFO_RESETRECEIVE  0x02
#define SER_FIFO_RESETTRANSMIT 0x04

#define SER_MAXBAUD 115200L

int ser_UARTType (int port) {

	outb_p(0xAA, port+SER_LINE_CONTROL); /* set Divisor-Latch */
	if (inb_p(port+SER_LINE_CONTROL) != 0xAA)
		return 1;
	outb_p(0x55, port+SER_DIVISOR_MSB); /* write Divisor */
	if (inb_p(port+SER_DIVISOR_MSB) != 0x55)
		return 1;
	outb_p(0x55, port+SER_LINE_CONTROL); /* clear Divisor-Latch */
	if (inb_p(port+SER_LINE_CONTROL) != 0x55)
		return 1;
	outb_p(0x55, port+SER_IRQ_ENABLE);
	if (inb_p(port+SER_IRQ_ENABLE) != 0x05)
		return 1;
	outb_p(0, port+SER_FIFO); /* clear FIFO and IRQ */
	outb_p(0, port+SER_IRQ_ENABLE);
	if (inb_p(port+SER_IRQ_ID) != 1)
		return 1;
	outb_p(0xF5, port+SER_MODEM_CONTROL);
	if (inb_p(port+SER_MODEM_CONTROL) != 0x15)
		return 1;
	outb_p(SER_MCR_LOOP, port+SER_MODEM_CONTROL); /* Looping */
	inb_p(port+SER_MODEM_STATUS);
	if ((inb_p(port+SER_MODEM_STATUS) & 0xF0) != 0)
		return 1;
	outb_p(0x1F, port+SER_MODEM_CONTROL);
	if ((inb_p(port+SER_MODEM_STATUS) & 0xF0) != 0xF0)
		return 1;
	outb_p(SER_MCR_DTR | SER_MCR_RTS, port + SER_MODEM_CONTROL);

	outb_p(0x55, port+SER_SCRATCH); /* Scratch-Register ?*/
	if (inb_p(port+SER_SCRATCH) != 0x55)
		return 2;	//INS8250;
	outb_p(0, port+SER_SCRATCH);

	outb_p(0xCF, port+SER_FIFO);              /* FIFO ? */
	if ((inb_p(port+SER_IRQ_ID) & 0xC0) != 0xC0)
		return 3;	//NS16450;
	outb_p(0, port+SER_FIFO);
                            /* Alternate-Function Register ? */
	outb_p(SER_LCR_SETDIVISOR, port+SER_LINE_CONTROL);
	outb_p(0x07, port+SER_2FUNCTION);
	if (inb_p(port+SER_2FUNCTION ) != 0x07)
	{
		outb_p(0, port+SER_LINE_CONTROL);
		return 4;	//NS16550A;
	}
	outb_p(0, port+SER_LINE_CONTROL);               /* reset registers */
	outb_p(0, port+SER_2FUNCTION);
	return 5;	//NS16C552;
}

void ser_FIFOLevel(int port, BYTE level) {

	if (level)
		outb_p(level | SER_FIFO_ENABLE, port+SER_FIFO);
	else
		outb_p(SER_FIFO_RESETRECEIVE | SER_FIFO_RESETTRANSMIT, port+SER_FIFO);
}

int ser_Init(int port, LONG baudRate, BYTE params) {
WORD uDivisor;

	if (ser_UARTType(port)!=1)
	{
		uDivisor=SER_MAXBAUD / baudRate;
		outb_p(inb_p(SER_LINE_CONTROL) | SER_LCR_SETDIVISOR, port+SER_LINE_CONTROL);
		outb_p(0xFF & uDivisor, port+SER_DIVISOR_LSB);
		outb_p(uDivisor>>8, port+SER_DIVISOR_MSB);
		outb_p(inb_p(SER_LINE_CONTROL) & ~SER_LCR_SETDIVISOR, port+SER_LINE_CONTROL);

		outb_p(params, port+SER_LINE_CONTROL);
		inb_p(port+SER_TXBUFFER);
		return 1;
	}
	return 0;
}

