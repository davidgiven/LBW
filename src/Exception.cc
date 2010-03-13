/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "Exception.h"
#include <errno.h>

Exception::Exception(const string& message):
	_errno(errno)
{
	_message = StringF("%s: %s",
			message.c_str(),
			strerror(_errno));
}
