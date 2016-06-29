#if 0

#include <process.h>
#include <atlconv.h>

#include "filechecker.h"
#include "directoryman.h"

#include "trace.h"
#include "utility.h"


FileChecker::FileChecker()
{

}

FileChecker::~FileChecker()
{

}


void FileChecker::Start(CDirectoryManager *parent, const wchar_t *_path)
{
	owner = parent;
	path = _path;
	stop = false;
	unsigned threadid = 0;

	checkhandle[1] = CreateEvent( NULL, TRUE, FALSE, NULL );
	checkhandle[2] = CreateEvent( NULL, TRUE, FALSE, NULL );

	threadhandle = (HANDLE)_beginthreadex(NULL, 0, threadloop, (void*)this, 0, &threadid);
	//SetThreadName(threadhandle, 'Main thread'); 
}


void FileChecker::Stop()
{		
	stop = true;
	SetEvent(checkhandle[2]);
}

void FileChecker::Reset(const wchar_t *_path)
{
	path = _path;
	//FindCloseChangeNotification(checkhandle[0]);
	SetEvent(checkhandle[1]);
}

#if 0

#else

unsigned int FileChecker::ThreadLoop()
{
	while(stop == false)
	{
		DWORD dwWaitStatus;
		wchar_t checkpath[_MAX_FNAME] = { 0, };
		unicode::strcpy( checkpath, _MAX_FNAME, path.c_str() );

		_TRACE(unicode::format(L"Bind : %s\n", checkpath));
		utility::Log(checkpath);

		// root일 경우
		if( unicode::strlen(checkpath) == 2 )
		{
			checkpath[2] = (TCHAR)'\\';
			checkpath[3] = (TCHAR)'\0';
		}

		checkhandle[0] = FindFirstChangeNotification( 
			checkpath,
			FALSE,                         // do not watch subtree 
			FILE_NOTIFY_CHANGE_FILE_NAME|
			FILE_NOTIFY_CHANGE_DIR_NAME|
			FILE_NOTIFY_CHANGE_CREATION
 			//FILE_NOTIFY_CHANGE_ATTRIBUTES|
 			//FILE_NOTIFY_CHANGE_SIZE|
 			//FILE_NOTIFY_CHANGE_LAST_WRITE
			); // watch file name changes 

		if (checkhandle[0] == INVALID_HANDLE_VALUE || checkhandle[0] == NULL ) 
			return false;

		bool loop = true;
		while (loop) 
		{ 
			dwWaitStatus = WaitForMultipleObjects(3, checkhandle, FALSE, INFINITE); 

			switch (dwWaitStatus) 
			{ 
			case WAIT_OBJECT_0: 
				{
					// 뭔가 변경됨!
					//owner->RefreshDirectory(); 

					if ( FindNextChangeNotification(checkhandle[0]) == FALSE )
					{
						utility::Log(L"Error FindNextChangeNotification\n");
					}
				}

				break; 

				// reset
			case WAIT_OBJECT_0 + 1:
				{
					// 이전 핸들 제거
					FindCloseChangeNotification(checkhandle[0]);
					ResetEvent(checkhandle[0]);

					wchar_t resetpath[_MAX_FNAME] = { 0, };
					unicode::strcpy( resetpath, _MAX_FNAME, path.c_str() );

					// root일 경우
					if( unicode::strlen(resetpath) == 2 )
					{
						resetpath[2] = (TCHAR)'\\';
						resetpath[3] = (TCHAR)'\0';
					}

					_TRACE(unicode::format(L"Bind : %s\n", resetpath));
					utility::Log(resetpath);

					checkhandle[0] = FindFirstChangeNotification( 
						resetpath,
						FALSE,
						FILE_NOTIFY_CHANGE_FILE_NAME|
						FILE_NOTIFY_CHANGE_DIR_NAME|
						FILE_NOTIFY_CHANGE_CREATION
						//FILE_NOTIFY_CHANGE_ATTRIBUTES|
						//FILE_NOTIFY_CHANGE_SIZE|
						//FILE_NOTIFY_CHANGE_LAST_WRITE
						);

					ResetEvent(checkhandle[1]);
				}
				break;

				// end
			case WAIT_OBJECT_0 + 2:
				loop = false;
				break;

			case WAIT_TIMEOUT:

				// A timeout occurred, this would happen if some value other 
				// than INFINITE is used in the Wait call and no changes occur.
				// In a single-threaded environment you might not want an
				// INFINITE wait.

				//printf("\nNo changes in the timeout period.\n");
				break;

			default: 
				//printf("\n ERROR: Unhandled dwWaitStatus.\n");
				//ExitProcess(GetLastError());
				break;
			}
		}
	}

	FindCloseChangeNotification(checkhandle[0]);
	CloseHandle( checkhandle[1] );
	CloseHandle( checkhandle[2] );
	return 0;
}

#endif

#endif