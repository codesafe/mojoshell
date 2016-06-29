
#ifndef _ZIPPER
#define _ZIPPER

#include "../stringutil.h"

#define COMPRESS_METHOD_NONE		0
#define COMPRESS_METHOD_FAST		1
#define COMPRESS_METHOD_NORMAL		2
#define COMPRESS_METHOD_SIZE		3


struct Z_FileInfo
{
	int nFileCount;
	int nFolderCount;
	DWORD dwUncompressedSize;
};

class CZipper  
{
public:
	CZipper(wchar *szFilePath = NULL, wchar *szRootFolder = NULL, bool bAppend = FALSE, int method = COMPRESS_METHOD_NORMAL);
	virtual ~CZipper();

	// simple interface
	static bool ZipFile(wchar *szFilePath); // saves as same name with .zip
	static bool ZipFolder(wchar *szFilePath, bool bIgnoreFilePath); // saves as same name with .zip
	
	// works with prior opened zip
	bool AddFileToZip(wchar *szFilePath, bool bIgnoreFilePath = FALSE);
	bool AddFileToZip(wchar *szFilePath, wchar *szRelFolderPath, unsigned long crc); // replaces path info from szFilePath with szFolder
	bool AddFolderToZip(wchar *szFolderPath, bool bIgnoreFilePath = FALSE);

	// extended interface
	bool OpenZip(wchar *szFilePath, wchar *szRootFolder = NULL, bool bAppend = FALSE);
	bool CloseZip(); // for multiple reuse
	void GetFileInfo(Z_FileInfo& info);

	//////////////////////////////////////////////////////////////////////////

	bool OpenFileToZip(wchar *szFilePath, wchar *szRelFolderPath, wchar *passwd, unsigned long CRC);
	bool AddDataToZip(char *buffer, int size);
	void CloseFileToZip();
	
protected:
	void* m_uzFile;
	wchar m_szRootFolder[MAX_PATH + 1];
	Z_FileInfo m_info;

	int	compress_method;

protected:
	void PrepareSourcePath(wchar *szPath);
	bool GetLastModified(const wchar* szPath, SYSTEMTIME& sysTime);
};


#endif
