#ifndef _CONFIGMANAGER_
#define _CONFIGMANAGER_

#include "predef.h"
#include "stringutil.h"
#include "xml/xml.h"

class ConfigManager
{
private :
	static ConfigManager*	instance;

	ConfigManager();
	~ConfigManager();

public:
	static ConfigManager *GetInstance()
	{
		if (instance == NULL)
			instance = new ConfigManager;
		return instance;
	}

	bool		Open();
	void		Close();
	void		Save();

	void	SetConfig(String key, String value);
	String	GetConfigValue(String key);
	int		GetConfigInt(String key);

	String	GetFuntionKey(String key, String subkey);
	void	SetFuntionKey(String key, String subkey, String value);

	void	SetSVN(String name, String exe, String path);
	bool	GetSVN(String name , SVNInfo &info);
	void	GetSVN(std::vector<SVNInfo> &info);
	bool	ExistSVN(String name);
	void	RemoveSVN(String name);

	void	SetTortoisePath(String path);
	String	GetTortoisePath();

	String	GetCommonOption(String key);
	bool	GetCommonOptionBool(String key);
	void	SetCommonOption(String key, String value);
	void	SetCommonOptionBool(String key, bool value);

	CXmlNode*	FindNode(String key);

private:

	CXml		mainconfig_xml;		// mainconfig
	CXmlNode*	rootnode;

};


#endif