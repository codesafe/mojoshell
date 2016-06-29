#include "ini.h"
//#include "Blowfish.h"
#include "stringutil.h"


const wchar tag[2] = { '[', ']' };
const unsigned long encode_sig = 0xc00ec0de;

CINI::CINI()
{
	m_buffer = NULL;
	m_len = 0;
	m_pos = 0;
	memset(m_encryptkey, 0, sizeof(m_encryptkey));
}

CINI::~CINI()
{
	if( m_buffer )
		delete [] m_buffer;
}


bool CINI::Parse(char* buffer, int len)
{
	std::wstring key;
	std::wstring value;

	wchar *str = (wchar *)buffer;
	enum
	{
		_idle,
		_readkey,
		_readvalue,
	} state = _idle;

	for (int i=0; i< len/2; i++)
	{
 		if( str[i] == 0xfeff )
		{
			_ASSERT(state == _idle);
 			continue;
		}

		if( str[i] == tag[0] && state == _idle)
		{
			state = _readkey;
			continue;
		}
	
		if( str[i] == tag[1] && str[i+1] == 0x0d && str[i+2] == 0x0a && state == _readkey)
		{
			state = _readvalue;
			i+=3-1;
			continue;
		}

		if( state == _readvalue && str[i] == 0x0d && str[i+1] == 0x0a )
		{
			m_List.insert( std::make_pair(key, value) );
			state = _idle;
			key.clear();
			value.clear();

			i+=2-1;
			continue;
		}

		if( state == _readkey )
		{
			key += str[i];
		}

		if( state == _readvalue )
		{
			value += str[i];
		}
	}

	if( !key.empty() && !value.empty() )
	{
		_ASSERT(state == _readvalue);
		m_List.insert( std::make_pair(key, value) );
	}

	return true;
}

bool CINI::LoadINI(const wchar* fname)
{
	if (m_encryptkey[0] != '\0')
	{
		bool ret = false;
		//FILE* fp=unicode::fopen(fname, L"rb", false);
		FILE *fp;
		_wfopen_s(&fp, fname, L"rb");

		if (fp != NULL)
		{
			char *buffer = NULL;
			fseek(fp, 0, SEEK_END);
			int len  = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			buffer = new char [len];
			fread(buffer, 1, len, fp);
			fclose(fp);

			unsigned char sig[4];
			memcpy(sig, buffer, 4);
			ret = Parse(buffer, len);

			delete [] buffer;
		}

		return ret;
	}
	else
	{
		//FILE* fp=unicode::fopen(fname, L"rb", false);
		FILE* fp;
		_wfopen_s(&fp, fname, L"rb");

		if (fp != NULL)
		{
			char *buffer = NULL;
			fseek(fp, 0, SEEK_END);
			int len  = ftell(fp);
			buffer = new char [len];

			fseek(fp, 0, SEEK_SET);
			fread(buffer, 1 ,len, fp);
			fclose(fp);

			// parse
			bool ret = Parse(buffer, len);
			delete [] buffer;

			return ret;
		}
		return false;
	}
}

void CINI::SetKey(const wchar* key)
{
	//unicode::conv_s(m_encryptkey, sizeof(m_encryptkey), key);
}

#define ALLOCSIZE	32

void	CINI::MemWrite(void *data, int len)
{
	int dpos = 0;
	while( len > 0 )
	{
		int dlen = len;
		if( len > ALLOCSIZE )
			dlen = ALLOCSIZE; 
		len -= ALLOCSIZE;

		if( m_len - m_pos < dlen )
		{
			if( m_buffer )
			{
				// realloc
				char *temp = new char[ m_len + ALLOCSIZE ];
				memcpy(temp, m_buffer, m_pos );
				delete [] m_buffer;

				m_buffer = temp;
				m_len += ALLOCSIZE;
			}
			else
			{
				m_buffer = new char[ ALLOCSIZE ];
				m_len = ALLOCSIZE;
			}
		}

		memcpy( m_buffer+m_pos, (char*)data+dpos, dlen );
		dpos += dlen;
		m_pos += dlen;
	}


}

bool CINI::SaveINI(const wchar* fname)
{
	unsigned short sig = 0xfeff;
	unsigned short end[2] = { 0x0d, 0x0a };

	MemWrite(&sig, sizeof(sig));

	std::map <std::wstring, std::wstring>::iterator it = m_List.begin();
	for (; it != m_List.end(); it++	)
	{
		MemWrite((void*)&tag[0], sizeof(tag[0]));
		MemWrite((void*)it->first.c_str(), it->first.size() * 2);
		MemWrite((void*)&tag[1], sizeof(tag[1]));

		MemWrite(&end[0], sizeof(end[0]));
		MemWrite(&end[1], sizeof(end[1]));


		MemWrite((void*)it->second.c_str(), it->second.size() * 2);
		MemWrite(&end[0], sizeof(end[0]));
		MemWrite(&end[1], sizeof(end[1]));
	}


	//FILE* fp=unicode::fopen(fname, L"wb", false);
	FILE* fp;
	_wfopen_s(&fp, fname, L"wb");
	
	if (fp == NULL)
		return false;

	fwrite(m_buffer, 1, m_pos, fp);
	fclose(fp);

	if( m_buffer )
	{
		delete [] m_buffer;
		m_buffer = NULL;
		m_pos = 0;
		m_len = 0;
	}

	return true;
}

bool CINI::GetBool(const wchar* name, bool def)
{
	const wchar *s = Find(name);
	if( s )
	{
		return !unicode::strcmp(s, L"true") ? true : false;
	}
	
	return def;
}

int CINI::GetInt(const wchar* name, int def)
{
	const wchar *s = Find(name);
	if( s )
	{
		return unicode::atoi(s);
	}

	return def;
}

float CINI::GetFloat(const wchar* name, float def)
{
	const wchar *s = Find(name);
	if( s )
	{
		return unicode::atof(s);
	}

	return def;
}

const wchar *CINI::GetString(const wchar* name, const wchar* def)
{
	const wchar *s = Find(name);
	if( s )
	{
		return s;
	}

	return def;
}

void CINI::Set(const wchar* name, bool b)
{
	std::wstring key(name);
	std::wstring value(b == true ? L"true" : L"false");

	std::map <std::wstring, std::wstring>::iterator it = m_List.find(key);
	if( it != m_List.end() )
	{
		it->second = value;
		return;
	}

	// 추가
	m_List.insert( std::make_pair(std::wstring(name), value) );
}

void CINI::Set(const wchar* name, int n)
{
	std::wstring key(name);
	std::wstring value = unicode::format(L"%d", n);
	
	std::map <std::wstring, std::wstring>::iterator it = m_List.find(key);
	if( it != m_List.end() )
	{
		it->second = value;
		return;
	}

	// 추가
	m_List.insert( std::make_pair(std::wstring(name), value) );
}

void CINI::Set(const wchar* name, float f)
{
	std::wstring key(name);
	std::wstring value = unicode::format(L"%f", f);

	std::map <std::wstring, std::wstring>::iterator it = m_List.find(key);
	if( it != m_List.end() )
	{
		it->second = value;
		return;
	}

	// 추가
	m_List.insert( std::make_pair(std::wstring(name), value) );

}

void CINI::Set(const wchar* name, const wchar *str)
{
	std::wstring key(name);
	std::wstring value(str);

	std::map <std::wstring, std::wstring>::iterator it = m_List.find(key);
	if( it != m_List.end() )
	{
		it->second = value;
		return;
	}

	// 추가
	m_List.insert( std::make_pair(std::wstring(name), value) );

}

bool	CINI::Remove(const wchar* name)
{
	std::map <std::wstring, std::wstring>::iterator it = m_List.find(name);
	if( it != m_List.end() )
	{
		m_List.erase(it);
		return true;
	}

	return false;
}

const wchar *CINI::Find(const wchar *k)
{
	std::wstring key(k);
	std::map <std::wstring, std::wstring>::iterator it = m_List.find(key);
	if( it != m_List.end() )
	{
		return it->second.c_str();
	}

	return NULL;
}