#include "globals.h"
#include <errno.h>

Exception::Exception(const string& message):
	_errno(errno)
{
	_message = StringF("%s: %s",
			message.c_str(),
			strerror(_errno));
}
