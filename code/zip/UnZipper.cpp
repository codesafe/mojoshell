
#include "Unzipper.h"
#include "unzip.h"
#include "iowin32.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const UINT BUFFERSIZE = 2048;

UnZipper::UnZipper(const char*  szFileName) : m_uzFile(0)
{
    m_szOutputFolder[0] = 0;

    if (szFileName)
    {
        m_uzFile = unzOpen(szFileName);

        if (m_uzFile)
        {
            char szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szFName[_MAX_FNAME];
            _splitpath(szFileName, szDrive, szFolder, szFName, NULL);
            swprintf(m_szOutputFolder, _countof(m_szOutputFolder), L"%s%s%s", szDrive, szFolder, szFName);
        }
    }

	totalzipfilenum = 0;
	currentzipfilenum = 0;
}

UnZipper::~UnZipper()
{
	CloseZip();
}

bool UnZipper::CloseZip()
{
    unzCloseCurrentFile(m_uzFile);

    int nRet = unzClose(m_uzFile);
    m_uzFile = NULL;

    return (nRet == UNZ_OK);
}


// static version
bool UnZipper::Unzip(UnCompressProgress *dialog, wchar*  szFileName, wchar*  szFolder, bool bIgnoreFilePath, const wchar *passwd)
{
	UnZipper unz;

	if (!unz.OpenZip(szFileName))
		return FALSE;

	if(passwd)
	{
		char password[128];
		unicode::convstr_s(password, 128, passwd);
		return unz.UnzipTo(dialog, szFolder, bIgnoreFilePath, password);
	}
	else
		return unz.UnzipTo(dialog, szFolder, bIgnoreFilePath, NULL);
}

int UnZipper::IsEncryptZip(wchar* filename)
{
	UnZipper unz;
	if (!unz.OpenZip(filename))
		return CHECK_ZIP_ERROR;

	return unz.CheckEncryptedZip();
}

int UnZipper::CheckEncryptedZip()
{
	if (!m_uzFile) 
		return CHECK_ZIP_ERROR;

	if (!GotoFirstFile())
		return CHECK_ZIP_ERROR;

	if (unzOpenCurrentFile(m_uzFile) != UNZ_OK)
		return CHECK_ZIP_ERROR;

	char pBuffer[BUFFERSIZE];
	int ret = unzReadCurrentFile(m_uzFile, pBuffer, BUFFERSIZE);

	if( ret == -3 )
	{
		// password
		return CHECK_ZIP_ENCRYPT;
		
	}
	return CHECK_ZIP_OK;
}


/*
// simple interface
bool UnZipper::Unzip(bool bIgnoreFilePath)
{
    if (!m_uzFile)
            return FALSE;

    return UnzipTo(m_szOutputFolder, bIgnoreFilePath);
}
*/

bool UnZipper::UnzipTo(UnCompressProgress *dialog, wchar*  szFolder, bool bIgnoreFilePath, char *password)
{
    if (!m_uzFile)
		return false;

    if (!szFolder || !CreateFolder(szFolder))
		return false;

	totalzipfilenum = GetFileCount();
    if (totalzipfilenum == 0)
		return false;

    if (!GotoFirstFile())
		return false;

    do
    {
		if(password)
		{
			if (unzOpenCurrentFilePassword(m_uzFile, password) != UNZ_OK)
				return false;
		}
		else
		{
			if (unzOpenCurrentFile(m_uzFile) != UNZ_OK)
				return false;
		}

        if (!UnzipFile(dialog, szFolder, bIgnoreFilePath))
			return false;
    }
    while (GotoNextFile());
    
    return true;
}


// extended interface
bool UnZipper::OpenZip(wchar*  szFileName)
{
    CloseZip();

    if (szFileName)
    {
		char _path[MAX_PATH];
		unicode::convstr_s(_path, MAX_PATH, szFileName);
        m_uzFile = unzOpen(_path);

        if (m_uzFile)
        {
            wchar szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szFName[_MAX_FNAME];
            _wsplitpath(szFileName, szDrive, szFolder, szFName, NULL);

            swprintf(m_szOutputFolder, _countof(m_szOutputFolder),L"%s%s%s", szDrive, szFolder, szFName);
        }
    }

    return (m_uzFile != NULL);
}

bool UnZipper::SetOutputFolder(wchar*  szFolder)
{
    DWORD dwAttrib = GetFileAttributes(szFolder);

    if (dwAttrib != 0xffffffff && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
            return FALSE;

    lstrcpy(m_szOutputFolder, szFolder);

    return CreateFolder(szFolder);
}

int UnZipper::GetFileCount()
{
    if (!m_uzFile)
        return 0;

    unz_global_info info;
    if (unzGetGlobalInfo(m_uzFile, &info) == UNZ_OK)
    {
		return (int)info.number_entry;
    }

    return 0;
}

bool UnZipper::GetFileInfo(int nFile, UZ_FileInfo& info)
{
    if (!m_uzFile)
		return true;

    if (!GotoFile(nFile))
		return true;

    return GetFileInfo(info);
}

/*
bool UnZipper::UnzipFile(int nFile, wchar*  szFolder, bool bIgnoreFilePath)
{
    if (!m_uzFile)
            return FALSE;

    if (!szFolder)
            szFolder = m_szOutputFolder;

    if (!GotoFile(nFile))
            return FALSE;

    return UnzipFile(szFolder, bIgnoreFilePath);
}
*/

bool UnZipper::GotoFirstFile(wchar*  szExt)
{
    if (!m_uzFile)
		return false;

    if (!szExt || !lstrlen(szExt))
		return (unzGoToFirstFile(m_uzFile) == UNZ_OK);

    // else
    if (unzGoToFirstFile(m_uzFile) == UNZ_OK)
    {
        UZ_FileInfo info;
        if (!GetFileInfo(info))
			return false;

        // test extension
        wchar szFExt[_MAX_EXT];
		wchar _path[_MAX_EXT];
		unicode::convstr_s(_path, _countof(_path), info.szFileName);
        _wsplitpath(_path, NULL, NULL, NULL, szFExt);

        if (szFExt)
        {
			if (lstrcmpi(szExt, szFExt + 1) == 0)
				return true;
	    }
        return GotoNextFile(szExt);
    }

    return false;
}


bool UnZipper::GotoNextFile(wchar*  szExt)
{
    if (!m_uzFile)
		return FALSE;

    if (!szExt || !lstrlen(szExt))
		return (unzGoToNextFile(m_uzFile) == UNZ_OK);

    // else
    UZ_FileInfo info;

    while (unzGoToNextFile(m_uzFile) == UNZ_OK)
    {
        if (!GetFileInfo(info))
                return FALSE;

        // test extension
        wchar szFExt[_MAX_EXT];
		wchar _path[_MAX_EXT];
		unicode::convstr_s(_path, _countof(_path), info.szFileName);
        _wsplitpath(_path, NULL, NULL, NULL, szFExt);

        if (szFExt)
        {
            if (lstrcmpi(szExt, szFExt + 1) == 0)
				return TRUE;
        }
    }
    return FALSE;
}


bool UnZipper::GetFileInfo(UZ_FileInfo& info)
{
    if (!m_uzFile)
		return false;

    unz_file_info uzfi;

    ZeroMemory(&info, sizeof(info));
    ZeroMemory(&uzfi, sizeof(uzfi));

    if (UNZ_OK != unzGetCurrentFileInfo(m_uzFile, &uzfi, info.szFileName, MAX_PATH, NULL, 0, info.szComment, MAX_COMMENT))
		return false;

    // copy across
    info.dwVersion = uzfi.version;  
    info.dwVersionNeeded = uzfi.version_needed;
    info.dwFlags = uzfi.flag;       
    info.dwCompressionMethod = uzfi.compression_method; 
    info.dwDosDate = uzfi.dosDate;  
    info.dwCRC = uzfi.crc;   
    info.dwCompressedSize = uzfi.compressed_size; 
    info.dwUncompressedSize = uzfi.uncompressed_size;
    info.dwInternalAttrib = uzfi.internal_fa; 
    info.dwExternalAttrib = uzfi.external_fa; 

    // replace filename forward slashes with backslashes
    int nLen = strlen(info.szFileName);

    while (nLen--)
    {
        if (info.szFileName[nLen] == '/')
			info.szFileName[nLen] = '\\';
    }

    // is it a folder?
    info.bFolder = (info.szFileName[strlen(info.szFileName) - 1] == '\\');

    return true;
}

bool UnZipper::UnzipFile(UnCompressProgress *dialog, wchar*  szFolder, bool bIgnoreFilePath)
{
    if (!m_uzFile)
		return false;

    if (!szFolder)
		szFolder = m_szOutputFolder;

	currentzipfilenum++;

    if (!CreateFolder(szFolder))
        return false;

    UZ_FileInfo info;
    GetFileInfo(info);

    // if the item is a folder then simply return 'TRUE'
    if (info.szFileName[strlen(info.szFileName) - 1] == '\\')
        return true;

    // build the output filename
    wchar szFilePath[MAX_PATH];
    lstrcpy(szFilePath, szFolder);

    // append backslash
    if (szFilePath[lstrlen(szFilePath) - 1] != '\\')
            lstrcat(szFilePath, L"\\");

    if (bIgnoreFilePath)
    {
        char szFName[_MAX_FNAME], szExt[_MAX_EXT];
        _splitpath(info.szFileName, NULL, NULL, szFName, szExt);
        sprintf(info.szFileName, "%s%s", szFName, szExt);
    }

	wchar _path[MAX_PATH];
	unicode::convstr_s(_path, _countof(_path), info.szFileName);
    lstrcat(szFilePath, _path);

    // open the input and output files
    if (!CreateFilePath(szFilePath))
		return false;

    HANDLE hOutputFile = ::CreateFile(szFilePath, 
                                    GENERIC_WRITE,
                                    0,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

    if (INVALID_HANDLE_VALUE == hOutputFile)
		return false;

//     if (unzOpenCurrentFile(m_uzFile) != UNZ_OK)
// 		return false;

    // read the file and output
    int nRet = UNZ_OK;
    char pBuffer[BUFFERSIZE];

	wchar _filename[MAX_PATH];
	unicode::convstr_s(_filename, _countof(_filename), info.szFileName);
 	dialog->Update(L"currentfile", _filename);
	dialog->Update(L"filecount", unicode::format(L"(%d/%d)", currentzipfilenum, totalzipfilenum));

	int totalprogress = (int)((float)currentzipfilenum / (float)totalzipfilenum * 100.f);
 	dialog->Update(L"totalprogress", unicode::format(L"%d", totalprogress));

	int totaluncompresssize = 0;
    do
    {
        nRet = unzReadCurrentFile(m_uzFile, pBuffer, BUFFERSIZE);
		totaluncompresssize += nRet;
        if (nRet > 0)
        {
            // output
            DWORD dwBytesWritten = 0;
            if (!::WriteFile(hOutputFile, pBuffer, nRet, &dwBytesWritten, NULL) || dwBytesWritten != (DWORD)nRet)
            {
                nRet = UNZ_ERRNO;
                break;
            }
        }

		dialog->Update(L"progresssize", unicode::format(L"(%d/%d)", totaluncompresssize,info.dwUncompressedSize));
		int progresssize = (int)((float)totaluncompresssize / (float)info.dwUncompressedSize * 100.f);
		dialog->Update(L"zipprogress", unicode::format(L"%d", progresssize));

    }
    while (nRet > 0);

    CloseHandle(hOutputFile);
    unzCloseCurrentFile(m_uzFile);

    if (nRet == UNZ_OK)
		SetFileModTime(szFilePath, info.dwDosDate);

    return (nRet == UNZ_OK);
}

bool UnZipper::GotoFile(int nFile)
{
    if (!m_uzFile)
		return FALSE;

    if (nFile < 0 || nFile >= GetFileCount())
		return FALSE;

    GotoFirstFile();

    while (nFile--)
    {
        if (!GotoNextFile())
			return FALSE;
    }

    return TRUE;
}

/*
bool CUnzipper::GotoFile(char*  szFileName, bool bIgnoreFilePath)
{
    if (!m_uzFile)
            return FALSE;

    // try the simple approach
    if (unzLocateFile(m_uzFile, szFileName, 2) == UNZ_OK)
            return TRUE;

    else if (bIgnoreFilePath)
    { 
        // brute force way
        if (unzGoToFirstFile(m_uzFile) != UNZ_OK)
                return FALSE;

        UZ_FileInfo info;
        do
        {
            if (!GetFileInfo(info))
                    return FALSE;

            // test name
            char szFName[_MAX_FNAME], szName[_MAX_FNAME], szExt[_MAX_EXT];
            _splitpath(info.szFileName, NULL, NULL, szName, szExt);

            sprintf(szFName, "%s%s", szName, szExt);

            if (lstrcmpi(szFileName, szFName) == 0)
                    return TRUE;
        }
        while (unzGoToNextFile(m_uzFile) == UNZ_OK);
    }

    // else
    return FALSE;
}
*/

bool UnZipper::CreateFolder(wchar*  szFolder)
{
    if (!szFolder || !lstrlen(szFolder))
		return FALSE;

    DWORD dwAttrib = GetFileAttributes(szFolder);

    // already exists ?
    if (dwAttrib != 0xffffffff)
		return ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);

    // recursively create from the top down
    wchar* szPath = _wcsdup(szFolder);
    wchar* p = wcsrchr(szPath, '\\');

    if (p) 
    {
        // The parent is a dir, not a drive
        *p = '\0';
                
        // if can't create parent
        if (!CreateFolder(szPath))
        {
            free(szPath);
            return FALSE;
        }
        free(szPath);

        if (!::CreateDirectory(szFolder, NULL)) 
			return FALSE;
    }
    
    return TRUE;
}

bool UnZipper::CreateFilePath(wchar*  szFilePath)
{
    wchar* szPath = _wcsdup(szFilePath);
    wchar* p = wcsrchr(szPath,'\\');
    bool bRes = FALSE;

    if (p)
    {
        *p = '\0';
        bRes = CreateFolder(szPath);
    }

    free(szPath);
    return bRes;
}


bool UnZipper::SetFileModTime(wchar*  szFilePath, DWORD dwDosDate)
{
    HANDLE hFile = CreateFile(szFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (!hFile)
		return FALSE;
    
    FILETIME ftm, ftLocal, ftCreate, ftLastAcc, ftLastWrite;
    bool bRes = (GetFileTime(hFile, &ftCreate, &ftLastAcc, &ftLastWrite) == TRUE);
    if (bRes)
		bRes = (TRUE == DosDateTimeToFileTime((WORD)(dwDosDate >> 16), (WORD)dwDosDate, &ftLocal));

    if (bRes)
		bRes = (TRUE == LocalFileTimeToFileTime(&ftLocal, &ftm));

    if (bRes)
		bRes = (TRUE == SetFileTime(hFile, &ftm, &ftLastAcc, &ftm));

    CloseHandle(hFile);
    return bRes;
}

//////////////////////////////////////////////////////////////////////////

#if 0
DeCompress::DeCompress()
{

}

DeCompress::~DeCompress()
{

}

bool	DeCompress::openZip(String filename)
{
	char _path[MAX_PATH];
	unicode::convstr_s(_path, MAX_PATH, filename.c_str());
	handle = unzOpen(_path);

	return (!handle == NULL);
}

bool DeCompress::closeZip()
{
	unzCloseCurrentFile(handle);
	int ret = unzClose(handle);
	handle = NULL;
	return (ret == UNZ_OK);
}


// zip안의 파일에 대한 정보
bool	DeCompress::getFileInfo(zipfileInfo& info)
{
	unz_file_info uzfi;
	ZeroMemory(&info, sizeof(info));
	ZeroMemory(&uzfi, sizeof(uzfi));

	if (UNZ_OK != unzGetCurrentFileInfo(handle, &uzfi, info.filename, MAX_PATH, NULL, 0, info.comment, MAX_COMMENT))
		return false;

	info.dwVersion = uzfi.version;  
	info.dwVersionNeeded = uzfi.version_needed;
	info.dwFlags = uzfi.flag;       
	info.dwCompressionMethod = uzfi.compression_method; 
	info.dwDosDate = uzfi.dosDate;  
	info.dwCRC = uzfi.crc;   
	info.dwCompressedSize = uzfi.compressed_size; 
	info.dwUncompressedSize = uzfi.uncompressed_size;
	info.dwInternalAttrib = uzfi.internal_fa; 
	info.dwExternalAttrib = uzfi.external_fa; 

	int len = strlen(info.filename);
	while (len--)
	{
		if (info.filename[len] == '/')
			info.filename[len] = '\\';
	}

	info.isfolder = (info.filename[strlen(info.filename) - 1] == '\\');
	return true;
}


// zip안의 파일 갯수 얻기 (파일/폴더)
int		DeCompress::getTotalFileCount()
{
	if (!handle) return 0;

	unz_global_info info;
	if (unzGetGlobalInfo(handle, &info) == UNZ_OK)
	{
		return (int)info.number_entry;
	}

	return 0;
}

bool	DeCompress::moveToFirst()
{
	if (!handle) return false;

	if (unzGoToFirstFile(handle) == UNZ_OK)
	{
// 		if (!GetFileInfo(info))
// 			return FALSE;
		return true;
	}
	return false;
}


bool	DeCompress::moveToNext()
{
	if (!handle) return false;

	while (unzGoToNextFile(handle) == UNZ_OK)
	{
// 		if (!GetFileInfo(info))
// 			return false;
		return true;
	}
	return false;
}

bool	DeCompress::unZip(UnCompressProgress *dialog, wchar* dest, char *password)
{
	if (!handle) return false;

	// 다른곳으로 이동
// 	if (!dest || !CreateFolder(dest))
// 		return false;

// 	totalzipfilenum = GetFileCount();
// 	if (totalzipfilenum == 0)
// 		return false;

	if (!moveToFirst())
		return false;

	do
	{
		if(password)
		{
			if (unzOpenCurrentFilePassword(handle, password) != UNZ_OK)
				return false;
		}
		else
		{
			if (unzOpenCurrentFile(handle) != UNZ_OK)
				return false;
		}

		zipfileInfo info;
		getFileInfo(info);

		// if the item is a folder then simply return 'TRUE'
		if (info.filename[strlen(info.filename) - 1] == '\\')
			return true;

		// build the output filename
		wchar fullpath[MAX_PATH];
		lstrcpy(fullpath, dest);

		// append backslash
		if (fullpath[lstrlen(fullpath) - 1] != '\\')
			lstrcat(fullpath, L"\\");

		// 	if (bIgnoreFilePath)
		// 	{
		// 		char szFName[_MAX_FNAME], szExt[_MAX_EXT];
		// 		_splitpath(info.szFileName, NULL, NULL, szFName, szExt);
		// 		sprintf(info.szFileName, "%s%s", szFName, szExt);
		// 	}

		wchar _path[MAX_PATH];
		unicode::convstr_s(_path, _countof(_path), info.filename);
		lstrcat(fullpath, _path);

		// 해당 폴더 없으면 생성
		if (!createFilePath(fullpath))
			return false;

		writeUnzip(dest);

	} while (moveToNext());

	return true;
}


bool	DeCompress::writeUnzip(wchar *dest)
{


	HANDLE hOutputFile = ::CreateFile(szFilePath, 
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE == hOutputFile)
		return false;

	//     if (unzOpenCurrentFile(m_uzFile) != UNZ_OK)
	// 		return false;

	// read the file and output
	int nRet = UNZ_OK;
	char pBuffer[BUFFERSIZE];
	do
	{
		nRet = unzReadCurrentFile(m_uzFile, pBuffer, BUFFERSIZE);
		if (nRet > 0)
		{
			// output
			DWORD dwBytesWritten = 0;
			if (!::WriteFile(hOutputFile, pBuffer, nRet, &dwBytesWritten, NULL) || dwBytesWritten != (DWORD)nRet)
			{
				nRet = UNZ_ERRNO;
				break;
			}
		}
	}
	while (nRet > 0);

	CloseHandle(hOutputFile);
	unzCloseCurrentFile(m_uzFile);

	if (nRet == UNZ_OK)
		SetFileModTime(szFilePath, info.dwDosDate);
}

bool	DeCompress::createFolder(wchar* szFolder)
{
	if (!szFolder || !lstrlen(szFolder))
		return false;

	DWORD dwAttrib = GetFileAttributes(szFolder);

	// already exists ?
	if (dwAttrib != 0xffffffff)
		return ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);

	// recursively create from the top down
	wchar* szPath = _wcsdup(szFolder);
	wchar* p = wcsrchr(szPath, '\\');

	if (p) 
	{
		// The parent is a dir, not a drive
		*p = '\0';

		// if can't create parent
		if (!CreateFolder(szPath))
		{
			free(szPath);
			return false;
		}
		free(szPath);

		if (!::CreateDirectory(szFolder, NULL)) 
			return false;
	}

	return true;
}

bool  DeCompress::createFilePath(wchar*  szFilePath)
{
	wchar* szPath = _wcsdup(szFilePath);
	wchar* p = wcsrchr(szPath,'\\');
	bool bRes = FALSE;

	if (p)
	{
		*p = '\0';
		bRes = createFolder(szPath);
	}

	free(szPath);
	return bRes;
}

#endif