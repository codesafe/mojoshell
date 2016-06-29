

#ifndef _XML_
#define _XML_

#include "../predef.h"
#include "FastXml.h"

class CXml;

class CXmlNode
{
public :
	friend	class CXml;

public :

	struct	_XmlData
	{
		String	m_name;
		String	m_value;

		const wchar*	getName() { return m_name.c_str(); }
		const wchar*	getValue() { return m_value.c_str(); }
	};

public :
	CXmlNode() : prev(NULL), next(NULL), child(NULL), parent(NULL) {};
	CXmlNode(const wchar *name, const wchar *value) : next(NULL), child(NULL), parent(NULL) 
	{
		m_data.m_name = name;
		m_data.m_value = value;
	};

	void clear()
	{
		m_attribute.clear();

		if( child )
			delete child;
		child = NULL;

		if( next )
			delete next;
		next = NULL;
	}

	bool isEmpty()
	{
		return (m_attribute.size() == 0 && NULL == child  && NULL == next);
	}

	// destruct
	~CXmlNode()
	{
		m_attribute.clear();

		if( child )
			delete child;

// 		if( next )
// 			delete next;
	}

	void			setName(const wchar *name) { m_data.m_name = name; }
	void			setValue(const wchar *value) { if(value) m_data.m_value = value; }

	const wchar*		getName() { return m_data.m_name.c_str(); }
	const wchar*		getValue() { return m_data.m_value.c_str(); }

	int				getIntValue() { return unicode::atoi(m_data.m_value.c_str()); }
	float			getFloatValue() { return unicode::atof(m_data.m_value.c_str()); }

	static CXmlNode* find(CXmlNode* node, const wchar *key)
	{
		for( CXmlNode *s = node ; s != NULL; s = s->next )
		{
			if( s->m_data.m_name == key )
			{
				return s;
			}
		}

		// not found
		return NULL;
	}

	static CXmlNode* findChild(CXmlNode* node, const wchar *key)
	{
		CXmlNode* ch = node->getChild();

		for( ; ch != NULL; ch = ch->next )
		{
			if( ch->m_data.m_name == key )
			{
				return ch;
			}
		}
		// not found
		return NULL;
	}

	static void removeNode(CXmlNode* node)
	{
		if( node->prev )
		{
			if( node->next )
			{
				node->prev->next = node->next;
				node->next->prev = node->prev;
			}
			else
			{
				node->prev->next = node->next;
			}
		}
		else
		{
			// 맨앞
			if( node->next )
			{
				node->parent->child = node->next;
				node->next->prev = node->prev;
			}
			else
			{
				node->parent->child = NULL;
			}
		}

		delete node;
	}

	CXmlNode*	getNext() { return next; }
	CXmlNode*	getParent() { return parent; }
	CXmlNode*	getChild() { return child; }
	_XmlData*	getData() { return &m_data; }

	CXmlNode*	addData(const wchar *name, const wchar *value){ return addNext(name, value); };
	CXmlNode*	addData(const wchar *name, int v)
	{
		return addNext(name, unicode::format(L"%d", v));
	};

	CXmlNode*	addData(const wchar *name, float v)
	{
		return addNext(name, unicode::format(L"%f", v));
	};

	CXmlNode*	addNext(const wchar *name, const wchar *value)
	{
		CXmlNode *node = new CXmlNode;
		node->m_data.m_name = name;
		if(value) node->m_data.m_value = value;

		node->prev = this;
		node->next = this->next;
		if(node->next)
			node->next->prev = node;
		this->next = node;
		node->parent = this->parent;

		return node;
	}

	CXmlNode*	addChild(const wchar *name, const wchar *value)
	{
		CXmlNode *node = new CXmlNode;
		node->m_data.m_name = name;
		if(value) node->m_data.m_value = value;

		if( this->child )
		{
			CXmlNode *t = this->child;
			while(t->next)
			{
				t = t->next;
			}

			t->next = node;
			node->prev = t;
			node->parent = t->parent;
		}
		else
		{
			this->child = node;
			node->parent = this;
		}

		return node;
	}

	std::vector<_XmlData>	*getAttiribute() { return &m_attribute; }
	const wchar* getAttiribute(const wchar *key) 
	{ 
		int n = m_attribute.size();
		for(int i=0; i<n; i++)
		{
			if( m_attribute[i].m_name == key )
				return m_attribute[i].m_value.c_str();
		}
		return NULL;
	}

	bool	addAttribute(const wchar *name, const wchar *value)
	{
		int n = m_attribute.size();
		for(int i=0; i<n; i++)
		{
			if( m_attribute[i].m_name == name )
			{
				m_attribute[i].m_value = value;
				return false;
			}
		}

		CXmlNode::_XmlData att;
		att.m_name = name;
		att.m_value = value;
		m_attribute.push_back(att);
		return true;
	}

private :

	_XmlData					m_data;
	std::vector<_XmlData>		m_attribute;

	CXmlNode	*prev;
	CXmlNode	*next;
	CXmlNode	*parent;
	CXmlNode	*child;
};

////////////////////////////////////////////////////////////////////////////

class CXml : public FastXmlInterface
{
public :
		enum
		{
			SEARCH_LEVEL = 0,
			SEARCH_TREE,
		};

public :

	CXml() : m_rootnode(NULL), m_lastnode(NULL)
	{

	}

	~CXml()
	{
		ReleaseNode();
	};

	// Ascii open
	CXmlNode *openXML(const wchar* filename)
	{
		FastXml *xml = createFastXml();
		bool ret = xml->processXmlFile(filename, this);
		releaseFastXml(xml);

		return ret ? m_rootnode : NULL;
	}

	CXmlNode *openXML(const wchar *buffer, int len)
	{
		FastXml *xml = createFastXml();
		bool ret = xml->processXml(buffer, len, this);
		releaseFastXml(xml);

		return ret ? m_rootnode : NULL;
	};

	static void	saveNode(FILE *fp, CXmlNode *node, int tab)
	{
#ifdef WIN32
		char utf8buffer[2048];
		char utf8buffer2[2048];

		CXmlNode *n = node;
		for ( ; n != NULL; n = n->getNext() )
		{
			String name = n->getName();
			String value = n->getValue();

			// print tab
			//for (int t=0;t<tab;t++)	fwprintf_s(fp, L"\t");
			for (int t=0;t<tab;t++)	fprintf_s(fp, "\t");

			unicode::convstr_s(utf8buffer, sizeof(utf8buffer), name.c_str(), true);
			//fwprintf_s(fp, L"<%s", utf8buffer);
			fprintf_s(fp, "<%s", utf8buffer);

			// save attribute
			for(size_t i=0; i< n->m_attribute.size(); i++)
			{
				unicode::convstr_s(utf8buffer, sizeof(utf8buffer), n->m_attribute[i].getName(), true);	
				unicode::convstr_s(utf8buffer2, sizeof(utf8buffer2), n->m_attribute[i].getValue(), true);
				//fwprintf_s(fp, L" %s=\"%s\"", utf8buffer, utf8buffer2);
				fprintf_s(fp, " %s=\"%s\"", utf8buffer, utf8buffer2);
			}
			
			if(value.size() > 0)
			{
				unicode::convstr_s(utf8buffer, sizeof(utf8buffer), value.c_str(), true);
				unicode::convstr_s(utf8buffer2, sizeof(utf8buffer2), name.c_str(), true);

// 				fwprintf_s(fp, L">");
// 				fwprintf_s(fp, L"%s", utf8buffer);
// 				fwprintf_s(fp, L"</%s>\n",utf8buffer2);

				fprintf_s(fp, ">");
				fprintf_s(fp, "%s", utf8buffer);
				fprintf_s(fp, "</%s>\n",utf8buffer2);

			}
			else
			{
				CXmlNode *child = n->getChild();
				if( child )
				{
					//fwprintf_s(fp, L">\n");
					fprintf_s(fp, ">\n");
					saveNode(fp , child, tab+1);
					//for (int t=0;t<tab;t++)	fwprintf_s(fp, L"\t");
					for (int t=0;t<tab;t++)	fprintf_s(fp, "\t");

					unicode::convstr_s(utf8buffer, sizeof(utf8buffer), name.c_str(), true);
					//fwprintf_s(fp, L"</%s>\n",utf8buffer);
					fprintf_s(fp, "</%s>\n",utf8buffer);
				}
				else
				{
					//fwprintf_s(fp, L"/>\n");
					fprintf_s(fp, "/>\n");
				}

			}
		}
#endif
	}

	static bool	saveXML(const String &filename, CXmlNode *root)
	{
#ifdef WIN32
		
		FILE *fp = NULL;
		_wfopen_s(&fp, filename.c_str(), L"wt");
		if( fp == NULL ) return false;

		unsigned char header[3] = { 0xef, 0xbb, 0xbf };
		fwrite(header, 3, 1, fp);

		fprintf_s(fp, "<root>\n");
		//fwprintf_s(fp, L"<root>\n");
		int tab = 1;
		saveNode(fp, root, tab);
		//fwprintf_s(fp, L"</root>\n");
		fprintf_s(fp, "</root>\n");

		fclose(fp);
#endif
		return true;
	}


	bool processComment(const wchar *comment) { return true; }
	void processClose(const wchar *element)
	{
		if(m_openchild == false)
		{
			// <?xml version="1.0" encoding="utf-8"?>
			if( m_lastnode == NULL ) return;
			m_lastnode = m_lastnode->parent;
		}

		m_openchild = false;
		return;
	}

	bool processElement(const wchar *elementName, int argc, const wchar **argv, const wchar *elementData, int lineno)
	{
		assert(elementName);
		if( m_rootnode == NULL )
		{
			m_rootnode = CreateNode();
			m_rootnode->setName(elementName);
			m_rootnode->setValue(elementData);
			m_lastnode = m_rootnode;

			for ( int i=0; i<argc; i+=2 )
				m_lastnode->addAttribute(argv[i], argv[i+1]);
		}
		else
		{
			if( m_openchild )
			{
				//_TRACE("child : %s -> %s\n",m_lastnode->getName().c_str(), elementName);
				m_lastnode = m_lastnode->addChild(elementName, elementData);
			}
			else
			{
				//_TRACE("next : %s -> %s\n",m_lastnode->getName().c_str(), elementName);
				m_lastnode = m_lastnode->addNext(elementName, elementData);
			}

			// 
			for ( int i=0; i<argc; i+=2 )
				m_lastnode->addAttribute(argv[i], argv[i+1]);
		}
		m_openchild = true;

		return true;
	}

	CXmlNode	*getRoot() { return m_rootnode; }

private :
	CXmlNode	*CreateNode()
	{
		CXmlNode *node = new CXmlNode;
		return node;
	}

	void	ReleaseNode()
	{
		if(m_rootnode)
			delete m_rootnode;
	}

	CXmlNode*	m_rootnode;
	CXmlNode*	m_lastnode;

	bool		m_openchild;
};





#endif
