#if 0

#ifndef _FILECHECKER_
#define _FILECHECKER_

#include <Windows.h>
#include <string>

class CDirectoryManager;

class FileChecker
{
public :
	FileChecker();
	~FileChecker();

	void	Start(CDirectoryManager *parent, const wchar_t *path);
	void	Stop();
	void	Reset(const wchar_t *path);

	unsigned int ThreadLoop();

	static unsigned int __stdcall threadloop(void * ptr) { return ((FileChecker*)ptr)->ThreadLoop(); }

private:
	std::wstring	path;	
	HANDLE			threadhandle;
	HANDLE			checkhandle[3];
	bool			stop;

	CDirectoryManager *owner;

};

#endif


#endif