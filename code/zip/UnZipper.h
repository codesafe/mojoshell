#ifndef _UNZIPPER_
#define _UNZIPPER_

#include <Windows.h>
#include "../stringutil.h"
#include "../customdialog.h"

#define CHECK_ZIP_ERROR		-1
#define CHECK_ZIP_OK			0
#define CHECK_ZIP_ENCRYPT		1

#define MAX_COMMENT	255

struct UZ_FileInfo
{
        char szFileName[260 + 1];
        char szComment[MAX_COMMENT + 1];
        
        unsigned long dwVersion;  
        unsigned long dwVersionNeeded;
        unsigned long dwFlags;   
        unsigned long dwCompressionMethod; 
        unsigned long dwDosDate;        
        unsigned long dwCRC;   
        unsigned long dwCompressedSize; 
        unsigned long dwUncompressedSize;
        unsigned long dwInternalAttrib; 
        unsigned long dwExternalAttrib; 
        bool bFolder;
};

class UnZipper  
{
public:
        UnZipper(const char* szFileName = NULL);
        virtual ~UnZipper();
        
        static bool Unzip(UnCompressProgress *dialog, wchar*  szFileName, wchar*  szFolder = NULL, bool bIgnoreFilePath = FALSE, const wchar *passwd = NULL);
		static int UnZipper::IsEncryptZip(wchar*  filename);

        // works with prior opened zip
        //bool Unzip(bool bIgnoreFilePath = FALSE); // unzips to output folder or sub folder with zip name 
        bool UnzipTo(UnCompressProgress *dialog, wchar*  szFolder, bool bIgnoreFilePath = FALSE, char *password=NULL); // unzips to specified folder

        // extended interface
        bool OpenZip(wchar*  szFileName);
        bool CloseZip(); // for multiple reuse
        bool SetOutputFolder(wchar*  szFolder); // will try to create
        
        // unzip by file index
        int GetFileCount();
        bool GetFileInfo(int nFile, UZ_FileInfo& info);
        //bool UnzipFile(int nFile, wchar*  szFolder = NULL, bool bIgnoreFilePath = FALSE);
        
        // unzip current file
        bool GotoFirstFile(wchar*  szExt = NULL);
        bool GotoNextFile(wchar*  szExt = NULL);
        bool GetFileInfo(UZ_FileInfo& info);
        bool UnzipFile(UnCompressProgress *dialog, wchar*  szFolder = NULL, bool bIgnoreFilePath = FALSE);

        // helpers
        //bool GotoFile(char*  szFileName, bool bIgnoreFilePath = TRUE);
        bool GotoFile(int nFile);
        
protected:
        void* m_uzFile;
        wchar m_szOutputFolder[MAX_PATH + 1];

		int totalzipfilenum;
		int currentzipfilenum;

protected:
        static bool CreateFolder(wchar*  szFolder);
        static bool CreateFilePath(wchar*  szFilePath); // truncates from the last '\'
        static bool SetFileModTime(wchar*  szFilePath, DWORD dwDosDate);

		int CheckEncryptedZip();
};

//////////////////////////////////////////////////////////////////////////


struct zipfileInfo
{
	char filename[MAX_PATH + 1];
	char comment[MAX_COMMENT + 1];

	unsigned long dwVersion;  
	unsigned long dwVersionNeeded;
	unsigned long dwFlags;   
	unsigned long dwCompressionMethod; 
	unsigned long dwDosDate;        
	unsigned long dwCRC;   
	unsigned long dwCompressedSize; 
	unsigned long dwUncompressedSize;
	unsigned long dwInternalAttrib; 
	unsigned long dwExternalAttrib; 
	bool isfolder;
};

class DeCompress
{
public :
	DeCompress();
	~DeCompress();

	bool	openZip(String filename);
	bool	closeZip();

	bool	unZip(UnCompressProgress *dialog, wchar* dest, char *password);
	bool	getFileInfo(zipfileInfo& info);
	int		getTotalFileCount();

private :

	bool	createFolder(wchar* folderpath);
	bool	createFilePath(wchar* path);
	bool	moveToFirst();
	bool	moveToNext();

	bool	writeUnzip(wchar *dest);


	void* handle;
};


#endif

