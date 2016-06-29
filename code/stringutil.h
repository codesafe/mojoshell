#ifndef _STRINGUTIL_H_
#define _STRINGUTIL_H_

#include <Windows.h>
#include <tchar.h>
#include <assert.h>
#include <string>
#include <vector>
#include <algorithm>

#include "predef.h"

#ifdef __cplusplus
namespace unicode
{

#define MAX_ARGS	14

#define ARGS_1  p[0]
#define ARGS_2  p[0], p[1]
#define ARGS_3  p[0], p[1], p[2]
#define ARGS_4  p[0], p[1], p[2], p[3]
#define ARGS_5  p[0], p[1], p[2], p[3], p[4]
#define ARGS_6  p[0], p[1], p[2], p[3], p[4], p[5]
#define ARGS_7  p[0], p[1], p[2], p[3], p[4], p[5], p[6]
#define ARGS_8  p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]
#define ARGS_9  p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8]
#define ARGS_10 p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9]
#define ARGS_11 p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10]
#define ARGS_12 p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11]
#define ARGS_13 p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12]
#define ARGS_14 p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13]

	inline int customScanf( const wchar *str, const wchar *format, va_list arglist)
	{
		const wchar *currBuff;
		void	*p[MAX_ARGS];
		wchar	currChar;
		int		numArgs = 0;

		currBuff = wcschr( format, L'%');
		if( currBuff == NULL ) return 0;
		do
		{
			//			currBuff = _wcsinc( currBuff );
			currBuff = &currBuff[1];
			currChar = wchar( *currBuff );
			if ( currChar == NULL )
			{

			}
			else
			{
				if ( numArgs >= MAX_ARGS ) { _ASSERT(0); return 0; }
				p[numArgs++] = va_arg( arglist, void * );

				if(currChar == L's')
				{
					p[numArgs++] = va_arg( arglist, void * );
				}
			}
			currBuff = wcschr( currBuff, '%' );
		} while ( currBuff != NULL );

		switch ( numArgs )
		{
		case  0: return 0;
		case  1: return swscanf_s( str, format, ARGS_1  );
		case  2: return swscanf_s( str, format, ARGS_2  );
		case  3: return swscanf_s( str, format, ARGS_3  );
		case  4: return swscanf_s( str, format, ARGS_4  );
		case  5: return swscanf_s( str, format, ARGS_5  );
		case  6: return swscanf_s( str, format, ARGS_6  );
		case  7: return swscanf_s( str, format, ARGS_7  );
		case  8: return swscanf_s( str, format, ARGS_8  );
		case  9: return swscanf_s( str, format, ARGS_9  );
		case 10: return swscanf_s( str, format, ARGS_10 );
		case 11: return swscanf_s( str, format, ARGS_11 );
		case 12: return swscanf_s( str, format, ARGS_12 );
		case 13: return swscanf_s( str, format, ARGS_13 );
		case 14: return swscanf_s( str, format, ARGS_14 );
		default :
			_ASSERT(0);
			break;
		}

		return 0;
	}

	inline int scanf(const wchar *buffer, const wchar * format, ...)
	{
		int result;
		va_list args; 
		va_start(args, format); 
		result = customScanf(buffer, format, args);
		va_end(args); 

		return result;
	}

	inline void sprintf(wchar_t *dest, int destcnt, const wchar_t * format, ...)
	{
		va_list ap;
		va_start(ap, format);
		vswprintf(dest, destcnt, format, ap);
		va_end(ap);
	}

	inline int atoi(const wchar * str) { return _wtoi(str); }
	inline float atof(const wchar * str) { return (float) _wtof(str); }
	inline void strcpy(wchar * dest, int destsize, const wchar * src) { wcscpy_s(dest, destsize, src); }
	inline void strncpy(wchar * dest, int destsize, const wchar * src, int count ) { wcsncpy_s(dest, destsize, src, count ); }
	inline int strlen(const wchar *buffer) { return (int)wcslen(buffer); }
	inline int strcmp(const wchar * str1, const wchar * str2) { return wcscmp(str1, str2); }
	inline int strcat(wchar * str1, int size, const wchar * str2) { return wcscat_s(str1, size, str2); }
	inline const wchar * strstr(const wchar * str1, const wchar * str2) { return wcsstr(str1, str2); }
	inline wchar * strstr(wchar * str1, const wchar * str2) { return wcsstr(str1, str2); }
	inline wchar * strchr(wchar * str1, wchar c) { return wcschr(str1, c); }

	inline const wchar * strpbrk(const wchar * str1, const wchar * str2) { return wcspbrk(str1, str2); }

	inline std::wstring Format(const wchar* fmt, ...)
	{
		int size = 512;
		wchar* buffer = 0;
		buffer = new wchar[size];
		va_list vl;
		va_start(vl, fmt);
		int nsize = vswprintf(buffer, size, fmt, vl);
		if(size<=nsize)
		{ 
			//fail delete buffer and try again
			delete[] buffer;
			buffer = 0;
			buffer = new wchar[nsize+1]; //+1 for /0
			nsize = vswprintf(buffer, size, fmt, vl);
		}
		std::wstring ret(buffer);
		va_end(vl);
		delete[] buffer;
		return ret;
	}

	inline const wchar * format(const wchar * fmt, ...)
	{
		_declspec( thread ) static wchar _temp[8][1024];
		_declspec( thread ) static int _tempcnt=0;
		int idx = _tempcnt++&7;
		va_list ap;
		va_start(ap, fmt);
		vswprintf(_temp[idx], _countof(*_temp), fmt, ap);
		va_end(ap);
		return _temp[idx];
	}

	inline void convstr_s(wchar_t * dest, int destsize, const char *src, bool utf8=false) { MultiByteToWideChar(utf8 == true ? CP_UTF8 : CP_ACP, 0, src, (int)::strlen(src)+1, dest, destsize == _TRUNCATE ? (int)::strlen(src)+1 : destsize); }
	inline void convstr_s(char * dest, int destsize, const wchar *src, bool utf8=false) { WideCharToMultiByte(utf8 == true ? CP_UTF8 : CP_ACP, 0, src, (int)wcslen(src)+1, dest, destsize == _TRUNCATE ? (int)wcslen(src)*2+1 : destsize, NULL, NULL); }

	inline std::wstring replace(const std::wstring& source, wchar replaceThis, wchar replaceWith)
	{
		std::wstring ret;
		ret.resize(source.length());

		for (unsigned i = 0; i < ret.length(); ++i)
		{
			if (source[i] == replaceThis)
				ret[i] = replaceWith;
			else
				ret[i] = source[i];
		}

		return ret;
	}

	inline void splitPath(const std::wstring& fullPath, std::wstring& pathName, std::wstring& fileName, std::wstring& extension, bool dirpath = false)
	{
		std::wstring fullPathCopy = replace(fullPath, '\\', '/');

		if( !dirpath )
		{
			size_t extPos = fullPathCopy.rfind('.');
			if (extPos != std::wstring::npos)
			{
				extension = fullPathCopy.substr(extPos);
				fullPathCopy = fullPathCopy.substr(0, extPos);
			}
			else
				extension = _T("");
		}

		size_t pathPos = fullPathCopy.rfind('/');
		if (pathPos != std::wstring::npos)
		{
			fileName = fullPathCopy.substr(pathPos + 1);
			pathName = fullPathCopy.substr(0, pathPos + 1);
		}
		else
		{
			fileName = fullPathCopy;
			pathName = _T("");
		}
	}

	inline std::wstring getPath(const std::wstring& fullPath)
	{
		std::wstring path, file, extension;
		splitPath(fullPath, path, file, extension);
		return path;
	}

	inline std::wstring getFileName(const std::wstring& fullPath)
	{
		std::wstring path, file, extension;
		splitPath(fullPath, path, file, extension);
		return file;
	}

	inline std::wstring getExtension(const std::wstring& fullPath, bool lowerCaseExtension)
	{
		std::wstring path, file, extension;
		splitPath(fullPath, path, file, extension, lowerCaseExtension);
		return extension;
	}

	// String tokenizer
	inline std::vector<String> split(const wchar *str, wchar c)
	{
		std::vector<String> result;
		do
		{
			const wchar *begin = str;

			while(*str != c && *str)
				str++;

			result.push_back(String(begin, str));
		} while (0 != *str++);

		return result;
	}


	inline String replaceAll(const String &str, const String &pattern, const String &replace)
	{   
		String result = str;   
		String::size_type pos = 0;   
		String::size_type offset = 0;   

		while((pos = result.find(pattern, offset)) != String::npos)   
		{   
			result.replace(result.begin() + pos, result.begin() + pos + pattern.size(), replace);   
			offset = pos + replace.size();   
		}   
		return result;   
	}

	inline void Tokenize(const String& str, std::vector<String>& tokens, const String& delimiters)
	{
		String::size_type lastPos = str.find_first_not_of(delimiters, 0);
		String::size_type pos = str.find_first_of(delimiters, lastPos);

		while (String::npos != pos || String::npos != lastPos)
		{
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			lastPos = str.find_first_not_of(delimiters, pos);
			pos = str.find_first_of(delimiters, lastPos);
		}
	}

	inline void Upper(String &str)
	{
		transform(str.begin(), str.end(), str.begin(), toupper);
	}


};
#endif


#endif

