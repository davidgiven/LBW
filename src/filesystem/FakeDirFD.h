/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef FAKEDIRFD_H
#define FAKEDIRFD_H

#include "FD.h"

class FakeDirFD : public FD
{
public:
	FakeDirFD(int fd, VFSNode* node);
	~FakeDirFD();
};

#endif
