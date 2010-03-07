#include "globals.h"
#include "FileFD.h"

#if 0
void FileFD::Open(const string& pathname, int flags, int mode)
{
	_pathname = pathname;
}

string FileFD::Massage(const string& pathname)
{
	if (pathname[0] == '/')
		return Options.Chroot + pathname;
	return pathname;
}
#endif
