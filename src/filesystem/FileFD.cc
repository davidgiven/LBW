/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

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
