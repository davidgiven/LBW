#ifndef LINUX_TERMIOS_H
#define LINUX_TERMIOS_H

#define	LINUX_TCSANOW	0
#define	LINUX_TCSADRAIN	1
#define	LINUX_TCSAFLUSH	2

extern int linux_tcgetattr(int fd, void* termios);
extern int linux_tcsetattr(int fd, int action, void* termios);

#endif
