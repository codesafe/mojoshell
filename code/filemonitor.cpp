#include <shlwapi.h> 
#include "filemonitor.h"
#include "directoryman.h"
#include "trace.h"

#pragma comment(lib, "shlwapi")

_ChangeNotify::_ChangeNotify ()
{
	hDir   = NULL;
	dwBufLength = 0L;
	hCompPort     = NULL;
	memset(lpBuffer, 0L, sizeof(char)* MAX_BUFFER);
	memset(&Overlapped, 0L, sizeof(OVERLAPPED));
}

//////////////////////////////////////////////////////////////////////////


FileMonitor::~FileMonitor()
{
	Close();
}

void FileMonitor::Close()
{
	if(hCompPort )
		PostQueuedCompletionStatus( hCompPort , 0, 0, NULL );

	Sleep(10);

	if(hDir)
	{
		CloseHandle( hDir );
		hDir   = NULL;
	}

	if(hCompPort )
	{
		CloseHandle( hCompPort );
		hCompPort     = NULL;
	}

	if(threadhandle)
	{
		CloseHandle( threadhandle );
		threadhandle = NULL;
	}
}

bool FileMonitor::Start( const wchar* lpPath )
{
	wchar temp[MAX_PATH] = {0,};
	lstrcpy( temp , lpPath );
	PathAddBackslash(temp);
	monitorpath = temp;

	hDir = CreateFile (
		temp,
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED ,
		NULL);

	if( hDir == INVALID_HANDLE_VALUE )
		return FALSE ;

	// Set up a key(directory info) for each directory
	hCompPort = CreateIoCompletionPort (
		hDir,
		hCompPort,
		( DWORD) this ,
		0);

	if( hCompPort == INVALID_HANDLE_VALUE )
		return FALSE ;

	return Watch (hCompPort);
}

void FileMonitor::Change(const wchar* lpPath)
{
	Close();
	Start(lpPath );
}

void WINAPI HandleDirectoryChange( DWORD Shellfolder )
{
	FileMonitor* pFolder = (FileMonitor*) Shellfolder;
	DWORD numBytes ;
	DWORD cbOffset ;
	LPDIRECTORY_INFO di ;
	LPOVERLAPPED lpOverlapped ;
	PFILE_NOTIFY_INFORMATION fni ;

	do
	{
		// Retrieve the directory info for this directory
		// through the completion key
		BOOL bComp = GetQueuedCompletionStatus(
			(HANDLE)pFolder->hCompPort,
			& numBytes,
			(LPDWORD) &di ,
			& lpOverlapped,
			INFINITE);

		if(!di && !lpOverlapped)
		{
			ExitThread (0);
			return;
		}

		if ( di )
		{
			fni = (PFILE_NOTIFY_INFORMATION )(di->lpBuffer+4);
			do
			{
				wchar temp [MAX_PATH] = { 0 };
				unicode::strncpy(temp, _countof(temp), fni->FileName, fni->FileNameLength/sizeof(wchar));
				//wprintf(L"FileName(%s)\n" , temp);

				switch (fni->Action) 
				{
					case FILE_ACTION_ADDED : 
						_TRACE(L"file added : %s\n", temp);
						pFolder->SendMonitorInfo(fni->Action, temp);
						break;

					case FILE_ACTION_REMOVED : 
						_TRACE(L"file deleted : %s\n", temp); 
						pFolder->SendMonitorInfo(fni->Action, temp);
						break;

					case FILE_ACTION_MODIFIED : 
						_TRACE(L"time stamp or attribute changed : %s\n",temp); 
						pFolder->SendMonitorInfo(fni->Action, temp);
						break;

					case FILE_ACTION_RENAMED_OLD_NAME : 
						_TRACE(L"file name changed - old name : %s\n", temp); 
						pFolder->renamed_old = temp;
						//pFolder->SendMonitorInfo(fni->Action, temp);
						break;

					case FILE_ACTION_RENAMED_NEW_NAME : 
						_TRACE(L"file name changed - new name : %s\n", temp);
						pFolder->renamed_new = temp;
						if( pFolder->renamed_old.empty() == false &&  pFolder->renamed_new.empty() == false)
						{
							pFolder->SendMonitorInfo(fni->Action, (wchar*)pFolder->renamed_old.c_str(), (wchar*)pFolder->renamed_new.c_str());
						}

						pFolder->renamed_old.clear();
						pFolder->renamed_new.clear();
						break;

					default : 
						_TRACE(L"unknown event : %d\n", fni->Action);
						break;
				}

				cbOffset = fni ->NextEntryOffset;
				fni = (PFILE_NOTIFY_INFORMATION )((LPBYTE) fni + cbOffset);

			} while( cbOffset );

			memset(pFolder->lpBuffer, 0L, sizeof(char)* MAX_BUFFER);
			
			// Reissue the watch command
			ReadDirectoryChangesW(
				pFolder->hDir ,
				pFolder->lpBuffer ,
				MAX_BUFFER,
				//TRUE,
				FALSE,
				FILE_NOTIFY_CHANGE_FILE_NAME |
				FILE_NOTIFY_CHANGE_DIR_NAME |
				FILE_NOTIFY_CHANGE_ATTRIBUTES |
				FILE_NOTIFY_CHANGE_SIZE |
				FILE_NOTIFY_CHANGE_LAST_WRITE |
				FILE_NOTIFY_CHANGE_LAST_ACCESS |
				FILE_NOTIFY_CHANGE_CREATION | 
				FILE_NOTIFY_CHANGE_SIZE | 
				FILE_NOTIFY_CHANGE_FILE_NAME | 
				FILE_NOTIFY_CHANGE_DIR_NAME,
				& pFolder->dwBufLength ,
				& pFolder->Overlapped ,
				NULL);
		}

	} while( pFolder->threadhandle );
}

bool FileMonitor::Watch(HANDLE CompletePort)
{
	DWORD tid ;

	if(ReadDirectoryChangesW (
		hDir,
		lpBuffer,
		MAX_BUFFER,
		//TRUE,
		FALSE,
		FILE_NOTIFY_CHANGE_FILE_NAME |
		FILE_NOTIFY_CHANGE_DIR_NAME |
		FILE_NOTIFY_CHANGE_ATTRIBUTES |
		FILE_NOTIFY_CHANGE_SIZE |
		FILE_NOTIFY_CHANGE_LAST_WRITE |
		FILE_NOTIFY_CHANGE_LAST_ACCESS |
		FILE_NOTIFY_CHANGE_CREATION | 
		FILE_NOTIFY_CHANGE_SIZE | 
		FILE_NOTIFY_CHANGE_FILE_NAME | 
		FILE_NOTIFY_CHANGE_DIR_NAME,
		& dwBufLength,
		& Overlapped,
		NULL))
	{
		threadhandle = CreateThread (
			NULL,
			0,
			( LPTHREAD_START_ROUTINE) HandleDirectoryChange ,
			this,
			0,
			& tid);
		if(threadhandle )
			return true ;
		else
			return false ;
	}

	return false ;
}

void FileMonitor::RefreshParent()
{
	parent->RefreshDirectory();
}

void FileMonitor::SendMonitorInfo(unsigned long action, wchar *filename_old, wchar *filename_new)
{
	parent->SendMonitorInfo(action, (wchar*)monitorpath.c_str(), filename_old, filename_new);
}