#ifndef _INI_
#define _INI_

#include <map>
//#include "stringutil.h"

typedef wchar_t wchar;

class CINI
{
public :
	CINI();
	~CINI();

	bool LoadINI(const wchar* fname);
	bool SaveINI(const wchar* fname);
	void SetKey(const wchar* key);

	bool	GetBool(const wchar* name, bool def=false);
	int		GetInt(const wchar* name, int def=0);
	float	GetFloat(const wchar* name, float def=0.0f);
	const wchar *GetString(const wchar* name, const wchar* def=NULL);

	void Set(const wchar* name, bool b);
	void Set(const wchar* name, int n);
	void Set(const wchar* name, float f);
	void Set(const wchar* name, const wchar *str);

	bool	Remove(const wchar* name);
	std::map <std::wstring, std::wstring> GetAllTable() { return m_List; }


private :

	const wchar *Find(const wchar *key);
	bool		Parse(char* buffer, int len);
	void		MemWrite(void *data, int len);
	std::map <std::wstring, std::wstring> m_List;

	char	m_encryptkey[256];
	char *	m_buffer;
	int		m_len;
	int		m_pos;
};



#endif