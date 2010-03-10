/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef REF_H
#define REF_H

class HasRefCount
{
public:
	HasRefCount():
		_refcount(0)
	{
	}

	virtual ~HasRefCount()
	{
	}

	void Reference()
	{
		_refcount++;
	}

	void Dereference()
	{
		_refcount--;
		if (_refcount == 0)
			delete this;
	}

private:
	unsigned int _refcount;
};

template <class T> class Ref
{
public:
	Ref():
		_object(NULL)
	{
	}

	Ref(T* object):
		_object(object)
	{
		if (_object)
			_object->Reference();
	}

	Ref(const Ref<T>& other):
		_object(other._object)
	{
		if (_object)
			_object->Reference();
	}

	Ref& operator = (T* object)
	{
		if (object)
			object->Reference();
		if (_object)
			_object->Dereference();
		_object = object;
		return *this;
	}

	Ref& operator = (const Ref<T>& other)
	{
		return *this = other._object;
	}

	bool operator == (const Ref<T>& other) const
	{
		return _object == other._object;
	}

	bool operator == (T* other) const
	{
		return _object == other;
	}

	~Ref()
	{
		if (_object)
			_object->Dereference();
	}

	operator T* () const
	{
		return _object;
	}

	T* operator -> () const
	{
		return _object;
	}

private:
	T* _object;
};

#endif
