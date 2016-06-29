#include "colormanager.h"
#include "stringutil.h"

const wchar *xmlfile = { L"color.xml" };
ColorManager*	ColorManager::instance = NULL;

ColorManager::ColorManager()
{
	rootnode = xml.openXML(xmlfile);
	if(rootnode == NULL)
		rootnode = new CXmlNode;

	CXmlNode *systemnode = CXmlNode::findChild(rootnode, L"systemcolor");
	if( systemnode )
	{
		CXmlNode *child = systemnode->getChild();
		for ( ; child != NULL; child = child->getNext() )
		{
			const wchar *name = child->getAttiribute(L"name");
			const wchar *value = child->getAttiribute(L"value");
			unsigned long color;
			unicode::scanf(value, L"%x", &color);
			color = 0xff000000|color;
			systemtable.insert(std::make_pair(String(name), color));
		}
	}

	CXmlNode *extnode = CXmlNode::findChild(rootnode, L"extcolor");
	if( extnode )
	{
		CXmlNode *child = extnode->getChild();
		for ( ; child != NULL; child = child->getNext() )
		{
			const wchar *name = child->getAttiribute(L"name");
			const wchar *value = child->getAttiribute(L"value");
			unsigned long color;
			unicode::scanf(value, L"%x", &color);
			color = 0xff000000|color;
			extcolortable.insert(std::make_pair(String(name), color));
		}
	}
}

ColorManager::~ColorManager()
{
}

// 이미 있는가??
bool	ColorManager::ExistColor(int type, String tag)
{
	if( type == SYSTEM_COLOR )
	{
		std::map<String, unsigned long>::iterator it = systemtable.find(tag);
		if( it != systemtable.end() )
			return true;
	}
	else if( type == EXT_COLOR )
	{
		std::map<String, unsigned long>::iterator it = extcolortable.find(tag);
		if( it != extcolortable.end() )
			return true;
	}

	return false;
}

unsigned long ColorManager::GetColor(int type, String tag)
{
	unsigned long color = 0xffffffff;
	if( type == SYSTEM_COLOR )
	{
		std::map<String, unsigned long>::iterator it = systemtable.find(tag);
		if( it != systemtable.end() )
			color = it->second;
	}
	else if( type == EXT_COLOR )
	{
		std::map<String, unsigned long>::iterator it = extcolortable.find(tag);
		if( it != extcolortable.end() )
			color = it->second;
	}

	return color;
}

void ColorManager::RemoveColor(int type, String tag)
{
	if( type == SYSTEM_COLOR )
	{
		std::map<String, unsigned long>::iterator it = systemtable.find(tag);
		if( it != systemtable.end() )
		{
			systemtable.erase(it);

			CXmlNode *systemnode = CXmlNode::findChild(rootnode, L"systemcolor");
			if( systemnode )
			{
				CXmlNode *child = systemnode->getChild();
				for(; child != NULL; child = child->getNext())
				{
					String name = child->getAttiribute(L"name");
					if( name == tag )
					{
						CXmlNode::removeNode(child);
						break;
					}
				}
			}
		}	
	}
	else if( type == EXT_COLOR )
	{
		std::map<String, unsigned long>::iterator it = extcolortable.find(tag);
		if( it != extcolortable.end() )
		{
			extcolortable.erase(it);
			
			CXmlNode *extmnode = CXmlNode::findChild(rootnode, L"extcolor");
			if( extmnode )
			{
				CXmlNode *child = extmnode->getChild();
				for(; child != NULL; child = child->getNext())
				{
					String name = child->getAttiribute(L"name");
					if( name == tag )
					{
						CXmlNode::removeNode(child);
						break;
					}
				}
			}
		}
	}
}

void ColorManager::SetColor(int type, String tag, unsigned long color)
{
	if( type == SYSTEM_COLOR )
	{
		std::map<String, unsigned long>::iterator it = systemtable.find(tag);
		if( it == systemtable.end() )
		{
			systemtable.insert(std::make_pair(tag, color));

			CXmlNode *systemnode = CXmlNode::findChild(rootnode, L"systemcolor");
			if( systemnode )
			{
				wchar colorstr[32];
				color = 0x00FFFFFF & color;
				unicode::sprintf(colorstr, _countof(colorstr), L"%x", color);

				CXmlNode *node = systemnode->addChild(L"color",L"");
				node->addAttribute(L"name", tag.c_str());
				node->addAttribute(L"value", colorstr);
			}
		}	
		else
		{
			it->second = color;
			CXmlNode *systemnode = CXmlNode::findChild(rootnode, L"systemcolor");
			if( systemnode )
			{
				CXmlNode *child = systemnode->getChild();
				for(; child != NULL; child = child->getNext())
				{
					String name = child->getAttiribute(L"name");
					if( name == tag )
					{
						wchar colorstr[32];
						color = 0x00FFFFFF & color;
						unicode::sprintf(colorstr, _countof(colorstr), L"%x", color);
						child->addAttribute(L"value", colorstr);
						break;
					}
				}
			}
		}
	}
	else if( type == EXT_COLOR )
	{
		std::map<String, unsigned long>::iterator it = extcolortable.find(tag);
		if( it == extcolortable.end() )
		{
			extcolortable.insert(std::make_pair(tag, color));
			CXmlNode *extmnode = CXmlNode::findChild(rootnode, L"extcolor");
			if( extmnode )
			{
				wchar colorstr[32];
				color = 0x00FFFFFF & color;
				unicode::sprintf(colorstr, _countof(colorstr), L"%x", color);

				CXmlNode *node = extmnode->addChild(L"color",L"");
				node->addAttribute(L"name", tag.c_str());
				node->addAttribute(L"value", colorstr);
			}
		}
		else
		{
			it->second = color;
			CXmlNode *extmnode = CXmlNode::findChild(rootnode, L"extcolor");
			if( extmnode )
			{
				CXmlNode *child = extmnode->getChild();
				for(; child != NULL; child = child->getNext())
				{
					String name = child->getAttiribute(L"name");
					if( name == tag )
					{
						wchar colorstr[32];
						color = 0x00FFFFFF & color;
						unicode::sprintf(colorstr, _countof(colorstr), L"%x", color);
						child->addAttribute(L"value", colorstr);
						break;
					}
				}
			}
		}
	}
}

bool	ColorManager::SaveXML()
{
	return CXml::saveXML(xmlfile, rootnode->getChild());
}