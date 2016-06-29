#ifndef _FILEMONITOR_
#define _FILEMONITOR_

#include <Windows.h>
#include "predef.h"

class CDirectoryManager;

#define MAX_BUFFER 1024 * 1024

typedef struct _DIRECTORY_INFO
{
	HANDLE hDir ;
	char lpBuffer [MAX_BUFFER];
	unsigned long dwBufLength ;
	OVERLAPPED Overlapped ;
	HANDLE hCompPort ;
} DIRECTORY_INFO, * PDIRECTORY_INFO, *LPDIRECTORY_INFO ;


class _ChangeNotify : public DIRECTORY_INFO
{
public:
	_ChangeNotify();
	virtual ~_ChangeNotify () {};
	virtual bool Start( const wchar* lpPath ) = 0;

protected:
	virtual bool Watch(HANDLE CompletePort) = 0;
};

//////////////////////////////////////////////////////////////////////////

class FileMonitor : public _ChangeNotify
{
public:
	FileMonitor(CDirectoryManager* _parent) : _ChangeNotify () 
	{ 
		parent = _parent;
		threadhandle = NULL; 
	}

	virtual ~FileMonitor();

	bool Start(const wchar* path);
	void Change(const wchar* path);
	void Close();
	void RefreshParent();
	void SendMonitorInfo(unsigned long action, wchar *filename_old, wchar *filename_new = NULL);

	HANDLE	threadhandle;
	String	renamed_old;
	String	renamed_new;

protected:
	bool Watch (HANDLE CompletePort);

	CDirectoryManager*	parent;
	String				monitorpath;


};




#endif