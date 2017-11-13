#ifndef _UTIL_
#define _UTIL_

#include <Windows.h>
#include <shobjidl.h>
#include <atlconv.h> 
#include <atlbase.h>
#include <strsafe.h>
#include <OAIdl.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <ShellAPI.h>
#include <algorithm>
#include <time.h>

#include "predef.h"
#include "lock.h"
//#include "./zip/zip.h"
#include "canvas.h"
#include "trace.h"

namespace utility
{
	static CCriticalSection	loglock;


		static unsigned int crc32_tab[] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
		0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
		0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
		0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
		0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
		0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
		0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
		0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
		0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
		0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
		0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
		0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
		0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
		0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
		0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
		0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
		0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
		0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
		0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
		0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
		0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
		0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
		0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
		0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
		0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
		0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
		0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};

	static unsigned long crc32(unsigned long crc, const char *buf, size_t size)
	{
		const unsigned char *p;
		p = (unsigned char *)buf;
		crc = crc ^ ~0U;
		while (size--)
			crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
		return crc ^ ~0U;
	}

	static unsigned long CalcCRC32(const wchar *filename)
	{
		const int BUF = 1024;
		char inbuf[BUF];
		
		int readSize = 0;
		__int64 len = 0;
		unsigned long crc = 0;

		FILE *fp = NULL;
		errno_t err = _wfopen_s(&fp, filename, L"rb");
		if(fp == NULL) return -1;
		fseek(fp, 0, SEEK_END);
		len  = _ftelli64(fp);
		fseek(fp, 0, SEEK_SET);
		while(1)
		{
			readSize = fread(inbuf, 1, BUF, fp);
			if(readSize <= 0)
				break;
			crc = crc32(crc,inbuf,readSize);
		}
		fclose(fp);

		return crc;
	}

	static unsigned long GetPathAttribute(const wchar *fname)
	{
		unsigned long ret = GetFileAttributes(fname);
		return ret;
	}

	static __int64 GetFileLength(const wchar *fname)
	{
		FILE *fp;
		__int64 len = 0;
		_wfopen_s(&fp, fname, L"rb");

		if (fp != NULL)
		{
			fseek(fp, 0, SEEK_END);
			len  = _ftelli64(fp);
			fseek(fp, 0, SEEK_SET);
			fclose(fp);
		}
		return len;
	}

	static int isDirExists(const wchar_t* s) 
	{
		_wfinddatai64_t c_file;
		intptr_t hFile;
		int result = 0;

		hFile = _wfindfirsti64(s, &c_file);
		if (c_file.attrib & _A_SUBDIR ) result = 1;
		_findclose(hFile);
		return result;
	}

	// 휴지통으로 지우기 / 지우기
	static HRESULT _removefile(const wchar_t *path, bool recyclebin)
	{
		if (path == NULL)  
			return E_POINTER;  

		// Unicode UTF-16 로 문자열 변경  
		CT2W wszFileToDelete( path );

		IFileOperation *pfo = NULL;  
		HRESULT hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));  

		if (SUCCEEDED(hr))  
		{  
			DWORD flags = FOF_NO_UI; //FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;
			if( recyclebin ) { flags |= FOF_ALLOWUNDO; }  // 휴지통에 넣기

			hr = pfo->SetOperationFlags(flags);  
			if (SUCCEEDED(hr))  
			{  
				IShellItem *psiFileToDelete = NULL;  
				hr = SHCreateItemFromParsingName(wszFileToDelete, NULL, IID_PPV_ARGS(&psiFileToDelete));  
				if (SUCCEEDED(hr))  
				{  
					hr = pfo->DeleteItem( psiFileToDelete, NULL );  
				}
				psiFileToDelete->Release();  
			}

			if (SUCCEEDED(hr))  
			{  
				hr = pfo->PerformOperations();  
			}  
		}  
		if(pfo)
			pfo->Release();  
  
		return hr; 
	}

	//////////////////////////////////////////////////////////////////////////

	static HRESULT FileOperate(IFileOperation** pp, bool recyclebin)
	{
		HRESULT hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(pp));  
		if FAILED(hr)
			return hr;
		DWORD flags = FOF_NO_UI; //FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;
		if( recyclebin ) { flags |= FOF_ALLOWUNDO; }  // 휴지통에 넣기

		(*pp)->SetOperationFlags(flags);
		//(*pp)->SetOwnerWindow(GetWindow());
		return S_OK;
	}

	static HRESULT RunFileOperate(IFileOperation** pp)
	{
		HRESULT	hr;
		hr = (*pp)->PerformOperations();  
		return hr;
	}

	static void	ReleaseFileOperate(IFileOperation** pp)
	{
		(*pp)->Release();
	}

	// path to shellitem
	static IShellItem *CreateShellItem(const wchar *path)
	{
		IShellItem *item = NULL;  
		// Unicode UTF-16 로 문자열 변경  
		CT2W str( path );
		SHCreateItemFromParsingName(str, NULL, IID_PPV_ARGS(&item));  
		return item;
	}

	static void	ReleaseShellItem(IShellItem *item)
	{
		item->Release();
	}

	//////////////////////////////////////////////////////////////////////////

	static HRESULT FileRename(IFileOperation* op, IShellItem* item, const wchar_t *name)
	{
		return op->RenameItem(item, name, NULL);
	}

	static HRESULT FileDelete(IFileOperation* op, IShellItem* item)
	{
		return op->DeleteItem(item, NULL);
	}

	static HRESULT FileCopy(IFileOperation* op, IShellItem* item, IShellItem* folder, const wchar_t *name)
	{
		return op->CopyItem(item, folder, name, NULL);
	}

	static HRESULT FileMove(IFileOperation* op, IShellItem* item, IShellItem* folder, const wchar_t *name)
	{
		return op->MoveItem(item, folder, name, NULL);
	}

	//////////////////////////////////////////////////////////////////////////
	static void changeAttiribute(const wchar *path, unsigned long att)
	{
		unsigned long attribute = att;
		SetFileAttributes(path, attribute);
	}

	// 소스 , 타겟폴더, 새이름
	static bool	copyfile(const wchar *path, const wchar *newdir, const wchar *newname )
	{
		IFileOperation *ptr = NULL;
		HRESULT hr = FileOperate(&ptr, false);
		if( hr != S_OK )
			return false;

		IShellItem *item = CreateShellItem(path);
		IShellItem *targetitem = CreateShellItem(newdir);

		hr = FileCopy(ptr, item, targetitem, newname);
		hr = RunFileOperate(&ptr);

		ReleaseShellItem(item);
		ReleaseShellItem(targetitem);
		ReleaseFileOperate(&ptr);

		return true;
	}

	// 소스 , 타겟폴더, 새이름
	static bool	movefile(const wchar *path, const wchar *newdir, const wchar *newname )
	{
		IFileOperation *ptr = NULL;
		HRESULT hr = FileOperate(&ptr, false);
		if( hr != S_OK )
			return false;

		IShellItem *item = CreateShellItem(path);
		IShellItem *targetitem = CreateShellItem(newdir);

		hr = FileMove(ptr, item, targetitem, newname);
		hr = RunFileOperate(&ptr);

		ReleaseShellItem(item);
		ReleaseShellItem(targetitem);
		ReleaseFileOperate(&ptr);

		return true;
	}

	static bool removedirectoty(const wchar *path, bool recyclebin)
	{
		if(recyclebin)
		{
			IFileOperation *ptr = NULL;
			HRESULT hr = FileOperate(&ptr, recyclebin);
			if( hr != S_OK )
				return false;

			IShellItem *item = CreateShellItem(path);
			//hr = FileRename(ptr, item, newpath);
			if( item == NULL ) return false;
			hr = FileDelete(ptr, item);

			hr = RunFileOperate(&ptr);

			ReleaseShellItem(item);
			ReleaseFileOperate(&ptr);
		}
		else
		{
			BOOL ret = RemoveDirectory(path);
			return ret ? true : false;
		}

		return true;
	}

	static bool	removefile( const wchar *path, bool recyclebin )
	{
		if(recyclebin)
		{
			IFileOperation *ptr = NULL;
			HRESULT hr = FileOperate(&ptr, recyclebin);
			if( hr != S_OK )
				return false;

			IShellItem *item = CreateShellItem(path);
			//hr = FileRename(ptr, item, newpath);
			if( item == NULL ) return false;
			hr = FileDelete(ptr, item);

			hr = RunFileOperate(&ptr);

			ReleaseShellItem(item);
			ReleaseFileOperate(&ptr);
		}
		else
		{
			unsigned long attribute = GetPathAttribute(path);
			if (attribute & ATTR_READONLY)
			{
				attribute = attribute ^ ATTR_READONLY;
				utility::changeAttiribute(path, attribute);
			}

			BOOL ret = DeleteFile(path);
			if (ret == 0)
			{
				DWORD errorcode = GetLastError();
				_TRACE(L"%d\n", errorcode);
			}

			return ret ? true : false;
			//int ret = _wremove(path);
			//return (ret == 0) ? false : true;

			//_removefile(path, false);
		}

		return true;
	}

	static bool	renamefile( const wchar *path, const wchar *newpath )
	{
		IFileOperation *ptr = NULL;
		HRESULT hr = FileOperate(&ptr, false);
		if( hr != S_OK )
			return false;

		IShellItem *item = CreateShellItem(path);
		hr = FileRename(ptr, item, newpath);

		hr = RunFileOperate(&ptr);

		ReleaseShellItem(item);
		ReleaseFileOperate(&ptr);

		return true;
	}

	// access의 mode	
	//		00 : Existence only
	//		02 : Write-only
	//		04 : Read-only
	//		06 : Read and write
	static bool	access(const wchar *path)
	{
		// Read and write로 검사
		int r = _waccess( path, 0 );
		return r == -1 ? false : true;
	}

	static bool	makedirectory(const wchar *path)
	{
		// 경로가 존재하는가 검사
		if( access(path) == true ) return false;

		int r = _wmkdir(path);
		return r == -1 ? false : true;
	}

	// 스트링의 일부변환
	static String replace(const String& source,const String& sfind,const String& replace)
	{
		String ret = source;
		int pos = source.find(sfind);
		ret.erase(pos, sfind.length());
		ret.insert(pos,replace);
		return ret;
	}


	//////////////////////////////////////////////////////////////////////////

	static String getStringSize(__int64 fsize, int pi, bool simple = false)
	{
		int mok = 0;
		double filesize = fsize ;

		String Space = simple ? L"B" : L"Byte";
		String returnStr = L"";

		while (filesize > 1024.0)
		{
			filesize /= 1024.0;
			mok++;
		}

		if (mok == 1)
			Space = simple ? L"K" : L"Kb";
		else if (mok == 2)
			Space = simple ? L"M" : L"Mb";
		else if (mok == 3)
			Space = simple ? L"G" : L"Gb";
		else if (mok == 4)
			Space = simple ? L"T" : L"Tb";

		if(mok!=0)
		{
			if (pi == 1)
				returnStr = unicode::format(L"%.1f%s", filesize, Space.c_str());
			else if (pi == 2)
				returnStr = unicode::format(L"%.2f%s", filesize, Space.c_str());
			else if (pi == 3)
				returnStr = unicode::format(L"%.3f%s", filesize, Space.c_str());
			else
				returnStr = unicode::format(L"%d%s", (int)filesize, Space.c_str());
		}
		else
		{
			returnStr = unicode::format(L"%d%s", (int)filesize, Space.c_str());
		}
		return returnStr;
	}

	static String getStringAttribute(unsigned long attr)
	{
		String ret;
		ret = (attr & ATTR_READONLY) ? L"R" : L"_";
		ret += (attr & ATTR_HIDDEN) ? L"H" : L"_";
		ret += (attr & ATTR_SYSTEM) ? L"S" : L"_";
		ret += (attr & ATTR_ARCHIVE) ? L"A" : L"_";
		return ret;
	}
	
	// 지정된 글자수로 줄임
	static String getShortStr(String str, int len)
	{
		String ret;
		if( (int)str.size() > len )
		{
			ret = String(str, 0, len);
			ret += L"...";
		}
		else
		{
			ret = str;
		}
		return ret;
	}

	static String getShortStrMid(String str, int len)
	{
		String ret;
		int i= 0;
		if( (int)str.size() > len )
		{
			while(1)
			{
				int half = str.size()/2;
				String t = String(str,0,half-i) + L" ... " + String(str,half+i, str.size()-half+i) ;
				if( (int)t.size() < len )
				{
					ret = t;
					break;
				}
				i++;
			}
		}
		else
		{
			ret = str;
		}
		return ret;
	}

	// TODO. 원하는 pixel 크리고 줄임
	static String getShortStrPixel(String str, int width)
	{
		String ret;
		if( (int)str.size() > width )
		{
			ret = String(str, 0, width);
			ret += L"...";
		}
		else
		{
			ret = str;
		}
		return ret;
	}

	static BOOL ShowContextMenu(PIDLIST_ABSOLUTE pidlAbsolute, HWND hwnd)
	{
		int                 nId;
		HRESULT             hr;
		POINT               pt;
		HMENU               hmenuPopup;
		IContextMenu        *pContextMenu;
		IShellFolder        *pShellFolder;
		PITEMID_CHILD       pidlChild;
		CMINVOKECOMMANDINFO ici;

// 		LPITEMIDLIST        pidlItem = NULL;
// 		SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlItem);
// 
// 		pidlAbsolute = pidlItem;
		SHBindToParent(pidlAbsolute, IID_PPV_ARGS(&pShellFolder), NULL);
		pidlChild = ILFindLastID(pidlAbsolute);

		hr = pShellFolder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidlChild, IID_IContextMenu, NULL, (void **)&pContextMenu);
		if (hr != S_OK) {
			pShellFolder->Release();
			return FALSE;
		}

		hmenuPopup = CreatePopupMenu();
		pContextMenu->QueryContextMenu(hmenuPopup, 0, 1, 0x7fff, CMF_NORMAL);

		GetCursorPos(&pt);
		nId = TrackPopupMenu(hmenuPopup, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
		if (nId == 0) {
			pContextMenu->Release();
			pShellFolder->Release();
			return FALSE;
		}

		ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
		ici.fMask = 0;
		ici.hwnd = hwnd;
		ici.lpVerb = (LPCSTR)MAKEINTRESOURCE(nId - 1);
		ici.lpParameters = NULL;
		ici.lpDirectory = NULL;
		ici.nShow = SW_SHOW;

		hr = pContextMenu->InvokeCommand(&ici);

		pContextMenu->Release();
		pShellFolder->Release();

		return hr == S_OK;
	}

	static bool openShellContextMenuForObject(String &path, int xPos, int yPos, HWND parentWindow)
	{
		PIDLIST_ABSOLUTE pidlAbsolute;
		ITEMIDLIST * id = 0;

		std::wstring windowsPath = path;
		std::replace(windowsPath.begin(), windowsPath.end(), '/', '\\');

		SFGAOF out;
		HRESULT result = SHParseDisplayName(windowsPath.c_str(), NULL, &id, SFGAO_FILESYSTEM, &out);

 		//HRESULT result = SHParseDisplayName(windowsPath.c_str(), 0, &id, 0, 0);
 		if (!SUCCEEDED(result) || !id)
 			return false;
 		pidlAbsolute = (PIDLIST_ABSOLUTE)id;
		ShowContextMenu(pidlAbsolute, parentWindow);


/*
		ITEMIDLIST * id = 0;
		std::wstring windowsPath = path;
		std::replace(windowsPath.begin(), windowsPath.end(), '/', '\\');

		HRESULT result = SHParseDisplayName(windowsPath.c_str(), 0, &id, 0, 0);
		if (!SUCCEEDED(result) || !id)
			return false;

		IShellFolder * ifolder = 0;
		LPCITEMIDLIST idChild = 0;
		result = SHBindToParent(id, IID_IShellFolder, (void**)&ifolder, &idChild);
		if (!SUCCEEDED(result) || !ifolder)
			return false;

		IContextMenu * imenu = 0;
		result = ifolder->GetUIObjectOf(parentWindow, 1, (const ITEMIDLIST **)&idChild, IID_IContextMenu, 0, (void**)&imenu);
		if (!SUCCEEDED(result) || !ifolder)
			return false;

		HMENU hMenu = CreatePopupMenu();
		if (!hMenu)
			return false;

		if (SUCCEEDED(imenu->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_EXPLORE)))
		{
			SetForegroundWindow(parentWindow);
			int iCmd = TrackPopupMenuEx(hMenu, TPM_RETURNCMD, xPos, yPos, parentWindow, NULL);
			if (iCmd > 0)
			{
				CMINVOKECOMMANDINFOEX info = { 0 };
				info.cbSize = sizeof(info);
				info.fMask = CMIC_MASK_UNICODE;
				info.hwnd = parentWindow;
				info.lpVerb  = MAKEINTRESOURCEA(iCmd - 1);
				info.lpVerbW = MAKEINTRESOURCEW(iCmd - 1);
				info.nShow = SW_SHOWNORMAL;
				imenu->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
			}
		}
		DestroyMenu(hMenu);
*/
		return true;
	}

	static unsigned TokenizeString(const String& i_source, const String& i_seperators, bool i_discard_empty_tokens, std::vector<String>& o_tokens)
	{
		unsigned prev_pos = 0;
		unsigned pos = 0;
		unsigned number_of_tokens = 0;
		o_tokens.clear();
		pos = i_source.find_first_of(i_seperators, pos);
		while (pos != String::npos)
		{
			String token = i_source.substr(prev_pos, pos - prev_pos);
			if (!i_discard_empty_tokens || token != L"")
			{
				o_tokens.push_back(i_source.substr(prev_pos, pos - prev_pos));
				number_of_tokens++;
			}

			pos++;
			prev_pos = pos;
			pos = i_source.find_first_of(i_seperators, pos);
		}

		if (prev_pos < i_source.length())
		{
			o_tokens.push_back(i_source.substr(prev_pos));
			number_of_tokens++;
		}

		return number_of_tokens;
	}

	// 시스템의 루트 패스 인가??
	static bool	IsRootPath(const wchar *dir)
	{
		return PathIsRoot(dir) ? true : false;
	}

	// 드라이브의 루트인가?
	static bool	IsRootDirectory(const wchar *dir)
	{
		if( dir[1] == ':' && unicode::strlen(dir) == 2 )
			return true;
		return false;
	}

	static String GetParentPath(const wchar *fullpath)
	{	
		String ret;
		int len = unicode::strlen(fullpath);
		while(len >=0 )
		{
			if( fullpath[len-1] == '\\' )
			{
				ret = String(fullpath, len-1);
				break;
			}
			len--;
		}

		return ret;
	}

	static String	GetDrive(const wchar *fullpath)
	{
	/*
		String drive = String(fullpath, 0, 1);
		transform(drive.begin(), drive.end(), drive.begin(), toupper);
		return drive;*/
		wchar t[_MAX_FNAME]; 
		_wsplitpath_s(fullpath, t, _MAX_FNAME, NULL, 0, NULL, 0, NULL, 0); 
		return String(t); 

	}

	// Path만 얻기
	static String	GetPath(const wchar *fullpath)
	{
		wchar t[_MAX_FNAME]; 
		_wsplitpath_s(fullpath, NULL, 0, t, _MAX_FNAME, NULL, 0, NULL, 0); 
		return String(t); 
	}

	// 이름만얻기
	static String	GetName(const wchar *fullpath)
	{
		wchar t[_MAX_FNAME]; 
		_wsplitpath_s(fullpath, NULL, 0, NULL, 0, t, _MAX_FNAME, NULL, 0); 
		return String(t); 
	}

	// 확장자만
	static String	GetExtention(const wchar *fullpath)
	{
		wchar t[_MAX_EXT]; 
		_wsplitpath_s(fullpath, NULL, 0, NULL, 0, NULL, 0, t, _MAX_EXT); 
		return String(t);
	}

	// 이름 + 확장자
	static String	GetFileName(const wchar *fullpath)
	{
		wchar n[_MAX_FNAME]; 
		wchar t[_MAX_EXT]; 
		_wsplitpath_s(fullpath, NULL, 0, NULL, 0, n, _MAX_FNAME, t, _MAX_EXT); 

		wchar r[1024];
		unicode::sprintf(r, 1024, L"%s%s",n,t);
		return String(r);
	}

	static unsigned int generateHash(const char *string, size_t len)
	{
		unsigned int hash = 0;
		for(size_t i = 0; i < len; ++i)
		{
			hash = 65599 * hash + string[i];
		}
		return hash ^ (hash >> 16);
	}

	static unsigned int stringHash(String str)
	{
		char buf[2048];
		size_t len = str.length()*sizeof(short);
		memcpy(buf, (void*)str.c_str(), len);

		return generateHash(buf, len);
	}

	static SYSTEMTIME getLastWriteTime(String filename)
	{
		HANDLE hFile;
		hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

		FILETIME ftCreate, ftAccess, ftWrite;
		SYSTEMTIME stUTC, stLocal;
		memset(&stLocal, 0, sizeof(stLocal));

		if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
		{
			CloseHandle(hFile);
			return stLocal;
		}

		FileTimeToSystemTime(&ftWrite, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

		CloseHandle(hFile);
		return stLocal;
	}

	static void getFileTime(String filename, FILETIME &ftCreate, FILETIME &ftAccess, FILETIME &ftWrite)
	{
		HANDLE hFile;
		hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
		{
			CloseHandle(hFile);
		}

		CloseHandle(hFile);
	}

	static void Log(String txt)
	{
		loglock.Enter();

		FILE *fp = NULL;
		_wfopen_s(&fp, L"log.txt", L"a+");
		if(fp != NULL)
		{
			fwprintf_s(fp, L"--> %s\n", txt.c_str());
			fclose(fp);
		}

		loglock.Leave();
	}

	static void CreateProcess(String exe)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		CreateProcess(NULL, (LPWSTR)exe.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}

	static void CreateProcess(String exe, String dir)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		CreateProcess(NULL, (LPWSTR)exe.c_str(), NULL, NULL, TRUE, 0, NULL, dir.c_str(), &si, &pi);
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}

	static float GetDriveSpace(String drive, String &totalspace, String &freespace) 
	{
		ULARGE_INTEGER avail, total, free;
		avail.QuadPart = 0L;
		total.QuadPart = 0L;
		free.QuadPart = 0L;
		int m_total, m_free;

		// ex : drive (c:\\)
		GetDiskFreeSpaceEx(drive.c_str(), &avail, &total, &free);

		totalspace = getStringSize(total.QuadPart, 1);
		freespace = getStringSize(free.QuadPart, 1);

		// 리턴은 Percent
		m_total = (int)(total.QuadPart>>30);
		m_free = (int)(free.QuadPart>>30);

		return (float)(m_total-m_free)/(float)m_total;
	}



	static time_t FILETIME_to_time_t(const FILETIME *lpFileTime) 
	{
		time_t result;
		SYSTEMTIME st;
		struct tm tmp;
		FileTimeToSystemTime(lpFileTime,&st);
		memset(&tmp,0,sizeof(struct tm));
		tmp.tm_mday = st.wDay;
		tmp.tm_mon  = st.wMonth - 1;
		tmp.tm_year = st.wYear - 1900;

		tmp.tm_sec  = st.wSecond;
		tmp.tm_min  = st.wMinute;
		tmp.tm_hour = st.wHour;

		result = mktime(&tmp);
		return result;
	} 

	static void TimetToFileTime( time_t t, LPFILETIME pft )
	{
		LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
		pft->dwLowDateTime = (DWORD) ll;
		pft->dwHighDateTime = ll >>32;
	}

	static void DrawProgressbar(Canvas *c, int x, int y, int width, int height, float progress, bool selected,int clipwidth, int clipheight)
	{
		// FF7833(max) --> 007833 (min)
		unsigned long barcolor = 0xffa00000;
		unsigned long baroutline = 0xffb0b0b0;

		//unsigned char ch = (unsigned char)(0xFF * (progress > 1.0f ? 1.0f : progress));
		//barcolor = 0xff000000 | 0x00004E4E | (ch << 16);

		unsigned char red_ch = (unsigned char)(0xFF * (progress > 1.0f ? 1.0f : progress));
		unsigned char green_ch = (unsigned char)(0xFF * (progress > 1.0f ? 1.0f : 1.0f - progress));

		barcolor = 0xff000000 | (unsigned int)(red_ch << 16) | (unsigned int)(green_ch << 8);

		unsigned long undarkcolor = selected ? barcolor & 0xFFA0A0A0 : barcolor;
		unsigned long sizedarkcolor = selected ? 0xffb0b0b0 & 0xFFA0A0A0 : 0xffb0b0b0;

		int _width = (int)(width * progress);
		c->DrawRectClip(x, y, _width, height, undarkcolor, false, clipwidth, clipheight);
		c->DrawRectClip(x, y, width, height, sizedarkcolor, true, clipwidth, clipheight);
	}
};


#endif