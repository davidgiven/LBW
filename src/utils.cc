/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include <sys/procfs.h>
#include <stdarg.h>

void vwritef(int fd, const char* format, va_list ap)
{
	char buffer[160];
	vsnprintf(buffer, sizeof(buffer), format, ap);
	write(fd, buffer, strlen(buffer));
}

void writef(int fd, const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	vwritef(fd, format, ap);
	va_end(ap);
}

string cprintf(const char* format, ...)
{
	char buffer[160];

	va_list ap;
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);

	return buffer;
}

void DumpMemory(const void* address, u_int32_t len)
{
	DumpMemory((u_int32_t) address, len);
}

void DumpMemory(u_int32_t address, u_int32_t len)
{
	string s;

	u_int32_t max = address + len;
	while (address < max)
	{
		s += cprintf("%08x : ", address);

		for (int i = 0; i < 16; i++)
		{
			u_int32_t o = address + i;
			if (o < max)
				s += cprintf("%02x ", *(u_int8_t*) o);
			else
				s += cprintf("** ");
		}

		s += cprintf(": ");

		for (int i = 0; i < 16; i++)
		{
			u_int32_t o = address + i;
			if (o < max)
			{
				u_int8_t c = *(u_int8_t*) o;
				if ((c >= 32) && (c <= 126))
					s += c;
				else
					s += '.';
			}
			else
				s += ' ';
		}
		s += '\n';
		address += 16;
	}

	write(2, s.data(), s.size());
}

void DumpMap(pid_t pid)
{
	char filename[32];
	snprintf(filename, sizeof(filename), "/proc/%ld/map", pid);

	int fd = open(filename, O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "Could not access process %ld\n", pid);
		return;
	}

	int n = 0;
	for (;;)
	{
		struct prmap map;
		int i = read(fd, &map, sizeof(map));
		if (i != sizeof(map))
			break;

		fprintf(stderr, "% 3d  %08x+%08lx %c%c%c%c%c%c%c %s\n", n,
				(u_int32_t) map.pr_vaddr, map.pr_size,
				(map.pr_mflags & MA_PHYS) ? 'P' : ' ',
				(map.pr_mflags & MA_STACK) ? 'S' : ' ',
				(map.pr_mflags & MA_BREAK) ? 'B' : ' ',
				(map.pr_mflags & MA_SHARED) ? 'H' : ' ',
				(map.pr_mflags & MA_READ) ? 'R' : ' ',
				(map.pr_mflags & MA_WRITE) ? 'W' : ' ',
				(map.pr_mflags & MA_EXEC) ? 'X' : ' ',
				map.pr_mapname);
		n++;
	}

	close(fd);
}

string StringF(const char* format, ...)
{
	char buffer[256];

	va_list ap;
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);

	return string(buffer);
}
