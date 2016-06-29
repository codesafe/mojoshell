
#include <windows.h>	

#include "../stringutil.h"

#include "Zipper.h"
#include "zip.h"
#include "iowin32.h"

#pragma  comment (lib, "zlibstat.lib")

const UINT BUFFERSIZE = 2048;

bool CZipper::GetLastModified(const wchar* szPath, SYSTEMTIME& sysTime)
{
	ZeroMemory(&sysTime, sizeof(SYSTEMTIME));

	DWORD dwAttr = ::GetFileAttributes(szPath);

	// files only
	if (dwAttr == 0xFFFFFFFF)
	 return false;

	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((LPTSTR)szPath, &findFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	 return false;

	FindClose(hFind);

	FILETIME ft = findFileData.ftLastWriteTime;

	FileTimeToLocalFileTime(&findFileData.ftLastWriteTime, &ft);
	FileTimeToSystemTime(&ft, &sysTime);
	return true;
}

CZipper::CZipper(wchar *szFilePath, wchar *szRootFolder, bool bAppend, int method) : m_uzFile(0)
{
	compress_method = method;

	switch(compress_method)
	{
		case COMPRESS_METHOD_NONE :
			compress_method = Z_NO_COMPRESSION;
			break;
		case COMPRESS_METHOD_FAST :
			compress_method = Z_BEST_SPEED;
			break;
		case COMPRESS_METHOD_NORMAL :
			compress_method = Z_DEFAULT_COMPRESSION;
			break;
		case COMPRESS_METHOD_SIZE :
			compress_method = Z_BEST_COMPRESSION;
			break;
		
	}

	CloseZip();

	if (szFilePath)
		OpenZip(szFilePath, szRootFolder, bAppend);
}

CZipper::~CZipper()
{
	CloseZip();
}

bool CZipper::CloseZip()
{
	int nRet = m_uzFile ? zipClose(m_uzFile, NULL) : ZIP_OK;

	m_uzFile = NULL;
	m_szRootFolder[0] = 0;
	ZeroMemory(&m_info, sizeof(m_info));

	return (nRet == ZIP_OK);
}

void CZipper::GetFileInfo(Z_FileInfo& info)
{
	info = m_info;
}

// simple interface
bool CZipper::ZipFile(wchar *szFilePath)
{
	// make zip path
	wchar szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szName[_MAX_FNAME];
	_wsplitpath(szFilePath, szDrive, szFolder, szName, NULL);

	wchar szZipPath[MAX_PATH];
	_wmakepath(szZipPath, szDrive, szFolder, szName, L"zip");

	CZipper zip;

	if (zip.OpenZip(szZipPath, false))
		return zip.AddFileToZip(szFilePath, false);

	return FALSE;
}
	
bool CZipper::ZipFolder(wchar *szFilePath, bool bIgnoreFilePath)
{
	// make zip path
	wchar szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szName[_MAX_FNAME];
	_wsplitpath(szFilePath, szDrive, szFolder, szName, NULL);

	wchar szZipPath[MAX_PATH];
	_wmakepath(szZipPath, szDrive, szFolder, szName, L"zip");

	CZipper zip;

	if (zip.OpenZip(szZipPath, FALSE))
		return zip.AddFolderToZip(szFilePath, bIgnoreFilePath);

	return FALSE;
}
	
// works with prior opened zip
bool CZipper::AddFileToZip(wchar *szFilePath, bool bIgnoreFilePath)
{
	if (!m_uzFile)
		return FALSE;

	// we don't allow paths beginning with '..\' because this would be outside
	// the root folder
	if (!bIgnoreFilePath && wcsstr(szFilePath, L"..\\") == szFilePath)
		return false;

	bool bFullPath = (wcschr(szFilePath, ':') != NULL);

	// if the file is relative then we need to append the root before opening
	wchar szFullFilePath[MAX_PATH];
	
	lstrcpy(szFullFilePath, szFilePath);
	PrepareSourcePath(szFullFilePath);

	// if the file is a fullpath then remove the root path bit
	wchar szFileName[MAX_PATH] = L"";

	if (bIgnoreFilePath)
	{
		wchar szName[_MAX_FNAME], szExt[_MAX_EXT];
		_wsplitpath(szFilePath, NULL, NULL, szName, szExt);

		_wmakepath(szFileName, NULL, NULL, szName, szExt);
	}
	else if (bFullPath)
	{
		// check the root can be found
		if (0 != _wcsnicmp(szFilePath, m_szRootFolder, lstrlen(m_szRootFolder)))
			return false;

		// else
		lstrcpy(szFileName, szFilePath + lstrlen(m_szRootFolder));
	}
	else // relative path
	{
		// if the path begins with '.\' then remove it
		if (wcsstr(szFilePath, L".\\") == szFilePath)
			lstrcpy(szFileName, szFilePath + 2);
		else
			lstrcpy(szFileName, szFilePath);
	}

	// save file attributes
	zip_fileinfo zfi;

	zfi.internal_fa = 0;
	zfi.external_fa = ::GetFileAttributes(szFilePath);
	
	// save file time
	SYSTEMTIME st;

	GetLastModified(szFullFilePath, st);

	zfi.dosDate = 0;
	zfi.tmz_date.tm_year = st.wYear;
	zfi.tmz_date.tm_mon = st.wMonth - 1;
	zfi.tmz_date.tm_mday = st.wDay;
	zfi.tmz_date.tm_hour = st.wHour;
	zfi.tmz_date.tm_min = st.wMinute;
	zfi.tmz_date.tm_sec = st.wSecond;
	
	// load input file
	HANDLE hInputFile = ::CreateFile(szFullFilePath, 
									GENERIC_READ,
									0,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_READONLY,
									NULL);

	if (hInputFile == INVALID_HANDLE_VALUE)
		return FALSE;

	char c_szFileName[MAX_PATH];
	unicode::convstr_s(c_szFileName, MAX_PATH, szFileName);

	int nRet = zipOpenNewFileInZip(m_uzFile, 
									c_szFileName,
									&zfi, 
									NULL, 
									0,
									NULL,
									0, 
									NULL,
									Z_DEFLATED,
									Z_DEFAULT_COMPRESSION,
									NULL, 0);

	if (nRet == ZIP_OK)
	{
		m_info.nFileCount++;

		// read the file and output to zip
		wchar pBuffer[BUFFERSIZE];
		DWORD dwBytesRead = 0, dwFileSize = 0;

		while (nRet == ZIP_OK && ::ReadFile(hInputFile, pBuffer, BUFFERSIZE, &dwBytesRead, NULL))
		{
			dwFileSize += dwBytesRead;

			if (dwBytesRead)
				nRet = zipWriteInFileInZip(m_uzFile, pBuffer, dwBytesRead);
			else
				break;
		}

		m_info.dwUncompressedSize += dwFileSize;
	}

	zipCloseFileInZip(m_uzFile);
	::CloseHandle(hInputFile);

	return (nRet == ZIP_OK);
}

bool CZipper::AddFileToZip(wchar *szFilePath, wchar *szRelFolderPath, unsigned long crc)
{
	if (!m_uzFile)
		return FALSE;

	// szRelFolderPath cannot contain drive info
	if (szRelFolderPath && wcschr(szRelFolderPath, ':'))
		return FALSE;

	//////////////////////////////////////////////////////////////////////////

/*
	char inbuf[1024];
	int readSize = 0;
	__int64 len = 0;
	uLong crc = 0;
	FILE *fp;
	errno_t err = _wfopen_s(&fp, szFilePath, L"rb");
	fseek(fp, 0, SEEK_END);
	len  = _ftelli64(fp);
	fseek(fp, 0, SEEK_SET);
	while(1)
	{
		readSize = fread(inbuf, 1, 1024,fp);
		if(readSize <= 0)
			break;
		crc = crc32(crc,(const Bytef*)inbuf,readSize);
	}
	fclose(fp);
*/

	//////////////////////////////////////////////////////////////////////////

	// if the file is relative then we need to append the root before opening
	wchar szFullFilePath[MAX_PATH];
	
	lstrcpy(szFullFilePath, szFilePath);
	PrepareSourcePath(szFullFilePath);

	// save file attributes and time
	zip_fileinfo zfi;

	zfi.internal_fa = 0;
	zfi.external_fa = ::GetFileAttributes(szFilePath);
	
	// save file time
	SYSTEMTIME st;

	GetLastModified(szFullFilePath, st);

	zfi.dosDate = 0;
	zfi.tmz_date.tm_year = st.wYear;
	zfi.tmz_date.tm_mon = st.wMonth - 1;
	zfi.tmz_date.tm_mday = st.wDay;
	zfi.tmz_date.tm_hour = st.wHour;
	zfi.tmz_date.tm_min = st.wMinute;
	zfi.tmz_date.tm_sec = st.wSecond;

	// load input file
	HANDLE hInputFile = ::CreateFile(szFullFilePath, 
									GENERIC_READ,
									0,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_READONLY,
									NULL);

	if (hInputFile == INVALID_HANDLE_VALUE)
		return FALSE;

	// strip drive info off filepath
	wchar szName[_MAX_FNAME], szExt[_MAX_EXT];
	_wsplitpath(szFilePath, NULL, NULL, szName, szExt);

	// prepend new folder path 
	wchar szFileName[MAX_PATH];
	_wmakepath(szFileName, NULL, szRelFolderPath, szName, szExt);


	char c_szFileName[MAX_PATH];
	unicode::convstr_s(c_szFileName, MAX_PATH, szFileName + (szFileName[0] == '\\' ? 1 : 0));

	// open the file in the zip making sure we remove any leading '\'
	int nRet = zipOpenNewFileInZip(
									m_uzFile, 
									c_szFileName,
									&zfi, 
									NULL, 
									0,
									NULL,
									0, 
									NULL,
									Z_DEFLATED,
									Z_DEFAULT_COMPRESSION, 
									"test", crc);


	if (nRet == ZIP_OK)
	{
		m_info.nFileCount++;

		// read the file and output to zip
		char pBuffer[BUFFERSIZE];
		DWORD dwBytesRead = 0, dwFileSize = 0;

		while (nRet == ZIP_OK && ::ReadFile(hInputFile, pBuffer, BUFFERSIZE, &dwBytesRead, NULL))
		{
			dwFileSize += dwBytesRead;

			if (dwBytesRead)
				nRet = zipWriteInFileInZip(m_uzFile, pBuffer, dwBytesRead);
			else
				break;
		}

		m_info.dwUncompressedSize += dwFileSize;
	}

	zipCloseFileInZip(m_uzFile);
	::CloseHandle(hInputFile);

	return (nRet == ZIP_OK);
}

bool CZipper::AddFolderToZip(wchar *szFolderPath, bool bIgnoreFilePath)
{
/*
	if (!m_uzFile)
		return FALSE;

	m_info.nFolderCount++;

	// if the path is relative then we need to append the root before opening
	char szFullPath[MAX_PATH];
	
	lstrcpy(szFullPath, szFolderPath);
	PrepareSourcePath(szFullPath);

	// always add folder first
	// save file attributes
	zip_fileinfo zfi;
	
	zfi.internal_fa = 0;
	zfi.external_fa = ::GetFileAttributes(szFullPath);
	
	SYSTEMTIME st;
	
	GetLastModified(szFullPath, st, TRUE);
	
	zfi.dosDate = 0;
	zfi.tmz_date.tm_year = st.wYear;
	zfi.tmz_date.tm_mon = st.wMonth - 1;
	zfi.tmz_date.tm_mday = st.wDay;
	zfi.tmz_date.tm_hour = st.wHour;
	zfi.tmz_date.tm_min = st.wMinute;
	zfi.tmz_date.tm_sec = st.wSecond;
	
	// if the folder is a fullpath then remove the root path bit
	char szFolderName[MAX_PATH] = "";
	
	if (bIgnoreFilePath)
	{
		_splitpath(szFullPath, NULL, NULL, szFolderName, NULL);
	}
	else
	{
		// check the root can be found
		if (0 != _strnicmp(szFullPath, m_szRootFolder, lstrlen(m_szRootFolder)))
			return false;
		
		// else
		lstrcpy(szFolderName, szFullPath + lstrlen(m_szRootFolder));
	}
	
	// folders are denoted by a trailing '\\'
	lstrcat(szFolderName, "\\");
	
	// open the file in the zip making sure we remove any leading '\'
	int nRet = zipOpenNewFileInZip(m_uzFile, 
		szFolderName,
		&zfi, 
		NULL, 
		0,
		NULL,
		0, 
		NULL,
		Z_DEFLATED,
		Z_DEFAULT_COMPRESSION);
	
	zipCloseFileInZip(m_uzFile);

	// build searchspec
	char szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szName[_MAX_FNAME];
	_splitpath(szFullPath, szDrive, szFolder, szName, NULL);
	lstrcat(szFolder, szName);

	char szSearchSpec[MAX_PATH];
	_makepath(szSearchSpec, szDrive, szFolder, "*", "*");

	WIN32_FIND_DATA finfo;
	HANDLE hSearch = FindFirstFile(szSearchSpec, &finfo);

	if (hSearch != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			if (finfo.cFileName[0] != '.') 
			{
				char szItem[MAX_PATH];
				_makepath(szItem, szDrive, szFolder, finfo.cFileName, NULL);
				
				if (finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					AddFolderToZip(szItem, bIgnoreFilePath);
				}
				else 
					AddFileToZip(szItem, bIgnoreFilePath);
			}
		} 
		while (FindNextFile(hSearch, &finfo));
		
		FindClose(hSearch);
	}
*/
	return TRUE;
}

// extended interface
bool CZipper::OpenZip(wchar *szFilePath, wchar *szRootFolder, bool bAppend)
{
	CloseZip();

	if (!szFilePath || !lstrlen(szFilePath))
		return false;

	// convert szFilePath to fully qualified path 
	wchar szFullPath[MAX_PATH];

	if (!GetFullPathName(szFilePath, MAX_PATH, szFullPath, NULL))
		return false;

	// zipOpen will fail if bAppend is TRUE and zip does not exist
	if (bAppend && ::GetFileAttributes(szFullPath) == 0xffffffff)
		bAppend = false;

	char c_szFullPath[MAX_PATH];
	unicode::convstr_s(c_szFullPath, MAX_PATH, szFullPath);

	m_uzFile = zipOpen(c_szFullPath, bAppend ? 1 : 0);

	if (m_uzFile)
	{
		if (!szRootFolder)
		{
			wchar szDrive[_MAX_DRIVE], szFolder[MAX_PATH];
			_wsplitpath(szFullPath, szDrive, szFolder, NULL, NULL);

			_wmakepath(m_szRootFolder, szDrive, szFolder, NULL, NULL);
		}
		else if (lstrlen(szRootFolder))
		{
			_wmakepath(m_szRootFolder, NULL, szRootFolder, L"", NULL);
		}
	}

	return (m_uzFile != NULL);
}

void CZipper::PrepareSourcePath(wchar *szPath)
{
	bool bFullPath = (wcschr(szPath, ':') != NULL);

	// if the file is relative then we need to append the root before opening
	if (!bFullPath)
	{
		wchar szTemp[MAX_PATH];
		lstrcpy(szTemp, szPath);

		_wmakepath(szPath, NULL, m_szRootFolder, szTemp, NULL);
	}
}

//////////////////////////////////////////////////////////////////////////


bool CZipper::OpenFileToZip(wchar *szFilePath, wchar *szRelFolderPath, wchar *passwd, unsigned long CRC)
{
	if (!m_uzFile)
		return false;

	if (szRelFolderPath && wcschr(szRelFolderPath, ':'))
		return false;

	wchar szFullFilePath[MAX_PATH];
	lstrcpy(szFullFilePath, szFilePath);
	PrepareSourcePath(szFullFilePath);

	// save file attributes and time
	zip_fileinfo zfi;
	zfi.internal_fa = 0;
	zfi.external_fa = ::GetFileAttributes(szFilePath);

	SYSTEMTIME st;
	GetLastModified(szFullFilePath, st);
	zfi.dosDate = 0;
	zfi.tmz_date.tm_year = st.wYear;
	zfi.tmz_date.tm_mon = st.wMonth - 1;
	zfi.tmz_date.tm_mday = st.wDay;
	zfi.tmz_date.tm_hour = st.wHour;
	zfi.tmz_date.tm_min = st.wMinute;
	zfi.tmz_date.tm_sec = st.wSecond;

	wchar szName[_MAX_FNAME], szExt[_MAX_EXT];
	_wsplitpath(szFilePath, NULL, NULL, szName, szExt);

	wchar szFileName[MAX_PATH];
	_wmakepath(szFileName, NULL, szRelFolderPath, szName, szExt);

	char c_szFileName[MAX_PATH];
	char c_password[MAX_PATH];

	unicode::convstr_s(c_szFileName, MAX_PATH, szFileName + (szFileName[0] == '\\' ? 1 : 0));
	if( passwd != NULL)
		unicode::convstr_s(c_password, MAX_PATH, passwd);

	int nRet = zipOpenNewFileInZip(
		m_uzFile, 
		c_szFileName,
		&zfi, 
		NULL, 
		0,
		NULL,
		0, 
		NULL,
		Z_DEFLATED,
		//Z_DEFAULT_COMPRESSION, 
		compress_method,
		(passwd == NULL ? NULL : c_password), 
		CRC);

	m_info.nFileCount++;
	return (nRet == ZIP_OK);
}

bool CZipper::AddDataToZip(char *buffer, int size)
{
	int ret = zipWriteInFileInZip(m_uzFile, buffer, size);
	if(ret != ZIP_OK)
	{
		CloseFileToZip();
	}
	else
	{
		m_info.dwUncompressedSize += size;
	}

	return (ret == ZIP_OK);
}

void CZipper::CloseFileToZip()
{
	zipCloseFileInZip(m_uzFile);
}
