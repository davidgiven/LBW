/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef EXCEPTION_H
#define EXCEPTION_H

class Exception
{
public:
	Exception(const string& message);

	operator const string& () const
	{
		return _message;
	}

	operator int () const
	{
		return _errno;
	}

private:
	string _message;
	int _errno;
};

#endif
