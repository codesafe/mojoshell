#ifndef _COLOR_MANAGER_
#define _COLOR_MANAGER_

#include "predef.h"
#include "ini.h"
#include "stringutil.h"
#include "xml/xml.h"


//////////////////////////////////////////////////////////////////////////

#define	SYSTEM_COLOR	0
#define	EXT_COLOR		1

//////////////////////////////////////////////////////////////////////////

class ColorManager
{
private :
	static ColorManager*	instance;
	ColorManager();
	~ColorManager();

public:
	static ColorManager *GetInstance()
	{
		if (instance == NULL)
			instance = new ColorManager;
		return instance;
	}

	bool	ExistColor(int type, String tag);
	unsigned long GetColor(int type, String tag);
	void SetColor(int type, String tag, unsigned long color);
	void RemoveColor(int type, String tag);

	std::map<String, unsigned long> GetAlltable(int type) 
	{ 
		if( type == SYSTEM_COLOR )
			return systemtable; 
		else
			return extcolortable; 
	}

	bool	SaveXML();

private :
	std::map<String, unsigned long>	systemtable;
	std::map<String, unsigned long>	extcolortable;

	CXml		xml;
	CXmlNode*	rootnode;

};


#endif