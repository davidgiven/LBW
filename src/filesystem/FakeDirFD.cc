/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "filesystem/FakeDirFD.h"
#include "file.h"
#include <sys/uio.h>

FakeDirFD::FakeDirFD(int fd, VFSNode* node):
	FD(fd, node)
{
}

FakeDirFD::~FakeDirFD()
{
}
