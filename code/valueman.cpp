#include "valueman.h"

Valueman*	Valueman::instance = NULL;

Valueman::Valueman()
{

}
Valueman::~Valueman()
{

}

void	Valueman::reset()
{
	valuelist.clear();
}

void	Valueman::addvalue(const wchar *key, const wchar *value)
{
	std::map<String, String>::iterator it =	valuelist.find(String(key));

	if( it == valuelist.end() )
	{
		valuelist.insert( std::make_pair(String(key), String(value) ) );
		return;
	}

	// Overwrite
	it->second = value;
}


String Valueman::getvalue(const wchar *key)
{
	std::map<String, String>::iterator it =	valuelist.find(String(key));

	if( it != valuelist.end() )
	{
		return it->second.c_str();
	}

	return String(L"");
}

void	Valueman::remove(const wchar *key)
{
	std::map<String, String>::iterator it =	valuelist.find(String(key));
	if( it != valuelist.end() )
	{
		valuelist.erase(it);
		return;
	}
}