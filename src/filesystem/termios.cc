#include "globals.h"
#include "termios.h"
#include <termios.h>

typedef unsigned char linux_cc_t;
typedef unsigned int linux_speed_t;
typedef unsigned int linux_tcflag_t;

#define LINUX_NCCS 19
struct linux_termios {
	linux_tcflag_t c_iflag;               /* input mode flags */
	linux_tcflag_t c_oflag;               /* output mode flags */
	linux_tcflag_t c_cflag;               /* control mode flags */
	linux_tcflag_t c_lflag;               /* local mode flags */
	linux_cc_t c_line;                    /* line discipline */
	linux_cc_t c_cc[LINUX_NCCS];          /* control characters */
};

#define LINUX_VINTR 0
#define LINUX_VQUIT 1
#define LINUX_VERASE 2
#define LINUX_VKILL 3
#define LINUX_VEOF 4
#define LINUX_VTIME 5
#define LINUX_VMIN 6
#define LINUX_VSWTC 7
#define LINUX_VSTART 8
#define LINUX_VSTOP 9
#define LINUX_VSUSP 10
#define LINUX_VEOL 11
#define LINUX_VREPRINT 12
#define LINUX_VDISCARD 13
#define LINUX_VWERASE 14
#define LINUX_VLNEXT 15
#define LINUX_VEOL2 16

/* c_iflag bits */
#define LINUX_IGNBRK	0000001
#define LINUX_BRKINT	0000002
#define LINUX_IGNPAR	0000004
#define LINUX_PARMRK	0000010
#define LINUX_INPCK	0000020
#define LINUX_ISTRIP	0000040
#define LINUX_INLCR	0000100
#define LINUX_IGNCR	0000200
#define LINUX_ICRNL	0000400
#define LINUX_IUCLC	0001000
#define LINUX_IXON	0002000
#define LINUX_IXANY	0004000
#define LINUX_IXOFF	0010000
#define LINUX_IMAXBEL	0020000
#define LINUX_IUTF8	0040000

/* c_oflag bits */
#define LINUX_OPOST	0000001
#define LINUX_OLCUC	0000002
#define LINUX_ONLCR	0000004
#define LINUX_OCRNL	0000010
#define LINUX_ONOCR	0000020
#define LINUX_ONLRET	0000040
#define LINUX_OFILL	0000100
#define LINUX_OFDEL	0000200
#define LINUX_NLDLY	0000400
#define   LINUX_NL0	0000000
#define   LINUX_NL1	0000400
#define LINUX_CRDLY	0003000
#define   LINUX_CR0	0000000
#define   LINUX_CR1	0001000
#define   LINUX_CR2	0002000
#define   LINUX_CR3	0003000
#define LINUX_TABDLY	0014000
#define   LINUX_TAB0	0000000
#define   LINUX_TAB1	0004000
#define   LINUX_TAB2	0010000
#define   LINUX_TAB3	0014000
#define   LINUX_XTABS	0014000
#define LINUX_BSDLY	0020000
#define   LINUX_BS0	0000000
#define   LINUX_BS1	0020000
#define LINUX_VTDLY	0040000
#define   LINUX_VT0	0000000
#define   LINUX_VT1	0040000
#define LINUX_FFDLY	0100000
#define   LINUX_FF0	0000000
#define   LINUX_FF1	0100000

/* c_cflag bit meaning */
#define LINUX_CBAUD	0010017
#define  LINUX_B0	0000000		/* hang up */
#define  LINUX_B50	0000001
#define  LINUX_B75	0000002
#define  LINUX_B110	0000003
#define  LINUX_B134	0000004
#define  LINUX_B150	0000005
#define  LINUX_B200	0000006
#define  LINUX_B300	0000007
#define  LINUX_B600	0000010
#define  LINUX_B1200	0000011
#define  LINUX_B1800	0000012
#define  LINUX_B2400	0000013
#define  LINUX_B4800	0000014
#define  LINUX_B9600	0000015
#define  LINUX_B19200	0000016
#define  LINUX_B38400	0000017
#define LINUX_EXTA B19200
#define LINUX_EXTB B38400
#define LINUX_CSIZE	0000060
#define   LINUX_CS5	0000000
#define   LINUX_CS6	0000020
#define   LINUX_CS7	0000040
#define   LINUX_CS8	0000060
#define LINUX_CSTOPB	0000100
#define LINUX_CREAD	0000200
#define LINUX_PARENB	0000400
#define LINUX_PARODD	0001000
#define LINUX_HUPCL	0002000
#define LINUX_CLOCAL	0004000
#define LINUX_CBAUDEX 0010000
#define    LINUX_BOTHER 0010000
#define    LINUX_B57600 0010001
#define   LINUX_B115200 0010002
#define   LINUX_B230400 0010003
#define   LINUX_B460800 0010004
#define   LINUX_B500000 0010005
#define   LINUX_B576000 0010006
#define   LINUX_B921600 0010007
#define  LINUX_B1000000 0010010
#define  LINUX_B1152000 0010011
#define  LINUX_B1500000 0010012
#define  LINUX_B2000000 0010013
#define  LINUX_B2500000 0010014
#define  LINUX_B3000000 0010015
#define  LINUX_B3500000 0010016
#define  LINUX_B4000000 0010017
#define LINUX_CIBAUD	  002003600000	/* input baud rate */
#define LINUX_CMSPAR	  010000000000	/* mark or space (stick) parity */
#define LINUX_CRTSCTS	  020000000000	/* flow control */

#define LINUX_IBSHIFT	  16		/* Shift from CBAUD to CIBAUD */

/* c_lflag bits */
#define LINUX_ISIG	0000001
#define LINUX_ICANON	0000002
#define LINUX_XCASE	0000004
#define LINUX_ECHO	0000010
#define LINUX_ECHOE	0000020
#define LINUX_ECHOK	0000040
#define LINUX_ECHONL	0000100
#define LINUX_NOFLSH	0000200
#define LINUX_TOSTOP	0000400
#define LINUX_ECHOCTL	0001000
#define LINUX_ECHOPRT	0002000
#define LINUX_ECHOKE	0004000
#define LINUX_FLUSHO	0010000
#define LINUX_PENDIN	0040000
#define LINUX_IEXTEN	0100000

/* tcflow() and TCXONC use these */
#define LINUX_TCOOFF		0
#define	LINUX_TCOON		1
#define	LINUX_TCIOFF		2
#define	LINUX_TCION		3

/* tcflush() and TCFLSH use these */
#define	LINUX_TCIFLUSH	0
#define	LINUX_TCOFLUSH	1
#define	LINUX_TCIOFLUSH	2

static u_int32_t convert_speed_i2l(u_int32_t speed)
{
	switch (speed)
	{
		case B0:      return LINUX_B0;
		case B50:     return LINUX_B50;
		case B75:     return LINUX_B75;
		case B110:    return LINUX_B110;
		case B134:    return LINUX_B134;
		case B150:    return LINUX_B150;
		case B200:    return LINUX_B200;
		case B300:    return LINUX_B300;
		case B600:    return LINUX_B600;
		case B1200:   return LINUX_B1200;
		case B1800:   return LINUX_B1800;
		case B2400:   return LINUX_B2400;
		case B4800:   return LINUX_B4800;
		case B9600:   return LINUX_B9600;
		case B19200:  return LINUX_B19200;
		case B38400:  return LINUX_B38400;
		case B57600:  return LINUX_B57600;
		case B115200: return LINUX_B115200;
		case B230400: return LINUX_B230400;
		case B460800: return LINUX_B460800;
	}

	return LINUX_B0;
}

static u_int32_t convert_speed_l2i(u_int32_t speed)
{
	switch (speed)
	{
		case LINUX_B0:      return B0;
		case LINUX_B50:     return B50;
		case LINUX_B75:     return B75;
		case LINUX_B110:    return B110;
		case LINUX_B134:    return B134;
		case LINUX_B150:    return B150;
		case LINUX_B200:    return B200;
		case LINUX_B300:    return B300;
		case LINUX_B600:    return B600;
		case LINUX_B1200:   return B1200;
		case LINUX_B1800:   return B1800;
		case LINUX_B2400:   return B2400;
		case LINUX_B4800:   return B4800;
		case LINUX_B9600:   return B9600;
		case LINUX_B19200:  return B19200;
		case LINUX_B38400:  return B38400;
		case LINUX_B57600:  return B57600;
		case LINUX_B115200: return B115200;
		case LINUX_B230400: return B230400;
		case LINUX_B460800: return B460800;
	}

	return B0;
}

static void convert_termios(struct termios& it, struct linux_termios& lt)
{
	memset(&lt, 0, sizeof(lt));

#define COPYBIT_I2L(field, name) \
	if (it.field & name) lt.field |= LINUX_ ## name

	COPYBIT_I2L(c_iflag, IGNBRK);
	COPYBIT_I2L(c_iflag, BRKINT);
	COPYBIT_I2L(c_iflag, IGNPAR);
	COPYBIT_I2L(c_iflag, PARMRK);
	COPYBIT_I2L(c_iflag, INPCK);
	COPYBIT_I2L(c_iflag, ISTRIP);
	COPYBIT_I2L(c_iflag, INLCR);
	COPYBIT_I2L(c_iflag, IGNCR);
	COPYBIT_I2L(c_iflag, ICRNL);
	COPYBIT_I2L(c_iflag, IXON);
	COPYBIT_I2L(c_iflag, IXANY);
	COPYBIT_I2L(c_iflag, IXOFF);
	COPYBIT_I2L(c_iflag, IMAXBEL);

	COPYBIT_I2L(c_oflag, OPOST);
	COPYBIT_I2L(c_oflag, ONLCR);
	COPYBIT_I2L(c_oflag, ONLRET);
	COPYBIT_I2L(c_oflag, OCRNL);
	COPYBIT_I2L(c_oflag, ONOCR);

	COPYBIT_I2L(c_cflag, CLOCAL);
	COPYBIT_I2L(c_cflag, CREAD);
	COPYBIT_I2L(c_cflag, CSTOPB);
	COPYBIT_I2L(c_cflag, HUPCL);
	COPYBIT_I2L(c_cflag, PARENB);
	COPYBIT_I2L(c_cflag, PARODD);

	switch (it.c_cflag & CSIZE)
	{
		case CS5: lt.c_cflag |= LINUX_CS5; break;
		case CS6: lt.c_cflag |= LINUX_CS6; break;
		case CS7: lt.c_cflag |= LINUX_CS7; break;
		case CS8: lt.c_cflag |= LINUX_CS8; break;
	}

	lt.c_cflag |= convert_speed_i2l(it.c_ispeed) << 16;
	lt.c_cflag |= convert_speed_i2l(it.c_ospeed);

	COPYBIT_I2L(c_lflag, ISIG);
	COPYBIT_I2L(c_lflag, ICANON);
	COPYBIT_I2L(c_lflag, ECHO);
	COPYBIT_I2L(c_lflag, ECHOE);
	COPYBIT_I2L(c_lflag, ECHOK);
	COPYBIT_I2L(c_lflag, ECHONL);
	COPYBIT_I2L(c_lflag, NOFLSH);
	COPYBIT_I2L(c_lflag, TOSTOP);
	COPYBIT_I2L(c_lflag, ECHOCTL);
	COPYBIT_I2L(c_lflag, IEXTEN);

#define COPYCC_I2L(name) \
	lt.c_cc[LINUX_##name] = it.c_cc[name]

	COPYCC_I2L(VEOF);
	COPYCC_I2L(VEOL);
	COPYCC_I2L(VERASE);
	COPYCC_I2L(VINTR);
	COPYCC_I2L(VKILL);
	COPYCC_I2L(VMIN);
	COPYCC_I2L(VQUIT);
	COPYCC_I2L(VSUSP);
	COPYCC_I2L(VTIME);
	COPYCC_I2L(VSTART);
	COPYCC_I2L(VSTOP);
	COPYCC_I2L(VREPRINT);
	COPYCC_I2L(VWERASE);
	COPYCC_I2L(VEOL2);
	COPYCC_I2L(VLNEXT);
}

static void convert_termios(struct linux_termios& lt, struct termios& it)
{
	memset(&it, 0, sizeof(it));

#define COPYBIT_L2I(field, name) \
	if (lt.field & LINUX_##name) it.field |= name

	COPYBIT_L2I(c_iflag, IGNBRK);
	COPYBIT_L2I(c_iflag, BRKINT);
	COPYBIT_L2I(c_iflag, IGNPAR);
	COPYBIT_L2I(c_iflag, PARMRK);
	COPYBIT_L2I(c_iflag, INPCK);
	COPYBIT_L2I(c_iflag, ISTRIP);
	COPYBIT_L2I(c_iflag, INLCR);
	COPYBIT_L2I(c_iflag, IGNCR);
	COPYBIT_L2I(c_iflag, ICRNL);
	COPYBIT_L2I(c_iflag, IXON);
	COPYBIT_L2I(c_iflag, IXANY);
	COPYBIT_L2I(c_iflag, IXOFF);
	COPYBIT_L2I(c_iflag, IMAXBEL);

	COPYBIT_L2I(c_oflag, OPOST);
	COPYBIT_L2I(c_oflag, ONLCR);
	COPYBIT_L2I(c_oflag, ONLRET);
	COPYBIT_L2I(c_oflag, OCRNL);
	COPYBIT_L2I(c_oflag, ONOCR);

	COPYBIT_L2I(c_cflag, CLOCAL);
	COPYBIT_L2I(c_cflag, CREAD);
	COPYBIT_L2I(c_cflag, CSTOPB);
	COPYBIT_L2I(c_cflag, HUPCL);
	COPYBIT_L2I(c_cflag, PARENB);
	COPYBIT_L2I(c_cflag, PARODD);

	switch (lt.c_cflag & LINUX_CSIZE)
	{
		case LINUX_CS5: it.c_cflag |= CS5; break;
		case LINUX_CS6: it.c_cflag |= CS6; break;
		case LINUX_CS7: it.c_cflag |= CS7; break;
		case LINUX_CS8: it.c_cflag |= CS8; break;
	}

	it.c_ispeed = convert_speed_l2i((lt.c_cflag >> 16) & LINUX_CBAUD);
	it.c_ospeed = convert_speed_l2i(lt.c_cflag & LINUX_CBAUD);

	COPYBIT_L2I(c_lflag, ISIG);
	COPYBIT_L2I(c_lflag, ICANON);
	COPYBIT_L2I(c_lflag, ECHO);
	COPYBIT_L2I(c_lflag, ECHOE);
	COPYBIT_L2I(c_lflag, ECHOK);
	COPYBIT_L2I(c_lflag, ECHONL);
	COPYBIT_L2I(c_lflag, NOFLSH);
	COPYBIT_L2I(c_lflag, TOSTOP);
	COPYBIT_L2I(c_lflag, ECHOCTL);
	COPYBIT_L2I(c_lflag, IEXTEN);

#define COPYCC_L2I(name) \
	it.c_cc[name] = lt.c_cc[LINUX_##name]

	COPYCC_L2I(VEOF);
	COPYCC_L2I(VEOL);
	COPYCC_L2I(VERASE);
	COPYCC_L2I(VINTR);
	COPYCC_L2I(VKILL);
	COPYCC_L2I(VMIN);
	COPYCC_L2I(VQUIT);
	COPYCC_L2I(VSUSP);
	COPYCC_L2I(VTIME);
	COPYCC_L2I(VSTART);
	COPYCC_L2I(VSTOP);
	COPYCC_L2I(VREPRINT);
	COPYCC_L2I(VWERASE);
	COPYCC_L2I(VEOL2);
	COPYCC_L2I(VLNEXT);
}

int linux_tcgetattr(int fd, void* termios)
{
	struct linux_termios& lt = *(struct linux_termios*) termios;
	struct termios it;

	int result = tcgetattr(fd, &it);
	if (result == -1)
		return -ErrnoI2L(errno);
	convert_termios(it, lt);
	return 0;
}

int linux_tcsetattr(int fd, int action, void* termios)
{
	struct linux_termios& lt = *(struct linux_termios*) termios;
	struct termios it;

	int iaction;
	switch (action)
	{
		case LINUX_TCSANOW:    iaction = TCSANOW; break;
		case LINUX_TCSADRAIN:  iaction = TCSADRAIN; break;
		case LINUX_TCSAFLUSH:  iaction = TCSAFLUSH; break;
		default:
			return -LINUX_EINVAL;
	}

	convert_termios(lt, it);
	int result = tcsetattr(fd, action, &it);
	return SysError(result);
}
