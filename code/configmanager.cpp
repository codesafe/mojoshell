#include "configmanager.h"


const wchar *configname = L"mainconfig.xml";
ConfigManager*	ConfigManager::instance = NULL;


ConfigManager::ConfigManager()
{
	rootnode = NULL;
}

ConfigManager::~ConfigManager()
{

}

bool	ConfigManager::Open()
{
	bool ret;
	rootnode = mainconfig_xml.openXML(configname);
	ret = rootnode == NULL ? false : true;

	if( ret == false)
		rootnode = new CXmlNode;
	return ret;
}

void	ConfigManager::Save()
{
	if(rootnode)
		mainconfig_xml.saveXML(configname, rootnode->getChild());
}

void	ConfigManager::Close()
{
	Save();
	rootnode = NULL;
}

// COMMON
void	ConfigManager::SetConfig(String key, String value)
{
	// 있으면 overwrite, 없으면 생성
	CXmlNode *node = CXmlNode::findChild(rootnode, key.c_str());
	if( node )
	{
		node->setValue(value.c_str());
	}
	else
	{
		rootnode->addChild(key.c_str(), value.c_str());
	}
}

String	ConfigManager::GetConfigValue(String key)
{
	CXmlNode *node = CXmlNode::findChild(rootnode, key.c_str());
	if( node )
		return String(node->getValue());
	else
		rootnode->addChild(key.c_str(), L"");
	return String(L"");
}

int	ConfigManager::GetConfigInt(String key)
{
	CXmlNode *node = CXmlNode::findChild(rootnode, key.c_str());
	if( node )
	{
		int ret = unicode::atoi(node->getValue());
		return ret;
	}
	else
		rootnode->addChild(key.c_str(), L"");
	return -1;
}

// Function Key
String ConfigManager::GetFuntionKey(String key, String subkey)
{
	CXmlNode *node = CXmlNode::findChild(rootnode, key.c_str());
	if( node )
	{
		CXmlNode *cnode = CXmlNode::findChild(node, subkey.c_str());
		if( cnode )
		{
			return String(cnode->getValue());
		}
	}

	return String(L"");
}

void	ConfigManager::SetFuntionKey(String key, String subkey, String value)
{
	CXmlNode *node = CXmlNode::findChild(rootnode, key.c_str());
	if( node )
	{
		CXmlNode *cnode = CXmlNode::findChild(node, subkey.c_str());
		if( cnode )
		{
			cnode->setValue(value.c_str());
		}
		else
		{
			node->addChild(subkey.c_str(), value.c_str());
		}
	}
	else
	{
		node = rootnode->addChild(key.c_str(), L"");
		node->addChild(subkey.c_str(), value.c_str());
	}
}

void	ConfigManager::SetSVN(String name, String exec, String path)
{
	CXmlNode *node = CXmlNode::findChild(rootnode, L"svnconfig");
	if( node )
	{
		node = node->addChild(L"svn", L"");
		node->addAttribute(L"name", name.c_str());
		node->addAttribute(L"exec", exec.c_str());
		node->addAttribute(L"path", path.c_str());
	}
	else
	{
		// 생성
		node = rootnode->addChild(L"svnconfig", L"");
		node = node->addChild(L"svn", L"");
		node->addAttribute(L"name", name.c_str());
		node->addAttribute(L"exec", exec.c_str());
		node->addAttribute(L"path", path.c_str());
	}
}

bool	ConfigManager::GetSVN(String name , SVNInfo &info)
{
	CXmlNode *node = CXmlNode::findChild(rootnode, L"svnconfig");
	if( node )
	{
		CXmlNode *child = node->getChild();
		for ( ; child != NULL; child = child->getNext() )
		{
			String cname = child->getAttiribute(L"name");
			if( cname == name )
			{
				info.name = cname;
				info.exec = child->getAttiribute(L"exec");
				info.path = child->getAttiribute(L"path");
				return true;
			}
		}
	}

	return false;
}

void	ConfigManager::GetSVN(std::vector<SVNInfo> &info)
{
	CXmlNode *node = CXmlNode::findChild(rootnode, L"svnconfig");
	if( node )
	{
		CXmlNode *child = node->getChild();
		for ( ; child != NULL; child = child->getNext() )
		{
			SVNInfo f;
			f.name = child->getAttiribute(L"name");
			f.exec = child->getAttiribute(L"exec");
			f.path = child->getAttiribute(L"path");
			info.push_back(f);
		}
	}
}

bool	ConfigManager::ExistSVN(String name)
{
	CXmlNode *node = CXmlNode::findChild(rootnode, L"svnconfig");
	if( node )
	{
		CXmlNode *child = node->getChild();
		for ( ; child != NULL; child = child->getNext() )
		{
			String n = child->getAttiribute(L"name");
			if( name == n )
				return true;
		}
	}

	return false;
}

void	ConfigManager::RemoveSVN(String name)
{
	CXmlNode *node = CXmlNode::findChild(rootnode, L"svnconfig");
	if( node )
	{
		CXmlNode *child = node->getChild();
		for ( ; child != NULL; child = child->getNext() )
		{
			String n = child->getAttiribute(L"name");
			if( name == n )
			{
				CXmlNode::removeNode(child);
				return;
			}
		}
	}
}

// SVN
CXmlNode*	ConfigManager::FindNode(String key)
{
	return CXmlNode::findChild(rootnode, key.c_str());
}

// Common
String	ConfigManager::GetCommonOption(String key)
{
	CXmlNode *node = CXmlNode::findChild(rootnode, L"commonconfig");
	if( node )
	{
		CXmlNode *cnode = CXmlNode::findChild(node, key.c_str());
		if( cnode )
		{
			return String(cnode->getValue());
		}
	}
	
	return String(L"");
}

void	ConfigManager::SetTortoisePath(String path)
{
	CXmlNode *node = CXmlNode::findChild(rootnode, L"tortoisesvn");
	if( node )
	{
		node = node->addChild(L"path", path.c_str());
	}
	else
	{
		// 생성
		node = rootnode->addChild(L"tortoisesvn", L"");
		node = node->addChild(L"path", path.c_str());
	}
}


String	ConfigManager::GetTortoisePath()
{
/*
	CXmlNode *node = CXmlNode::findChild(rootnode, L"tortoisesvn");
	if( node )
	{
		CXmlNode *cnode = CXmlNode::findChild(node, L"path");
		if( cnode )
		{
			return String(cnode->getValue());
		}
	}
*/
	// regsitry에서 읽자
	wxRegKey key(wxRegKey::HKLM, "Software\\TortoiseSVN");

	wxString strValue;
	bool ret = key.QueryValue("ProcPath", strValue, false);
	if( ret == true )
	{
		return String(strValue);
	}

	return String(L"");
}


bool	ConfigManager::GetCommonOptionBool(String key)
{
	String ret = GetCommonOption(key);
	if( ret == L"true")
		return true;
	else
		return false;
}

void	ConfigManager::SetCommonOption(String key, String value)
{
	CXmlNode *node = CXmlNode::findChild(rootnode, L"commonconfig");
	if( node )
	{
		CXmlNode *cnode = CXmlNode::findChild(node, key.c_str());
		if( cnode )
		{
			cnode->setValue(value.c_str());
		}
		else
		{
			node->addChild(key.c_str(), value.c_str());
		}
	}
	else
	{
		// 생성
		node = rootnode->addChild(L"commonconfig", L"");
		node = node->addChild(key.c_str(), L"");
		node->setValue(value.c_str());
	}
}

void	ConfigManager::SetCommonOptionBool(String key, bool value)
{
	SetCommonOption(key, value ? L"true" : L"false");
}

