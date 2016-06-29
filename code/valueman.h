#ifndef _VALUEMAN_
#define _VALUEMAN_

#include "predef.h"
#include "stringutil.h"

class Valueman
{

private :
	static Valueman*	instance;

	Valueman();
	~Valueman();

public:
	static Valueman *GetInstance()
	{
		if (instance == NULL)
		{
			instance = new Valueman;
		}
		return instance;
	}


	void		reset();
	void		addvalue(const wchar *key, const wchar *value);
	String		getvalue(const wchar *key);
	void		remove(const wchar *key);

private :

	std::map<String, String>	valuelist;
};


#endif