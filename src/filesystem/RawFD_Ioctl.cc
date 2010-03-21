/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "RawFD.h"
#include "termios.h"
#include <stdlib.h>
#include <sys/ioctl.h>

#define LINUX_TCGETS		0x5401
#define LINUX_TCSETS		0x5402
#define LINUX_TCSETSW		0x5403
#define LINUX_TCSETSF		0x5404
#define LINUX_TCGETA		0x5405
#define LINUX_TCSETA		0x5406
#define LINUX_TCSETAW		0x5407
#define LINUX_TCSETAF		0x5408
#define LINUX_TCSBRK		0x5409
#define LINUX_TCXONC		0x540A
#define LINUX_TCFLSH		0x540B
#define LINUX_TIOCEXCL	0x540C
#define LINUX_TIOCNXCL	0x540D
#define LINUX_TIOCSCTTY	0x540E
#define LINUX_TIOCGPGRP	0x540F
#define LINUX_TIOCSPGRP	0x5410
#define LINUX_TIOCOUTQ	0x5411
#define LINUX_TIOCSTI		0x5412
#define LINUX_TIOCGWINSZ	0x5413
#define LINUX_TIOCSWINSZ	0x5414
#define LINUX_TIOCMGET	0x5415
#define LINUX_TIOCMBIS	0x5416
#define LINUX_TIOCMBIC	0x5417
#define LINUX_TIOCMSET	0x5418
#define LINUX_TIOCGSOFTCAR	0x5419
#define LINUX_TIOCSSOFTCAR	0x541A
#define LINUX_FIONREAD	0x541B
#define LINUX_TIOCINQ		FIONREAD
#define LINUX_TIOCLINUX	0x541C
#define LINUX_TIOCCONS	0x541D
#define LINUX_TIOCGSERIAL	0x541E
#define LINUX_TIOCSSERIAL	0x541F
#define LINUX_TIOCPKT		0x5420
#define LINUX_FIONBIO		0x5421
#define LINUX_TIOCNOTTY	0x5422
#define LINUX_TIOCSETD	0x5423
#define LINUX_TIOCGETD	0x5424
#define LINUX_TCSBRKP		0x5425	/* Needed for POSIX tcsendbreak() */
#define LINUX_TIOCSBRK	0x5427  /* BSD compatibility */
#define LINUX_TIOCCBRK	0x5428  /* BSD compatibility */
#define LINUX_TIOCGSID	0x5429  /* Return the session ID of FD */
//#define LINUX_TCGETS2		_IOR('T', 0x2A, struct termios2)
//#define LINUX_TCSETS2		_IOW('T', 0x2B, struct termios2)
//#define LINUX_TCSETSW2	_IOW('T', 0x2C, struct termios2)
//#define LINUX_TCSETSF2	_IOW('T', 0x2D, struct termios2)
#define LINUX_TIOCGRS485	0x542E
#define LINUX_TIOCSRS485	0x542F
//#define LINUX_TIOCGPTN	_IOR('T', 0x30, unsigned int) /* Get Pty Number (of pty-mux device) */
//#define LINUX_TIOCSPTLCK	_IOW('T', 0x31, int)  /* Lock/unlock Pty */
#define LINUX_TCGETX		0x5432 /* SYS5 TCGETX compatibility */
#define LINUX_TCSETX		0x5433
#define LINUX_TCSETXF		0x5434
#define LINUX_TCSETXW		0x5435

#define LINUX_FIONCLEX	0x5450
#define LINUX_FIOCLEX		0x5451
#define LINUX_FIOASYNC	0x5452
#define LINUX_TIOCSERCONFIG	0x5453
#define LINUX_TIOCSERGWILD	0x5454
#define LINUX_TIOCSERSWILD	0x5455
#define LINUX_TIOCGLCKTRMIOS	0x5456
#define LINUX_TIOCSLCKTRMIOS	0x5457
#define LINUX_TIOCSERGSTRUCT	0x5458 /* For debugging only */
#define LINUX_TIOCSERGETLSR   0x5459 /* Get line status register */
#define LINUX_TIOCSERGETMULTI 0x545A /* Get multiport config  */
#define LINUX_TIOCSERSETMULTI 0x545B /* Set multiport config */

#define LINUX_TIOCMIWAIT	0x545C	/* wait for a change on serial input line(s) */
#define LINUX_TIOCGICOUNT	0x545D	/* read serial port inline interrupt counts */

#define LINUX_SIOCGSTAMP    0x8906

#define LINUX_TIOCSPTLCK    0x40045431
#define LINUX_TIOCGPTN      0x80045430

int RawFD::Ioctl(int cmd, u_int32_t argument)
{
	int result;
	switch (cmd)
	{
		case LINUX_TCGETS:
			return linux_tcgetattr(_realfd, (void*) argument);

		case LINUX_TCSETS:
			return linux_tcsetattr(_realfd, LINUX_TCSANOW, (void*) argument);

		case LINUX_TCSETSW:
			return linux_tcsetattr(_realfd, LINUX_TCSADRAIN, (void*) argument);

		case LINUX_TCSETSF:
			return linux_tcsetattr(_realfd, LINUX_TCSAFLUSH, (void*) argument);

		case LINUX_TIOCGWINSZ:
		{
			/* Linux & Interix struct winsize are compatible */
			struct winsize* ws = (struct winsize*) argument;
			result = ioctl(_realfd, TIOCGWINSZ, ws);
			goto common;
		}

		case LINUX_TIOCSWINSZ:
		{
			/* Linux & Interix struct winsize are compatible */
			struct winsize* ws = (struct winsize*) argument;
			result = ioctl(_realfd, TIOCSWINSZ, ws);
			goto common;
		}

		case LINUX_TIOCSCTTY:
		{
			result = ioctl(_realfd, TIOCSCTTY, 0);
			goto common;
		}

		case LINUX_TIOCGPGRP:
		{
			int& pgrp = *(int*) argument;
			result = tcgetpgrp(_realfd);
			if (result == -1)
				goto error;
			pgrp = result;
			return 0;
		}

		case LINUX_TIOCSPGRP:
		{
			int& pgrp = *(int*) argument;
			result = tcsetpgrp(_realfd, pgrp);
			goto common;
		}

		case LINUX_FIONREAD:
		{
			result = ioctl(_realfd, FIONREAD, argument);
			goto common;
		}

		case LINUX_SIOCGSTAMP:
			throw EINVAL;

		/* PTY related */

		case LINUX_TIOCGPTN:
		{
			char buffer[32];
			memset(buffer, 0, sizeof(buffer));
			if (ptsname_r(_realfd, buffer, sizeof(buffer)) != 0)
				throw EINVAL;

			/* Interix doesn't use pty numbers, instead returning a handle to
			 * a /dev/tty* device with an obscure name. Linux wants a number
			 * so it can open /dev/pts/X. So, we encode the part of the name
			 * that identifies the tty device into the number. See PtsVFSNode.
			 */

			u32& number = *(u32*) (buffer + 8);
			*(u32*)argument = number;

			return 0;
		}

		case LINUX_TIOCSPTLCK:
		{
			int& i = *(int*) argument;

			Warning("Unsupported call to TIOCSPTLCK(%d, %d)", _realfd, i);
			return 0;
		}

		default:
			return FD::Ioctl(cmd, argument);
	}

common:
	if (result == 0)
		return 0;
error:
	throw errno;
}
