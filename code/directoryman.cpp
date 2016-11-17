		
#include <cctype>
#include <algorithm>
#include "directoryman.h"
#include "stringutil.h"
#include "trace.h"

#include "valueman.h"
#include "utility.h"
#include "filechecker.h"
#include "filemonitor.h"
#include "main.h"
#include "colormanager.h"
#include "imageloader.h"
#include "configmanager.h"

#include "compress/compressor.h"



CDirectoryManager::CDirectoryManager()
{
	ResetParam();
}

CDirectoryManager::~CDirectoryManager()
{
}


void	CDirectoryManager::Init(wxFrame *p, int what, CINI *ini)
{
	pathinfo_ini = ini;
	whatwindow = what;
	activewindow = false;
	parentwindow = p;

	CollectDriveinfo();

	m_displayoption = _DISPLAY_CAPITAL;
	m_sortmethod = _SORT_NAME_MIN;

	String softstr = std::wstring(L"SORT") + ( whatwindow == _LEFT_WINDOW ? L"_LEFT" : L"_RIGHT");
	const wchar *ret = pathinfo_ini->GetString(softstr.c_str());
	if( ret )
	{
		if( String(ret) == L"name-min" )
			m_sortmethod = _SORT_NAME_MIN;
		else if( String(ret) == L"name-max" )
			m_sortmethod = _SORT_NAME_MAX;
		if( String(ret) == L"time-min" )
			m_sortmethod = _SORT_TIME_MIN;
		else if( String(ret) == L"time-max" )
			m_sortmethod = _SORT_TIME_MAX;		
		if( String(ret) == L"size-min" )
			m_sortmethod = _SORT_SIZE_MIN;
		else if( String(ret) == L"size-max" )
			m_sortmethod = _SORT_SIZE_MAX;		
	}

	CollectCurrentDirInfo();

	//filechecker = new FileChecker;
	filemonitor = new FileMonitor(this);
	StartChecker();

	wxImage	selectimage;
	selectimage.LoadFile(L"arrow.bmp", wxBITMAP_TYPE_BMP);
	selectbitmap = new wxBitmap(selectimage);

	lastselectline = -1;
}


void	CDirectoryManager::ResetParam()
{
	m_selectedpos = 0;
	m_displaytop = 0;
	m_displaylines = 0;
	keymodifyer = 0;
	m_currentdisplaypos = 0;
}

void	CDirectoryManager::AnalizeDirectory(const wchar *path)
{
	HANDLE hSrch;
	WIN32_FIND_DATAW wfd;
	BOOL bResult=TRUE;
	wchar drive[_MAX_DRIVE];
	wchar dir[MAX_PATH];
	wchar temppath[MAX_PATH];
	
	unicode::strcpy(temppath, _countof(temppath), path);
	unicode::strcat(temppath, _countof(temppath), _T("\\*.*"));
 
	m_parent.m_type = TYPE_NONE;
	if( utility::IsRootDirectory( path ) == false )
	{
		m_parent.m_type = TYPE_DIRECTORY;
		m_parent.m_name = _T("..");
	}

	hSrch = FindFirstFile(temppath, &wfd);
	if (hSrch == INVALID_HANDLE_VALUE) return;

	_wsplitpath_s(path, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0);

	while (bResult) 
	{
		if( unicode::strcmp(wfd.cFileName, _T(".") ) == 0 )
		{
		}
		else if( unicode::strcmp(wfd.cFileName, _T("..") ) == 0 && wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
		}
		else if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
		{
			FILEINFO info;
			info.m_type = TYPE_DIRECTORY;
			info.m_name = wfd.cFileName;
			info.m_ext = _T("[폴더]");

			info.m_attribute = (wfd.dwFileAttributes & ATTR_READONLY) ? ATTR_READONLY : 0;
			info.m_attribute |= (wfd.dwFileAttributes & ATTR_HIDDEN) ? ATTR_HIDDEN : 0;
			info.m_attribute |= (wfd.dwFileAttributes & ATTR_SYSTEM) ? ATTR_SYSTEM : 0;
			info.m_attribute |= (wfd.dwFileAttributes & ATTR_ARCHIVE) ?  ATTR_ARCHIVE : 0;

			//transform(info.m_name.begin(), info.m_name.end(), info.m_name.begin(), toupper);

			int len = unicode::strlen(path);
			if(path[len-1] == '\\')
			info.m_fullpath = String(path) + wfd.cFileName;
			else
			info.m_fullpath = String(path) + String(_T("\\")) + wfd.cFileName;
			info.m_creationtime = wfd.ftCreationTime;
			info.m_LastAccessTime = wfd.ftLastAccessTime;
			info.m_LastWriteTime = wfd.ftLastWriteTime;

//			_TRACE( _T("dir : %s\n"), info.m_name.c_str());
			m_directoryinfolist.push_back(info);
		} 
		else 
		{
			String name, ext;
			
			FILEINFO info;
			info.m_type = TYPE_FILE;
			info.m_attribute = (wfd.dwFileAttributes & ATTR_READONLY) ? ATTR_READONLY : 0;
			info.m_attribute |= (wfd.dwFileAttributes & ATTR_HIDDEN) ? ATTR_HIDDEN : 0;
			info.m_attribute |= (wfd.dwFileAttributes & ATTR_SYSTEM) ? ATTR_SYSTEM : 0;
			info.m_attribute |= (wfd.dwFileAttributes & ATTR_ARCHIVE) ?  ATTR_ARCHIVE : 0;

			name = utility::GetName(wfd.cFileName);
			ext = utility::GetExtention(wfd.cFileName);

			ext.erase(ext.begin());
			info.m_fullpath = String(path) + String(_T("\\")) + info.m_name;
			info.m_excutepath = String(path) + String(_T("\\")) + wfd.cFileName;

			//transform(name.begin(), name.end(), name.begin(), toupper);
			//transform(ext.begin(), ext.end(), ext.begin(), toupper);

			info.m_name = name;
			info.m_ext = ext;
			info.m_creationtime = wfd.ftCreationTime;
			info.m_LastAccessTime = wfd.ftLastAccessTime;
			info.m_LastWriteTime = wfd.ftLastWriteTime;
			info.m_filesize = (((__int64)wfd.nFileSizeHigh)<<32) + wfd.nFileSizeLow;;

//			_TRACE( _T("file : %s\n"), info.m_name.c_str());
			m_fileinfolist.push_back(info);
		}

		bResult=FindNextFile(hSrch,&wfd);
	}

	FindClose(hSrch);
}


#include <cstring>
#include <sstream>
#include <iostream>
using namespace std;


void	CDirectoryManager::CollectDriveinfo()
{
	m_driveinfolist.clear();

	//HANDLE hDevice = NULL;
	HANDLE fileFind = NULL;

	// 드라이브 정보 얻기
	wchar drv='A';
	while(drv != '[')
	{
		const wchar *charDrvCF;
		wchar *charDrv;

		wstringstream Str;
		wstring drvStr;
		Str<<drv;
		Str>>drvStr;
		
		wstring drvSpc = drvStr + _T(":\\");
		wstring fCheck = _T("\\\\.\\");
		wstring fhCheck=fCheck+drvStr + _T(":");

		charDrvCF=fhCheck.c_str();
		charDrv = (wchar*)drvSpc.c_str();      
		
		//hDevice = CreateFile(charDrvCF, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,  NULL, OPEN_EXISTING, 0, NULL);

		//if(hDevice!=INVALID_HANDLE_VALUE)
		{
			wchar buffer[256] ={0,};
			BOOL ret = GetVolumeInformation(charDrv, buffer, 256, NULL, NULL, NULL,NULL,NULL	);
			charDrv[unicode::strlen(charDrv)-1] = 0;

			switch(GetDriveType(charDrv))
			{
			case DRIVE_FIXED:
				{
					FILEINFO drive;
					drive.m_type = TYPE_DRIVE;
					drive.m_selected = false;
					drive.m_name = charDrv;

					if( buffer[0] == 0 )
						drive.m_name += L" 로컬 디스크";
					else
						drive.m_name += wstring(L" ") + buffer;
					drive.m_fullpath = charDrv[0];
					drive.m_ext = L"DISK";
					m_driveinfolist.push_back(drive);

					//cout<<"Fixed drive detected: "<<charDrv<<endl;
					break;
				}
			case DRIVE_REMOVABLE:
				{
					FILEINFO drive;
					drive.m_type = TYPE_DRIVE;
					drive.m_selected = false;
					drive.m_name = charDrv;
					if( buffer[0] == 0 )
						drive.m_name += L" 이동식 디스크";
					else
						drive.m_name += wstring(L" ") + buffer;

					drive.m_fullpath = charDrv[0];
					drive.m_ext = L"REMOVABLE";
					m_driveinfolist.push_back(drive);

					//cout<<"Removable drive detected: "<<charDrv<<endl;
					break;
				}
			case DRIVE_NO_ROOT_DIR:
				{
					_TRACE(L"There is no volume mounted at the specified path.");
					break;
				}
			case DRIVE_REMOTE:
				{
					_TRACE(L"The drive is a remote (network) drive. ");
					break;
				}
			case DRIVE_CDROM:
				{
					FILEINFO drive;
					drive.m_type = TYPE_DRIVE;
					drive.m_selected = false;
					drive.m_name = charDrv;
					if( buffer[0] == 0 )
						drive.m_name += L" BD-ROM 드라이브";
					else
						drive.m_name += wstring(L" ") + buffer;

					drive.m_fullpath = charDrv[0];
					drive.m_ext = L"CDROM";
					m_driveinfolist.push_back(drive);

					//cout<<"The drive is a CD-ROM drive. "<<charDrv<<endl;
					break;
				}
			case DRIVE_RAMDISK:
				{
					FILEINFO drive;
					drive.m_type = TYPE_DRIVE;
					drive.m_selected = false;
					drive.m_name = charDrv;
					drive.m_fullpath = charDrv[0];
					drive.m_ext = L"RAMDISK";
					m_driveinfolist.push_back(drive);

					//cout<<"The drive is a RAM disk. "<<charDrv<<endl;
					break;
				}
			case DRIVE_UNKNOWN:
				{
					_TRACE(L"The drive type cannot be determined. ");
					break;
				}
			}
		}

		//CloseHandle(hDevice);
		drv++;
	}
	
	// INI 안의 DRIVE정보를 비교 / 갱신
	bool needsave = false;
	for(size_t i=0; i<m_driveinfolist.size(); i++)
	{
		String drive = String(m_driveinfolist[i].m_name.c_str(), 0, 1);
		const wchar *ret = pathinfo_ini->GetString(drive.c_str());

		if(ret)
		{
			// 드라이브 정보 있슴
			m_driveinfolist[i].m_fullpath = ret;
		}
		else
		{
			// 없어서 새로 설정
			m_driveinfolist[i].m_fullpath = drive + (L":");
			pathinfo_ini->Set(drive.c_str(), m_driveinfolist[i].m_fullpath.c_str());
			needsave = true;
		}
	}

	if( needsave )
		SavePathinfo();

	std::wstring drivestr = std::wstring(L"LASTDRIVE") + ( whatwindow == _LEFT_WINDOW ? L"_LEFT" : L"_RIGHT");
	const wchar *lastdrive = pathinfo_ini->GetString(drivestr.c_str());

	lastdrive = CheckValidDirectory(String(lastdrive));
	if( lastdrive )
	{
		m_currentDir = lastdrive;
		pathinfo_ini->Set( drivestr.c_str(), m_currentDir.c_str());
	}
	else
	{
		// 없슴.. 디폴트 설정

		wchar dir[MAX_PATH];
		GetCurrentDirectory(_countof(dir), dir);
		m_currentDir = dir;

		//pathinfo_ini->Set(drivestr.c_str(), GetDrive(m_currentDir.c_str()).c_str());
		pathinfo_ini->Set( drivestr.c_str(), m_currentDir.c_str() );
		SavePathinfo();
	}
}

// 디렉토리 정보 다시 읽음
void	CDirectoryManager::CollectCurrentDirInfo()
{
	filecheckerlock.Enter();

	m_directoryinfolist.clear();
	m_fileinfolist.clear();

	AnalizeDirectory(m_currentDir.c_str());

	// 정렬
	SortList();
	RestoreLastSelect();

	filecheckerlock.Leave();
}

void	CDirectoryManager::Up()
{
	if( m_selectedpos > 0 )
		m_selectedpos--;

	if( m_selectedpos <= m_displaytop)
		m_displaytop = m_selectedpos;

	SetLastSelect();

//	if( m_displaytop + m_displaylines <= m_selectedpos )
//		m_displaytop++;
}

void	CDirectoryManager::Down()
{
	if( (int)m_finalresult.size() > m_selectedpos+1 )
		m_selectedpos++;

	if( m_displaytop + m_displaylines <= m_selectedpos )
		m_displaytop++;

	SetLastSelect();
}

void	CDirectoryManager::Home()
{
	// 맨 위로
	m_selectedpos = 0;
	m_displaytop = 0;
	SetLastSelect();
}

void	CDirectoryManager::End()
{
	// 맨마지막으로 이동
	int len = (int)m_finalresult.size();

	if( len <= m_displaylines )
	{
		m_selectedpos = len-1;
		m_displaytop = 0;
	}
	else
	{
		m_displaytop = len - m_displaylines;
		m_selectedpos = len -1;
	}
	SetLastSelect();
}


void	CDirectoryManager::Pagedown()
{
	int filenum = (int)m_finalresult.size();
	if( m_displaytop + m_displaylines < filenum )
	{
		m_displaytop += m_displaylines;
		m_selectedpos += m_displaylines;

		m_selectedpos = m_selectedpos >= filenum ? filenum-1 : m_selectedpos;
	}
	else
	{
		m_selectedpos = filenum-1;
	}
	SetLastSelect();
}

void	CDirectoryManager::Pageup()
{
	if( m_displaytop - m_displaylines >= 0 )
	{
		m_displaytop -= m_displaylines;
		m_selectedpos -= m_displaylines;
	}
	else
	{
		m_displaytop = 0;
		m_selectedpos = 0;
	}
	SetLastSelect();
}

void	CDirectoryManager::PressKeyboard(int action, wxKeyEvent& event)
{
	int keycode = event.GetKeyCode();
	//int keycode = event.GetUnicodeKey();
	int modfier = event.GetModifiers();
	keymodifyer = modfier;

	//////////////////////////////////////////////////////////////////////////
		
	bool ret = false;

	ret |= ActionKey(action, keycode, modfier);

	// shift key
	ret |= ActionSHIFTKey(action, keycode, modfier);

	// CTRL키 조합
	ret |= ActionCTRLKey(action, keycode, modfier);

	// ALT키 조합
	ret |= ActionALTKey(action, keycode, modfier);


	if( keycode >= '0' && keycode <= 'z' && modfier == 0 && ret == false && action == _KEY_DOWN)
	{
		_TRACE(L"keycode : %c\n", keycode);
		unsigned long qtime = timeGetTime();
		if( !quicksearch.empty() && qtime - quicksearchTime > 1500 )
		{
			// 입력시간이 일정기간 넘으면 초기화
			quicksearch.clear();
		}

		quicksearch += keycode;
		quicksearchTime = timeGetTime();

		DoQuickSearch();
	}

	if( ret )
	{
		ResetQuickSearch();
	}

}

bool	CDirectoryManager::ActionKey(int action, int keycode, int modfier)
{
	bool ret = false;
	if( keycode == WXK_RETURN && action == _KEY_DOWN )
	{
		ActionCur( (modfier & wxMOD_CONTROL ? true : false) );
		ret = true;		
	}
	else if( (
		( keycode == WXK_F1 && action == _KEY_DOWN ) ||
		( keycode == WXK_F2 && action == _KEY_DOWN ) ||
		( keycode == WXK_F3 && action == _KEY_DOWN ) ||
		( keycode == WXK_F4 && action == _KEY_DOWN ) ||
		( keycode == WXK_F5 && action == _KEY_DOWN ) ||
		( keycode == WXK_F6 && action == _KEY_DOWN ) ||
		( keycode == WXK_F7 && action == _KEY_DOWN ) ||
		( keycode == WXK_F8 && action == _KEY_DOWN ) ||
		( keycode == WXK_F9 && action == _KEY_DOWN ) ||
		( keycode == WXK_F10 && action == _KEY_DOWN ) ) && !(modfier & wxMOD_ALT) )
	{
		for(int i=WXK_F1; i<WXK_F10; i++)
		{
			if( (keycode == i) && (action == _KEY_DOWN) )
			{
				FunctionKey::GetInstance()->ExecFunction(i - WXK_F1, m_finalresult[m_selectedpos]);
				break;
			}
		}
		ret = true;
	}
	else if( keycode == WXK_NUMPAD_MULTIPLY && action == _KEY_DOWN )
	{
		// 선택된것 --> 해제 , 안된것 선택
		for (int i=0; i < (int)m_finalresult.size(); i++)
		{
			if( m_finalresult[i].m_type & TYPE_DIRECTORY || m_finalresult[i].m_type & TYPE_FILE )
			{
				if( m_finalresult[i].m_name != L".." )
				{
					m_finalresult[i].m_selected = !m_finalresult[i].m_selected;
				}
			}
		}
		ret = true;
	}
	else if( keycode == WXK_NUMPAD_ADD && action == _KEY_DOWN )
	{
		String key = L"addselect";
		EnterTextDialog dialog( parentwindow, L"고급 선택", L"선택할 확장자를 넣어주세요.", L"", key.c_str());
		dialog.ShowModal();

		String retstr =  Valueman::GetInstance()->getvalue(key.c_str());
		if( !retstr.empty() )
		{
			Valueman::GetInstance()->remove(key.c_str());
			unicode::Upper(retstr);

			for (int i=0; i < (int)m_finalresult.size(); i++)
			{
				if( m_finalresult[i].m_type & TYPE_FILE )
				{
					String ext = m_finalresult[i].m_ext;
					unicode::Upper(ext);
					if( ext == retstr )
					{
						m_finalresult[i].m_selected = true;
					}
				}
			}
			ret = true;
		}
	}
	else if( keycode == WXK_NUMPAD_SUBTRACT && action == _KEY_DOWN )
	{
		String key = L"removeselect";
		EnterTextDialog dialog( parentwindow, L"고급 선택 해제", L"해제할 확장자를 넣어주세요.", L"", key.c_str());
		dialog.ShowModal();

		String retstr =  Valueman::GetInstance()->getvalue(key.c_str());
		if( !retstr.empty() )
		{
			Valueman::GetInstance()->remove(key.c_str());
			unicode::Upper(retstr);

			for (int i=0; i < (int)m_finalresult.size(); i++)
			{
				if( m_finalresult[i].m_type & TYPE_FILE )
				{
					String ext = m_finalresult[i].m_ext;
					unicode::Upper(ext);
					if( ext == retstr )
					{
						m_finalresult[i].m_selected = false;
					}
				}
			}
			ret = true;
		}
	}
	else if( keycode == WXK_NUMPAD_DIVIDE && action == _KEY_DOWN )
	{
		int type = 0;
		if( m_finalresult[m_selectedpos].m_type & TYPE_DIRECTORY && m_finalresult[m_selectedpos].m_name != L".." )
		{
			type = TYPE_DIRECTORY;
		}
		else if( m_finalresult[m_selectedpos].m_type & TYPE_FILE )
		{
			type = TYPE_FILE;
		}

		if( type != 0)
		{
			bool selected = m_finalresult[m_selectedpos].m_selected;
			for (int i=0; i < (int)m_finalresult.size(); i++)
			{
				if( m_finalresult[i].m_type & type )
				{
					if( m_finalresult[i].m_name != L".." )
					{
						m_finalresult[i].m_selected = !selected;
					}
				}
			}
		}
		ret = true;
	}
	else if( keycode == '\\' && action == _KEY_DOWN )
	{
		BackToRoot();
		ret = true;
	}
	else if( keycode == '.' && action == _KEY_DOWN )
	{
		MoveToBackDir();	
		ret = true;
	}
	else if( keycode == '/' && action == _KEY_DOWN )
	{
		ExcuteInput();
		ret = true;
	}
	else if( (keycode == WXK_RIGHT && action == _KEY_DOWN) || (keycode == WXK_PAGEDOWN && action == _KEY_DOWN) )
	{
		if( !quicksearchlist.empty() )
		{
			RollQuickSearchFront();
		}
		else
		{
			Pagedown();
			ret = true;
		}
	}
	else if( (keycode == WXK_LEFT && action == _KEY_DOWN) || (keycode == WXK_PAGEUP && action == _KEY_DOWN) )
	{
		if( !quicksearchlist.empty() )
		{
			RollQuickSearchBack();
		}
		else
		{
			Pageup();
			ret = true;
		}
	}
	else if( keycode == WXK_UP && action == _KEY_DOWN )
	{
		Up();
		ret = true;
	}
	else if( keycode == WXK_DOWN && action == _KEY_DOWN )
	{
		Down();
		ret = true;
	}
	else if( keycode == WXK_HOME && action == _KEY_DOWN )
	{
		Home();
	}
	else if( keycode == WXK_END && action == _KEY_DOWN )
	{
		End();
		ret = true;
	}
	else if( keycode == WXK_DELETE && action == _KEY_DOWN )
	{
		bool recyclebin = true;
		if( modfier & wxMOD_SHIFT )
			recyclebin = false;

		std::wstring deletemsg;		// 메시지 텍스트
		std::wstring mstr = !recyclebin ? L"삭제" : L"휴지통으로 이동";

		int selnum = 0;
		for(int i=0; i<(int)m_finalresult.size(); i++)
		{
			if( m_finalresult[i].m_selected )
				selnum++;
		}

		if(selnum > 0)
		{
			// 선택된것들 지우기
			deletemsg = unicode::format(L"선택된 %d개의 파일/폴더를 %s 합니까?", selnum, mstr.c_str());
			wxMessageDialog dial(NULL,
				deletemsg.c_str(), 
				mstr.c_str(), 
				wxCENTER | wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);

			int ret = dial.ShowModal();

			// 선택된 여러개 지우기
			if( ret == wxID_YES )
			{
				DeleteSelectedFile(recyclebin);
			}
		}
		else
		{
			// 현재 커서꺼 한개 지우기
			if( m_finalresult[m_selectedpos].m_type & TYPE_DIRECTORY || m_finalresult[m_selectedpos].m_type & TYPE_FILE )
			{
				if(m_finalresult[m_selectedpos].m_type & TYPE_DIRECTORY)
				{
					deletemsg = unicode::format( L"%s\n\n폴더를 %s 합니까?", m_finalresult[m_selectedpos].m_name.c_str(), mstr.c_str() );
				}
				else if(m_finalresult[m_selectedpos].m_type & TYPE_FILE)
				{
					deletemsg = unicode::format( L"%s.%s\n\n파일를 %s 합니까?", m_finalresult[m_selectedpos].m_name.c_str(), m_finalresult[m_selectedpos].m_ext.c_str(), mstr.c_str());
				}

				wxMessageDialog dial(NULL, 
					deletemsg.c_str(), 
					mstr.c_str(), 
					wxCENTER | wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
				//wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION);
				int ret = dial.ShowModal();

				if( ret == wxID_YES )
				{
					Deletefile(recyclebin);
				}
			}
		}
		ret = true;
	}
	else if( keycode == WXK_SPACE && action == _KEY_DOWN )
	{
		if( m_finalresult[m_selectedpos].m_type & TYPE_DIRECTORY || m_finalresult[m_selectedpos].m_type & TYPE_FILE )
		{
			if( m_finalresult[m_selectedpos].m_name != L".." )
			{
				// 현재것 select 토클
				m_finalresult[m_selectedpos].m_selected = !m_finalresult[m_selectedpos].m_selected;
			}
			Down();
		}
		else
			Down();

		ret = true;
	}
	else if( keycode == WXK_BACK && action == _KEY_DOWN )
	{
		if( !quicksearch.empty() )
		{
			quicksearch.erase(quicksearch.size() - 1);
		}

		if( !quicksearch.empty() )
			DoQuickSearch();

	}
	else if( keycode == WXK_ESCAPE && action == _KEY_DOWN )
	{
		ResetQuickSearch();
	}
	else if( keycode == WXK_TAB && action == _KEY_UP )
	{
		SelectImage();
	}

	return ret;
}

bool	CDirectoryManager::ActionSHIFTKey(int action, int keycode, int modfier)
{
	bool ret = false;
	if( modfier & wxMOD_SHIFT )
	{
		// 드라이브 이동
		for (int c = 'A'; c <= 'Z'; c++)
		{
			if( action == _KEY_DOWN && keycode == c )
			{
				if( m_currentDir.c_str()[0] != c )
				{
					for (int i=0; i<(int)m_driveinfolist.size(); i++)
					{
						if( m_driveinfolist[i].m_name.c_str()[0] == c )
						{
							ChangeDrive(m_driveinfolist[i].m_fullpath.c_str());
							ret = true;
							break;
						}
					}
				}

			}
		}
	}

	return ret;
}

bool	CDirectoryManager::ActionCTRLKey(int action, int keycode, int modfier)
{
	bool ret = false;
	if( modfier & wxMOD_CONTROL )
	{
		if( keycode == 'C' && action == _KEY_DOWN )
		{
			wxFileDataObject *dobj = new wxFileDataObject;

			int selnum = 0;
			for(int i=0; i<(int)m_finalresult.size(); i++)
			{
				if( m_finalresult[i].m_selected )
				{
					// 선택된것들 복사
					if(m_finalresult[i].m_type & TYPE_DIRECTORY)
						dobj->AddFile(m_finalresult[i].m_fullpath.c_str());
					else if(m_finalresult[i].m_type & TYPE_FILE)
						dobj->AddFile(m_finalresult[i].m_excutepath.c_str());
					selnum++;
				}
			}

			if(selnum == 0)
			{
				// 현재 커서에 있는것들 복사
				if(m_finalresult[m_selectedpos].m_type & TYPE_DIRECTORY)
				{
					if( m_finalresult[m_selectedpos].m_name != L".." )
						dobj->AddFile(m_finalresult[m_selectedpos].m_fullpath.c_str());
				}
				else if(m_finalresult[m_selectedpos].m_type & TYPE_FILE)
					dobj->AddFile(m_finalresult[m_selectedpos].m_excutepath.c_str());
			}


			wxClipboardLocker locker;
			if ( !locker )
				wxLogError(wxT("클립보드를 열 수 없습니다."));
			else
			{
				if ( !wxTheClipboard->AddData(dobj) )   
				{   
					wxLogError(wxT("Can't copy file(s) to the clipboard"));   
				}   
			}

			ret = true;
			//delete dobj;	// delete 금지
			ClearSelected();
		}
		else if( keycode == 'V' && action == _KEY_DOWN )
		{
			wxArrayString getfiles;
			// paste to clip board
			if (wxTheClipboard->Open())
			{
				if (wxTheClipboard->IsSupported( wxDF_FILENAME ))
				{
					wxFileDataObject data;
					wxTheClipboard->GetData( data );
					getfiles = data.GetFilenames();
					//wxMessageBox( data.GetFilenames()[0] );
				}
				wxTheClipboard->Close();
			}

			if( !getfiles.empty() )
			{
				PasteFiles(getfiles, m_currentDir);
			}
			ret = true;
		}
		else if( keycode == 'A' && action == _KEY_DOWN )
		{
			// 압축 이름 / 압축 수준 / 비번입력 받음
			std::vector<SCANINFO> compresslist;
			GetFileList(compresslist, true);
			CompressFiles(compresslist);
		}
		else if( keycode == 'X' && action == _KEY_DOWN )
		{
			// Uncompress
			//std::vector<SCANINFO> compresslist;
			//GetFileList(compresslist, true);
			//UnCompressFiles(compresslist);

			if(m_finalresult[m_selectedpos].m_type & TYPE_FILE)
			{
				String ext = utility::GetExtention(m_finalresult[m_selectedpos].m_excutepath.c_str());
				if( ext == L".zip" || ext == L".apk" || ext == L".7z" || ext == L".lzh" )
				{
					DeCompressFiles(m_finalresult[m_selectedpos].m_excutepath);
				}
			}
			ClearSelected();

		}
		else if( keycode == 'Z' && action == _KEY_DOWN )
		{
			ChangeAttribute();
		}
		else if( keycode == 'S' && action == _KEY_DOWN )
		{
			DoSVN();
		}
		else if( keycode == 'E' && action == _KEY_DOWN )
		{
			DoExplorer();
		}
		else if( keycode == 'P' && action == _KEY_DOWN )
		{
			// svn commit
			DoSVNCommand(L"commit");
		}
		else if( keycode == 'L' && action == _KEY_DOWN )
		{
			// svn update
			DoSVNCommand(L"update");
		}
	}
	return ret;
}

bool	CDirectoryManager::ActionALTKey(int action, int keycode, int modfier)
{
	bool ret = false;
	if( modfier & wxMOD_ALT )
	{
		if( keycode == 'C' && action == _KEY_DOWN )
		{
			// ALT-C (copy)
			Copy();
			ret = true;
		}
		else if( keycode == 'M' && action == _KEY_DOWN )
		{
			// Alt-M (Move) : Copy 후 delete
			Move();
			ret = true;
		}
		else if( keycode == 'R' && action == _KEY_DOWN )
		{
			// ALT-R (Rename)
			Rename();
			ret = true;
		}
		else if( keycode == 'K' && action == _KEY_DOWN  )
		{
			// ALT-K 폴더만들기
			MakeDirectory();
			ret = true;
		}
		else if( keycode == 'Z' && action == _KEY_DOWN  )
		{
			// ALT-Z 히든보이기 토글
			SetLastSelect();
			if( m_displayoption&_DISPLAY_HIDDEN)
				m_displayoption &= ~_DISPLAY_HIDDEN ;
			else
				m_displayoption |= _DISPLAY_HIDDEN ;

			SortList();
			RestoreLastSelect();
			ret = true;
		}
		else if( keycode == 'E' && action == _KEY_DOWN )
		{
			// ALT-E 이름 순서로 소팅
			SetLastSelect();
			if( m_sortmethod == _SORT_NAME_MIN || m_sortmethod == _SORT_NAME_MAX )
			{
				m_sortmethod = (m_sortmethod == _SORT_NAME_MIN ? _SORT_NAME_MAX : _SORT_NAME_MIN);
			}
			else
				m_sortmethod = _SORT_NAME_MIN;

			SaveSortInfo();
			SortList();
			RestoreLastSelect();

			ret = true;
		}
		else if( keycode == 'T' && action == _KEY_DOWN )
		{
			// 시간 순서로 소팅
			SetLastSelect();
			if( m_sortmethod == _SORT_TIME_MIN || m_sortmethod == _SORT_TIME_MAX )
			{
				m_sortmethod = (m_sortmethod == _SORT_TIME_MIN ? _SORT_TIME_MAX : _SORT_TIME_MIN);
			}
			else
				m_sortmethod = _SORT_TIME_MIN;

			SaveSortInfo();
			SortList();
			RestoreLastSelect();
			ret = true;
		}
		else if( keycode == 'S' && action == _KEY_DOWN )
		{
			// 크기 순서로 소팅
			SetLastSelect();
			if( m_sortmethod == _SORT_SIZE_MIN || m_sortmethod == _SORT_SIZE_MAX )
			{
				m_sortmethod = (m_sortmethod == _SORT_SIZE_MIN ? _SORT_SIZE_MAX : _SORT_SIZE_MIN);
			}
			else
				m_sortmethod = _SORT_SIZE_MIN;

			SaveSortInfo();
			SortList();
			RestoreLastSelect();
			ret = true;
		}
		else if( keycode == 'P' && action == _KEY_DOWN )
		{
			// 이미지 프리뷰 on/off
			String imagepreview = ConfigManager::GetInstance()->GetCommonOption(L"imagepreview");
			if (imagepreview == L"true")
			{
				ConfigManager::GetInstance()->SetCommonOptionBool(L"imagepreview", false);
			}
			else
			{
				ConfigManager::GetInstance()->SetCommonOptionBool(L"imagepreview", true);
			}

			ConfigManager::GetInstance()->Save();
		}
	}

	return ret;
}

// 퀵서치
void	CDirectoryManager::DoQuickSearch()
{
	quicksearchlist.clear();
	for (size_t t = 0; t < m_finalresult.size(); t++)
	{
		String str = m_finalresult[t].m_name.c_str();
		transform(str.begin(), str.end(), str.begin(), toupper);
		
		size_t i = 0;
		for(i = 0; i<quicksearch.size(); i++)
		{
			if( quicksearch[i] != str[i] )
				break;
		}

		if( i == quicksearch.size() )
		{
			SIMPLEINFO info;
			info.type = m_finalresult[t].m_type;
			info.name = m_finalresult[t].m_name;
			info.ext = m_finalresult[t].m_ext;
			quicksearchlist.push_back(info);
		}
	}

	if( !quicksearchlist.empty() )
	{
		lastselectinfo.type = quicksearchlist.front().type;
		lastselectinfo.name = quicksearchlist.front().name;
		lastselectinfo.ext = quicksearchlist.front().ext;

		RestoreLastSelect();
	}
}

void	CDirectoryManager::RollQuickSearchFront()
{
	if( !quicksearchlist.empty() )
	{	
		SIMPLEINFO info;
		info.type = quicksearchlist.front().type;
		info.name = quicksearchlist.front().name;
		info.ext = quicksearchlist.front().ext;

		std::list<SIMPLEINFO>::iterator front = quicksearchlist.begin();
		quicksearchlist.pop_front();
		quicksearchlist.push_back(info);

		lastselectinfo.type = quicksearchlist.front().type;
		lastselectinfo.name = quicksearchlist.front().name;
		lastselectinfo.ext = quicksearchlist.front().ext;
		RestoreLastSelect();
	}
}

void	CDirectoryManager::RollQuickSearchBack()
{
	if( !quicksearchlist.empty() )
	{	
		SIMPLEINFO info;
		info.type = quicksearchlist.back().type;
		info.name = quicksearchlist.back().name;
		info.ext = quicksearchlist.back().ext;

		std::list<SIMPLEINFO>::iterator front = quicksearchlist.begin();
		quicksearchlist.pop_back();
		quicksearchlist.push_front(info);

		lastselectinfo.type = quicksearchlist.front().type;
		lastselectinfo.name = quicksearchlist.front().name;
		lastselectinfo.ext = quicksearchlist.front().ext;
		RestoreLastSelect();
	}
}

void	CDirectoryManager::BackToRoot()
{
	String drive = utility::GetDrive(m_currentDir.c_str());
	m_currentDir = drive;
	m_currentDir = CheckValidDirectory(m_currentDir);

	String k = String(m_currentDir, 0, 1);
	transform(k.begin(), k.end(), k.begin(), toupper);
	pathinfo_ini->Set(k.c_str(), m_currentDir.c_str());

	// last drive 갱신
	std::wstring drivestr = std::wstring(L"LASTDRIVE") + ( whatwindow == _LEFT_WINDOW ? L"_LEFT" : L"_RIGHT");
	pathinfo_ini->Set( drivestr.c_str(), m_currentDir.c_str());

	CollectCurrentDirInfo();
	SavePathinfo();

	//filechecker->Reset( m_currentDir.c_str() );
	filemonitor->Change(m_currentDir.c_str());

	SelectImage();
}


void	CDirectoryManager::MoveToBackDir()
{
	String lastdir = m_currentDir;
	// 이전 dir을 얻어낸다.
	int i = m_currentDir.find_last_of(L"\\");
	if( i < 0 ) return;

	String t = String(m_currentDir, 0, i);
	m_currentDir = t;
	m_currentDir = CheckValidDirectory(m_currentDir);

	String k = String(m_currentDir, 0, 1);
	transform(k.begin(), k.end(), k.begin(), toupper);
	pathinfo_ini->Set(k.c_str(), m_currentDir.c_str());

	// last drive 갱신
	std::wstring drivestr = std::wstring(L"LASTDRIVE") + ( whatwindow == _LEFT_WINDOW ? L"_LEFT" : L"_RIGHT");
	pathinfo_ini->Set( drivestr.c_str(), m_currentDir.c_str());

	CollectCurrentDirInfo();
	SavePathinfo();

	ChooseBestPos(lastdir);

	//filechecker->Reset( m_currentDir.c_str() );
	filemonitor->Change(m_currentDir.c_str());

	SelectImage();
}

void	CDirectoryManager::ChooseBestPos(String lastdir)
{
	// 적당한 m_displaytop / m_selectedpos 표시 
	for(int i=0; i<(int)m_finalresult.size(); i++)
	{
		if( m_finalresult[i].m_type == TYPE_DIRECTORY &&  m_finalresult[i].m_fullpath == lastdir )
		{
			m_displaytop = (i / m_displaylines) * m_displaylines;
			m_selectedpos = i;
			break;
		}
	}
}

void	CDirectoryManager::RenderContents(wxPoint p, wxSize size, Canvas *canvas)
{
	filecheckerlock.Enter();

	DrawDirinfo(p, size, canvas);
	DrawFile(p, size, canvas);
	DrawFileinfo(p, size, canvas);
	DrawImage(p, size, canvas);

	filecheckerlock.Leave();
}

void	CDirectoryManager::DrawDirinfo(wxPoint p, wxSize size, Canvas *canvas)
{
	std::vector<String> pathlist;
	String temp = m_currentDir;

	int l = 0;
	int i=0;
	for (; i<(int)temp.size(); i++)
	{
		if( temp[i] == '\\' )
		{
			String t = String(temp, i-l, l);
			transform(t.begin(), t.end(), t.begin(), toupper);
			pathlist.push_back(t);
			l = 0;
		}
		else
		{
			l++;
		}
	}

	String t = String(temp, i-l, l);
	transform(t.begin(), t.end(), t.begin(), toupper);
	pathlist.push_back(t);

	temp.clear();
	for (int i=0; i<(int)pathlist.size(); i++)
	{
		if( i > 0 )
			temp += L" > ";
		temp += pathlist[i];
	}

	transform(temp.begin(), temp.end(), temp.begin(), toupper);

	// 현재 디렉토리의 정보를 상단에 출력
	unsigned long topinfobgcolor = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"TOPBG");

	unsigned long topfont = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"TOPFONT");
	unsigned long topfontbg = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"TOPFONTBG");
	unsigned long topfontoutline = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"TOPFONTOUTLINE");


	canvas->DrawRect(p.x+0, 0, (size.x / 2), LINE_HEIGHT, topinfobgcolor, false);
	//canvas->Print(p.x+5, 0, temp.c_str(), 0xffffffff, false, SHELL_FONTSIZE, 500, 20);

	int helpx = p.x + 3;
	for(size_t i=0; i < pathlist.size(); i++ )
	{
		wxSize s = canvas->GetTextSize(pathlist[i].c_str(), SHELL_FONTSIZE);
		canvas->DrawRect(helpx - 2, 0, s.x + 4, LINE_HEIGHT, topfontbg, false);
		canvas->DrawRect(helpx - 2, 0, s.x + 4, LINE_HEIGHT, topfontoutline, true);
		canvas->Print(helpx, 0, pathlist[i].c_str(), topfont, false, SHELL_FONTSIZE, 500, 20);	
		helpx += s.x + 5;
	}

	canvas->DrawLine(p.x+0, LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, 0xffa0a0a0, 1);

	// TODO. Quick Search
	if( !quicksearch.empty() )
	{
		int half = (size.x / 2);
		wxSize s = canvas->GetTextSize(quicksearch.c_str(), SHELL_FONTSIZE);
		canvas->DrawRect(p.x + half - s.x - 5, 0, s.x, LINE_HEIGHT, 0xff00909F, false);
		canvas->Print(p.x + half - s.x- 5, 0, quicksearch.c_str(), 0xffffffff, false, SHELL_FONTSIZE);

	}
}

void	CDirectoryManager::DrawFileinfo(wxPoint p, wxSize size, Canvas *canvas)
{
	unsigned long color = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"BOTTOMBG");

	unsigned long fontcolor = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"BOTTOMFONT");
	unsigned long fontbgcolor = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"BOTTOMFONTBG");
	unsigned long fontoutlinecolor = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"BOTTOMFONTOUTLINE");

	std::vector<String>	helplist;

	if( keymodifyer & wxMOD_ALT )
	{
		helplist.push_back(String(L"C : 복사"));
		helplist.push_back(String(L"K : 폴더 만들기"));
		helplist.push_back(String(L"M : 이동"));
		helplist.push_back(String(L"R : 이름 바꾸기"));
		helplist.push_back(String(L"Z : 숨김 표시"));
		//color = fontcolor;
	}
	else if( keymodifyer & wxMOD_CONTROL )
	{
		helplist.push_back(String(L"A : 압축"));
		helplist.push_back(String(L"C : 클립보드 복사"));
		helplist.push_back(String(L"V : 클립보드 붙이기"));
		helplist.push_back(String(L"X : 압축 풀기"));
		helplist.push_back(String(L"Z : 속성 편집"));
		//color = fontcolor;
	}
	else if( keymodifyer & wxMOD_SHIFT)
	{
		helplist.push_back(String(L"A ~ Z : 드라이브 이동"));
		//color = fontcolor;
	}
	else
	{
		if( m_selectedpos > -1 )
		{
			int dirselnum = 0;
			int fileselnum = 0;
			for(int i=0; i<(int)m_finalresult.size(); i++)
			{
				if( m_finalresult[i].m_selected )
				{
					if( m_finalresult[i].m_type == TYPE_DIRECTORY) 
						dirselnum++;
					else if( m_finalresult[i].m_type == TYPE_FILE) 
						fileselnum++;
				}
			}

			String str;
			if( dirselnum > 0 || fileselnum > 0 )
			{
				if( dirselnum > 0 )
					str = unicode::format(L"폴더 %d ", dirselnum);
				if( fileselnum > 0 )
					str += unicode::format(L"파일 %d ", fileselnum);
				str += L"선택됨";

				helplist.push_back( str );
			}

			String attribute;
			attribute += m_finalresult[m_selectedpos].m_attribute & ATTR_ARCHIVE ? L"A" : L".";
			attribute += m_finalresult[m_selectedpos].m_attribute & ATTR_READONLY ? L"R" : L".";
			attribute += m_finalresult[m_selectedpos].m_attribute & ATTR_HIDDEN ? L"H" : L".";
			attribute += m_finalresult[m_selectedpos].m_attribute & ATTR_SYSTEM ? L"S" : L".";

			FILETIME lft;  
			FileTimeToLocalFileTime(&m_finalresult[m_selectedpos].m_LastAccessTime, &lft);   // 로컬파일타임으로 조정

			SYSTEMTIME st;
			FileTimeToSystemTime(&lft, &st);
			String date = unicode::format(L"%04u-%02u-%02u %02u:%02u:%02u",
				st.wYear, st.wMonth, st.wDay,
				st.wHour,st.wMinute, st.wSecond);

			if( m_finalresult[m_selectedpos].m_type == TYPE_DIRECTORY )
			{
				String filename = utility::GetFileName(m_finalresult[m_selectedpos].m_fullpath.c_str());
				helplist.push_back( unicode::format(L"%s", attribute.c_str()) );
				helplist.push_back( unicode::format(L"%s", date.c_str()) );
				helplist.push_back( unicode::format(L"%s", filename.c_str()) );

				if( m_finalresult[m_selectedpos].m_name == L".." )
					helplist.clear();
			}
			else if( m_finalresult[m_selectedpos].m_type == TYPE_FILE )
			{
				String filename = utility::GetFileName(m_finalresult[m_selectedpos].m_excutepath.c_str());
				helplist.push_back( unicode::format(L"%s", utility::getStringSize( m_finalresult[m_selectedpos].m_filesize, 2).c_str()) );
				helplist.push_back( unicode::format(L"%s", attribute.c_str()) );
				helplist.push_back( unicode::format(L"%s", date.c_str()) );
				helplist.push_back( unicode::format(L"%s", filename.c_str()) );
			}
		}
	}

	canvas->DrawRect(p.x + 0, canvas->GetHeight()-LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, color, false);

	unsigned long outlinecolor = fontoutlinecolor;
	int helpx = p.x + 5;
	for(size_t i=0; i < helplist.size(); i++ )
	{
		wxSize s = canvas->GetTextSize(helplist[i].c_str(), SHELL_FONTSIZE);
		canvas->DrawRect(helpx - 2, canvas->GetHeight()-LINE_HEIGHT, s.x + 4, LINE_HEIGHT, fontbgcolor, false);
		canvas->DrawRect(helpx - 2, canvas->GetHeight()-LINE_HEIGHT, s.x + 4, LINE_HEIGHT, outlinecolor, true);
		canvas->Print(helpx, canvas->GetHeight()-LINE_HEIGHT, helplist[i].c_str(), fontcolor, false, SHELL_FONTSIZE, 500, 20);	
		helpx += s.x + 6;
	}
}


void	CDirectoryManager::DrawFile(wxPoint p, wxSize size, Canvas *c)
{
	filecheckerlock.Enter();

	size_t nstring = m_finalresult.size();

	CalcDisplayInfo();

	// 이름, 확장자, 속성, 크기
	struct _DISP
	{
		bool display;
		int posx;
		int clipwidth;
	} dispclip[4] = {false,0,0, false,0,0, false,0,0, false,0,0};

	const int extgap = 5;
	const int extwidth = 40;

	const int skipselect = 20;	// 맨앞 선택 표시 크기
	const int min_width = 350;
	const int canvaswidth = (size.x/2);

	if( canvaswidth <= min_width + skipselect )
	{
		// 너무 작어! 이름만 출력
		dispclip[0].display = true;
		dispclip[0].posx = skipselect + p.x;
		dispclip[0].clipwidth = canvaswidth - skipselect;

	}
	else
	{
		int pp=1;
		for (int i=3;i>0; i--)
		{
			int g = (extwidth + extgap) * (i);
			if(canvaswidth - g > min_width + skipselect)
			{
				dispclip[i].display = true;
				dispclip[i].posx = canvaswidth - (extwidth + extgap) * (pp++)  + p.x;
				dispclip[i].clipwidth = (extwidth + extgap);
			}
		}

		dispclip[0].display = true;
		dispclip[0].posx = skipselect + p.x;
		dispclip[0].clipwidth = (size.x/2) - skipselect - (extwidth + extgap) * (pp-1) - extgap;
	}

	//for (int i=m_displaytop ; i<m_displaylines; i++)
	for (int i=0 ; i<m_displaylines; i++)
	{
		if( m_displaytop + i >= (int)m_finalresult.size() )
		{
			break;
		}
		int pos = m_displaytop + i;

		switch(m_finalresult[pos].m_type)
		{
			case TYPE_DRIVE :
				{
					unsigned long color = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"DRIVE");
					unsigned long sizecolor = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"FILESIZE");

					String totalspace, freespace;	// GB
					float percent = utility::GetDriveSpace(m_finalresult[pos].m_fullpath, totalspace, freespace);

					wstring name = m_finalresult[pos].m_name + L" [ " + m_finalresult[pos].m_ext + L" ]";

					if( m_selectedpos == pos )
					{
						unsigned long darkcolor = color & ColorManager::GetInstance()->GetColor(EXT_COLOR, L"DARK");
						unsigned long sizedarkcolor = sizecolor & ColorManager::GetInstance()->GetColor(EXT_COLOR, L"DARK");

						if(activewindow)
							c->DrawRect(p.x, i * LINE_HEIGHT + LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, color, false);
						else
						{
							c->DrawRect(p.x + 0, i * LINE_HEIGHT + LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, color & 0x20202020, false);
							c->DrawRect(p.x + 0, i * LINE_HEIGHT + LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, color & 0xFFA0A0A0, true);
							darkcolor = color & 0xFFA0A0A0;
							sizedarkcolor = sizecolor & 0xFFA0A0A0;
						}

						if( m_finalresult[pos].m_ext == L"DISK" || m_finalresult[pos].m_ext == L"REMOVABLE")
						{
							int yp = i * LINE_HEIGHT + LINE_HEIGHT;
							wxSize iw = c->GetTextSize(name.c_str(), SHELL_FONTSIZE);
							c->Print(dispclip[0].posx, yp, name.c_str(), darkcolor, false, SHELL_FONTSIZE, dispclip[0].clipwidth, 20);

							String str = L"(" + freespace + L"/" + totalspace + L")";
							wxSize sw = c->GetTextSize(str.c_str(), SHELL_FONTSIZE);
							c->Print(dispclip[0].posx + iw.x, yp, str.c_str(), sizedarkcolor, false, SHELL_FONTSIZE, dispclip[0].clipwidth-iw.x, 20);

							utility::DrawProgressbar(c, dispclip[0].posx + iw.x + sw.x + 10, yp+3, 50, 9, percent, true, dispclip[0].clipwidth-iw.x-sw.x- 10, 20);
						}
						else
							c->Print(dispclip[0].posx, i * LINE_HEIGHT + LINE_HEIGHT, name.c_str(), darkcolor, false, SHELL_FONTSIZE, dispclip[0].clipwidth, 20);

						m_currentdisplaypos = i;
					}
					else
					{
						if( m_finalresult[pos].m_ext == L"DISK" || m_finalresult[pos].m_ext == L"REMOVABLE")
						{
							int yp = i * LINE_HEIGHT + LINE_HEIGHT;
							wxSize iw = c->GetTextSize(name.c_str(), SHELL_FONTSIZE);
							c->Print(dispclip[0].posx, yp, name.c_str(), color, false, SHELL_FONTSIZE, dispclip[0].clipwidth, 20);

							String str = L"(" + freespace + L"/" + totalspace + L")";
							wxSize sw = c->GetTextSize(str.c_str(), SHELL_FONTSIZE);
							c->Print(dispclip[0].posx + iw.x, yp, str.c_str(), sizecolor, false, SHELL_FONTSIZE, dispclip[0].clipwidth-iw.x, 20);
						
							utility::DrawProgressbar(c, dispclip[0].posx + iw.x + sw.x + 10, yp+3, 50, 9, percent, false, dispclip[0].clipwidth-iw.x-sw.x- 10, 20);
						}
						else
							c->Print(dispclip[0].posx, i * LINE_HEIGHT + LINE_HEIGHT, name.c_str(), color, false, SHELL_FONTSIZE, dispclip[0].clipwidth, 20);
					}

				}
				break;

			case TYPE_DIRECTORY :
				{
					unsigned long color = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"FOLDER");
					unsigned long attrcolor = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"FILEATTR");

					String name = m_finalresult[pos].m_name;
					String ext = m_finalresult[pos].m_ext;

					if( m_displayoption & _DISPLAY_CAPITAL )
					{		
						transform(name.begin(), name.end(), name.begin(), toupper);
						transform(ext.begin(), ext.end(), ext.begin(), toupper);
					}

					if( m_selectedpos == pos )
					{
						unsigned long darkcolor = color & ColorManager::GetInstance()->GetColor(EXT_COLOR, L"DARK");
						unsigned long darkattrcolor = attrcolor & ColorManager::GetInstance()->GetColor(EXT_COLOR, L"DARK");
						

						if(activewindow)
							c->DrawRect(p.x + 0, i * LINE_HEIGHT + LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, color, false);
						else
						{
							c->DrawRect(p.x + 0, i * LINE_HEIGHT + LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, color & 0x20202020, false);
							c->DrawRect(p.x + 0, i * LINE_HEIGHT + LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, color & 0xFFA0A0A0, true);
							darkcolor = color & 0xFFA0A0A0;
						}

						c->Print(dispclip[0].posx, i * LINE_HEIGHT + LINE_HEIGHT, name.c_str(), darkcolor, false, SHELL_FONTSIZE, dispclip[0].clipwidth, 20);
						if( dispclip[1].display )
							c->Print(dispclip[1].posx, i * LINE_HEIGHT + LINE_HEIGHT, ext.c_str(), darkcolor, false, SHELL_FONTSIZE, dispclip[1].clipwidth, 20);

						if(m_finalresult[pos].m_name != L".." && dispclip[2].posx)
						{
							c->Print(dispclip[2].posx, i * LINE_HEIGHT + LINE_HEIGHT, utility::getStringAttribute(m_finalresult[pos].m_attribute).c_str(), darkattrcolor, false, SHELL_FONTSIZE, dispclip[2].clipwidth, 20);
						}
						m_currentdisplaypos = i;
					}
					else
					{
						c->Print(dispclip[0].posx, i * LINE_HEIGHT + LINE_HEIGHT, name.c_str(), color, false, SHELL_FONTSIZE, dispclip[0].clipwidth, 20);
						if( dispclip[1].display )
							c->Print(dispclip[1].posx, i * LINE_HEIGHT + LINE_HEIGHT, ext.c_str(), color, false, SHELL_FONTSIZE, dispclip[1].clipwidth, 20);

						if(m_finalresult[pos].m_name != L".." && dispclip[2].display)
							c->Print(dispclip[2].posx, i * LINE_HEIGHT + LINE_HEIGHT, utility::getStringAttribute(m_finalresult[pos].m_attribute).c_str(), attrcolor, false, SHELL_FONTSIZE, dispclip[2].clipwidth, 20);
					}

					if(lastselectline != -1 && lastselectline == pos)
					{
						if(activewindow)
							c->DrawRect(p.x, i * LINE_HEIGHT + LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, 0xff00ff00, true);
					}
				}
				break;

			case TYPE_FILE :
				{
					String name = m_finalresult[pos].m_name;
					String ext = m_finalresult[pos].m_ext;
					String extstr = m_finalresult[pos].m_ext;

					if( m_displayoption & _DISPLAY_CAPITAL )
					{		
						transform(name.begin(), name.end(), name.begin(), toupper);
						transform(ext.begin(), ext.end(), ext.begin(), toupper);
					}

					transform(extstr.begin(), extstr.end(), extstr.begin(), toupper);

					unsigned long color = ColorManager::GetInstance()->GetColor(EXT_COLOR, extstr);
					unsigned long attrcolor = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"FILEATTR");
					unsigned long sizecolor = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"FILESIZE");

					if( m_selectedpos == pos )
					{
						unsigned long darkcolor = color & ColorManager::GetInstance()->GetColor(EXT_COLOR, L"DARK");
						unsigned long darkattrcolor = attrcolor & ColorManager::GetInstance()->GetColor(EXT_COLOR, L"DARK");
						

						if( activewindow )
							c->DrawRect(p.x + 0, i * LINE_HEIGHT + LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, color, false);
						else
						{
							c->DrawRect(p.x + 0, i * LINE_HEIGHT + LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, color & 0x20202020, false);
							c->DrawRect(p.x + 0, i * LINE_HEIGHT + LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, color & 0xFFa0a0a0, true);

							darkcolor = color & 0xa0a0a0a0;
						}

						c->Print(dispclip[0].posx, i * LINE_HEIGHT + LINE_HEIGHT, name.c_str(), darkcolor, false, SHELL_FONTSIZE, dispclip[0].clipwidth, 20);
						if( dispclip[1].display )
							c->Print(dispclip[1].posx, i * LINE_HEIGHT + LINE_HEIGHT, utility::getShortStr(ext,4).c_str(), darkcolor, false, SHELL_FONTSIZE, dispclip[1].clipwidth, 20);
						if( dispclip[2].display )
							c->Print(dispclip[2].posx, i * LINE_HEIGHT + LINE_HEIGHT, utility::getStringAttribute(m_finalresult[pos].m_attribute).c_str(), darkattrcolor, false, SHELL_FONTSIZE, dispclip[2].clipwidth, 20);
						if( dispclip[3].display )
							c->Print(dispclip[3].posx, i * LINE_HEIGHT + LINE_HEIGHT, utility::getStringSize(m_finalresult[pos].m_filesize,1,true).c_str(), darkcolor, false, SHELL_FONTSIZE, dispclip[3].clipwidth, 20);
						m_currentdisplaypos = i;
					}
					else
					{
						c->Print(dispclip[0].posx, i * LINE_HEIGHT + LINE_HEIGHT, name.c_str(), color, false, SHELL_FONTSIZE, dispclip[0].clipwidth, 20);
						if( dispclip[1].display )
							c->Print(dispclip[1].posx, i * LINE_HEIGHT + LINE_HEIGHT, utility::getShortStr(ext,4).c_str(), color, false, SHELL_FONTSIZE, dispclip[1].clipwidth, 20);
						if( dispclip[2].display )
							c->Print(dispclip[2].posx, i * LINE_HEIGHT + LINE_HEIGHT, utility::getStringAttribute(m_finalresult[pos].m_attribute).c_str(), attrcolor, false, SHELL_FONTSIZE, dispclip[2].clipwidth, 20);
						if( dispclip[3].display )
							c->Print(dispclip[3].posx, i * LINE_HEIGHT + LINE_HEIGHT, utility::getStringSize(m_finalresult[pos].m_filesize,1,true).c_str(), sizecolor, false, SHELL_FONTSIZE, dispclip[3].clipwidth, 20);

					}

					if(lastselectline != -1 && lastselectline == pos)
					{
						if(activewindow)
							c->DrawRect(p.x, i * LINE_HEIGHT + LINE_HEIGHT, (size.x / 2), LINE_HEIGHT, 0xff00ff00, true);
					}
				}
				break;
		}

		if( m_finalresult[pos].m_selected )
		{
			c->DrawBitmap(p.x + 0, i * LINE_HEIGHT + LINE_HEIGHT, *selectbitmap, true);
		}

	}

// 	wchar_t temp[64];
// 	unicode::sprintf(temp, _countof(temp), _T("%d"), m_displaylines);
// 	c->Print(450, 0, temp, 0xff00ffff, false, SHELL_FONTSIZE);

	filecheckerlock.Leave();
}

void	CDirectoryManager::DrawImage(wxPoint p, wxSize size, Canvas *c)
{
	String imagepreview = ConfigManager::GetInstance()->GetCommonOption(L"imagepreview");

	if(activewindow == false && imagepreview == L"true")
	{
		wxPoint np = whatwindow == _LEFT_WINDOW ? wxPoint(size.x/2,0) : wxPoint(0,0);
		ImageLoader::GetInstance()->renderimage(np, size, c);
	}
}


// 중간 분리 선
void	CDirectoryManager::DrawSeparate(int xpos, wxSize size, Canvas *canvas)
{
	unsigned long color = 0xffffffff;
	//canvas->DrawRect((size.x / 2) - (SEPARATE_WIDTH/2), 0, SEPARATE_WIDTH, size.y, color, false);
	canvas->DrawRect(xpos- (SEPARATE_WIDTH/2), 0, SEPARATE_WIDTH, size.y, color, false);
}

void	CDirectoryManager::SetWindowActive(bool t) 
{ 
	activewindow = t;
	if(activewindow == false)
	{
		keymodifyer = 0;
	}
}

void	CDirectoryManager::CalcDisplayInfo()
{
	int nstring = (int)m_finalresult.size();
	int h = m_windowSize.y - LINE_HEIGHT - LINE_HEIGHT;		// 상단 여백 , 하단 여백

	m_displaylines = h/LINE_HEIGHT;
/*
	if( nstring - m_displaytop >= (h/LINE_HEIGHT) )
	{
		m_displaylines = h/LINE_HEIGHT;
		//_TRACE(_T("1> %d\n"),m_displaylines);
	}
	else
	{
		m_displaylines = nstring;
		//_TRACE(_T("2> %d\n"),m_displaylines);
	}
*/
}


void	CDirectoryManager::SavePathinfo()
{
//	pathinfo_ini.Set(L"C", m_currentDir.c_str());
	pathinfo_ini->SaveINI(L"pathinfo.ini");
}

void	CDirectoryManager::ChangeDrive(const wchar *drive)
{

	// 변경할 드라이브 정보가 있는가
	String key = String(drive, 0,1);
	transform(key.begin(), key.end(), key.begin(), toupper);
	const wchar *newpath = pathinfo_ini->GetString(key.c_str());

	// TODO. 이동가능? , 마지막 드라이브 갱신
	m_currentDir = newpath;
	CollectCurrentDirInfo();

	std::wstring drivestr = std::wstring(L"LASTDRIVE") + ( whatwindow == _LEFT_WINDOW ? L"_LEFT" : L"_RIGHT");
	pathinfo_ini->Set(drivestr.c_str(), m_currentDir.c_str());

	SavePathinfo();
	//filechecker->Reset( m_currentDir.c_str() );
	filemonitor->Change(m_currentDir.c_str());
}

//////////////////////////////////////////////////////////////////////////

// 파일이나 폴더 삭제
void	CDirectoryManager::Deletefile(bool recyclebin)
{
	filecheckerlock.Enter();

	if( m_finalresult[m_selectedpos].m_type & TYPE_FILE )
	{
		// 파일 한개 삭제
		utility::removefile(m_finalresult[m_selectedpos].m_excutepath.c_str(), recyclebin);
	}
	else if( m_finalresult[m_selectedpos].m_type & TYPE_DIRECTORY  )
	{
		int totalnum = 0;
		int progress = 0;
		std::vector<SCANINFO> scaninfolist;
		Scandirectory(m_finalresult[m_selectedpos].m_fullpath.c_str(), totalnum, scaninfolist);

		totalnum++;

		// 디렉토리 삭제
		DeleteDialog dialog(parentwindow);

		bool ret = DeleteDirectory(m_finalresult[m_selectedpos].m_fullpath.c_str(), &dialog, recyclebin, totalnum, progress);
		dialog.Update(L"path", m_finalresult[m_selectedpos].m_fullpath);

		if( ret )
			utility::removedirectoty(m_finalresult[m_selectedpos].m_fullpath.c_str(), recyclebin);

//		_wrmdir(m_finalresult[m_selectedpos].m_fullpath.c_str());
		dialog.Destroy();

	}

	SetLastSelectInPos(m_selectedpos+1);
//	CollectCurrentDirInfo();
	filecheckerlock.Leave();
}


// 디렉토리 삭제
bool CDirectoryManager::DeleteDirectory(const wchar *lpFolder, DeleteDialog *dialog, bool recyclebin, int totalnum, int &progress) 
{ 
	bool ret = true;
	WIN32_FIND_DATA FindData; 
	HANDLE FindHandle; 

	wchar temp1[MAX_PATH]; 
	int lpFolder_Len;

	if(lpFolder==NULL || *lpFolder=='\0' ) 
		return ret; 
		
	lpFolder_Len = unicode::strlen(lpFolder); 
	if(lpFolder[lpFolder_Len-1]=='\\') 
		unicode::sprintf(temp1, _countof(temp1), L"%s*.*", lpFolder); 
	else 
		unicode::sprintf(temp1, _countof(temp1), L"%s\\*.*", lpFolder); 
	
	FindHandle=FindFirstFile(temp1, &FindData); 
	if(INVALID_HANDLE_VALUE!=FindHandle) 
	{ 
		while(1) 
		{ 
			if(lpFolder[lpFolder_Len-1]=='\\') 
			{ 
				unicode::sprintf(temp1, _countof(temp1), L"%s%s", lpFolder, FindData.cFileName); 
			} 
			else 
			{ 
				unicode::sprintf(temp1, _countof(temp1), L"%s\\%s", lpFolder, FindData.cFileName); 
			} 
			
			if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			{ 
				//Dir 
				if(unicode::strcmp(FindData.cFileName, L".")==0) 
				{ 
					//. -->> skip 
				} 
				else if(unicode::strcmp(FindData.cFileName, L"..")==0) 
				{ 
					//.. -->> skip 
				} 
				else 
				{ 
					// 디렉토리이므로 디렉토리 탐색
					if( DeleteDirectory(temp1, dialog, recyclebin, totalnum, progress) == false )
					{
						ret = false;
						break;
					}

					progress++;
					dialog->Update(L"path", temp1);
					dialog->Update(L"totalnum", unicode::format(L"(%d/%d)", progress, totalnum));

					int p = (int)((float)progress/(float)totalnum * 100.f);
					dialog->Update(L"progress", unicode::format(L"%d", p));

					if( dialog->IsCancel() )
					{
						ret = false;
						break;
					}

					// 디렉토리 삭제
					//utility::removefile(temp1, recyclebin);
					utility::removedirectoty(temp1, recyclebin);
				} 
			} 
			else 
			{ 
				//File 삭제
				progress++;
				String str = utility::getShortStrMid(temp1, 70);
				dialog->Update(L"path", str);
				dialog->Update(L"totalnum", unicode::format(L"(%d/%d)", progress, totalnum));
				int p = (int)((float)progress/(float)totalnum * 100.f);
				dialog->Update(L"progress", unicode::format(L"%d", p));

				if( dialog->IsCancel() )
				{
					ret = false;
					break;
				}

				utility::removefile(temp1, recyclebin);
			} 

			if(!FindNextFile(FindHandle, &FindData)) 
			{ 
				break;
			}
		}
	}

	FindClose(FindHandle); 
	return ret;
}

bool CDirectoryManager::Scandirectory(const wchar *path, int &count, std::vector<SCANINFO> &scaninfolist, bool showdialog)
{
	ScanDialog *dialog = NULL;
	if(showdialog)
	{
		dialog = new ScanDialog(parentwindow, L"분석중", L"경로를 분석중...");
	}

	bool ret = _Scandirectory(path, count, scaninfolist, dialog);
	if(showdialog && dialog)
	{
		dialog->Destroy();
		delete dialog;
	}

	return ret;
}


// 디렉토리내의 파일 / 디렉토리 갯수
bool		CDirectoryManager::_Scandirectory(const wchar *path, int &count, std::vector<SCANINFO> &scaninfolist, ScanDialog *dialog)
{
	WIN32_FIND_DATA FindData; 
	HANDLE FindHandle; 

	wchar temp1[MAX_PATH]; 
	int lpFolder_Len;

	if( path==NULL || *path=='\0' ) 
		return true; 

	lpFolder_Len = unicode::strlen(path); 
	if(path[lpFolder_Len-1]=='\\') 
		unicode::sprintf(temp1, _countof(temp1), L"%s*.*", path); 
	else 
		unicode::sprintf(temp1, _countof(temp1), L"%s\\*.*", path); 

	FindHandle=FindFirstFile(temp1, &FindData); 
	if(INVALID_HANDLE_VALUE!=FindHandle) 
	{ 
		while(1) 
		{ 
			if(path[lpFolder_Len-1]=='\\') 
			{ 
				unicode::sprintf(temp1, _countof(temp1), L"%s%s", path, FindData.cFileName); 
			} 
			else 
			{ 
				unicode::sprintf(temp1, _countof(temp1), L"%s\\%s", path, FindData.cFileName); 
			} 

			if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			{ 
				//Dir 
				if(unicode::strcmp(FindData.cFileName, L".")==0) 
				{ 
					//. -->> skip 
				} 
				else if(unicode::strcmp(FindData.cFileName, L"..")==0) 
				{ 
					//.. -->> skip 
				} 
				else 
				{ 
					// 디렉토리이므로 하위 디렉토리 탐색
// 					if(showdialog && dialog)
// 						dialog->Update(temp1);

					SCANINFO info;
					info.type = TYPE_DIRECTORY;
					info.path = temp1;
					scaninfolist.push_back(info);
					bool cancel = _Scandirectory(temp1, count, scaninfolist, dialog);
					if( cancel ) return true;
					count++;
				} 
			} 
			else 
			{ 
				if(dialog)
				{
					String str = utility::getShortStrMid(temp1, 80);
					dialog->Update(String(L"name"), str.c_str());
					String n = unicode::Format(L"검색파일: %d",count);
					dialog->Update(String(L"count"), n.c_str());
					bool cancel = dialog->IsCancel();
					if( cancel ) return true;
				}

				//File
				SCANINFO info;
				info.type = TYPE_FILE;
				info.path = temp1;
				scaninfolist.push_back(info);
				count++;
			} 

			if(!FindNextFile(FindHandle, &FindData)) 
			{ 
				break;
			}
		}
	}

	FindClose(FindHandle); 
	return false;
}


// 선택된 여러 파일/폴더 삭제
void	CDirectoryManager::DeleteSelectedFile(bool recyclebin)
{
	filecheckerlock.Enter();
	DeleteDialog dialog(parentwindow);

	int progress = 0;
	int totalnum = 0;

	// 파일 갯수 카운트
	for(int i=0; i<(int)m_finalresult.size(); i++)
	{
		if( m_finalresult[i].m_selected )
		{
			if( m_finalresult[i].m_type & TYPE_DIRECTORY )
			{
				std::vector<SCANINFO> scaninfolist;
				Scandirectory( m_finalresult[i].m_fullpath.c_str(), totalnum , scaninfolist);
				totalnum++;
			}
			else if( m_finalresult[i].m_type & TYPE_FILE )
				totalnum++;
		}
	}

	// 실제로 삭제
	for(int i=0; i<(int)m_finalresult.size(); i++)
	{
		if( m_finalresult[i].m_selected )
		{
			progress++;
			// 디렉토리 삭제
			if( m_finalresult[i].m_type & TYPE_DIRECTORY )
			{
				bool ret = DeleteDirectory(m_finalresult[i].m_fullpath.c_str(), &dialog, recyclebin, totalnum, progress);
				if( ret == false )
					break;
				utility::removedirectoty(m_finalresult[i].m_fullpath.c_str(), recyclebin);
			}
			else if( m_finalresult[i].m_type & TYPE_FILE )
			{
				utility::removefile(m_finalresult[i].m_excutepath.c_str(), recyclebin);
			}

			String str = utility::getShortStrMid(m_finalresult[i].m_excutepath.c_str(), 80);
			dialog.Update(L"path", str);
			dialog.Update(L"totalnum", unicode::format(L"(%d/%d)", progress, totalnum));

			int p = (int)((float)progress/(float)totalnum * 100.f);
			dialog.Update(L"progress", unicode::format(L"%d", p));

			if( dialog.IsCancel() )
				break;
		}
	}

	dialog.Destroy();
	filecheckerlock.Leave();

	ClearSelected();
	SetLastSelect();
}


// 폴더 만들기
void	CDirectoryManager::MakeDirectory()
{
	// TODO. 기존에 같은 폴더있나 검사 / 폴더 생성후 화면에 보여줄것

  	MakeDirDialog *dialog = new MakeDirDialog(parentwindow, wxT("폴더 만들기"));
  	dialog->ShowModal();
	//delete dialog;

	String ret = Valueman::GetInstance()->getvalue(L"makedir");
	if( !ret.empty() )
	{
		Valueman::GetInstance()->remove(L"makedir");

		String capitalret = ret;
		transform(capitalret.begin(), capitalret.end(), capitalret.begin(), toupper);

		// 같은 이름이 있는가 검사
		for (int i=0; i<(int)m_finalresult.size(); i++)
		{
			if( m_finalresult[i].m_type != TYPE_DIRECTORY ) continue;
			String temp = m_finalresult[i].m_name;
			transform(temp.begin(), temp.end(), temp.begin(), toupper);

			if( temp == capitalret )
			{
				// 이미 같은 이름이 있슴
				wxMessageDialog *dial = new wxMessageDialog(parentwindow, 
					wxT("같은 이름의 폴더가 있습니다."), wxT("오류"), wxOK | wxICON_ERROR);

				dial->ShowModal();
				//delete dial;
				return;
			}
		}

		// 폴더 생성
		String makefolder = m_currentDir + L"\\" + ret;
		int r = _wmkdir(makefolder.c_str());
		if( r == -1 )
		{
			wxMessageDialog *dial = new wxMessageDialog(parentwindow, 
				wxT("폴더를 생성 할 수 없습니다."), wxT("오류"), wxOK | wxICON_ERROR);
			dial->ShowModal();
			//delete dial;
			return;
		}

		// 폴더 생성 성공 리스트 갱신 && 그 위치로 이동
		lastselectinfo.type = TYPE_DIRECTORY;
		lastselectinfo.name = ret;

	}
}

// 선택 / 현재것을 복사
//////////////////////////////////////////////////////////////////////////

void	CDirectoryManager::Copy()
{
	int totalnum = 0;
	std::vector<SCANINFO> scaninfolist;

	String key = L"copydest";
	CDirectoryManager *nextdoor = ((MojoShell*)parentwindow)->GetNextDoor(whatwindow == _LEFT_WINDOW ? _RIGHT_WINDOW : _LEFT_WINDOW);
	String nextdoordir = nextdoor->GetCurrentDir();

	EnterTextDialog *dialog = new EnterTextDialog( parentwindow, L"다른창으로 복사", L"복사경로", nextdoordir.c_str(), key.c_str());
	dialog->ShowModal();
	delete dialog;

	String ret =  Valueman::GetInstance()->getvalue(key.c_str());

	if( !ret.empty() && utility::access(ret.c_str()) == true)
	{
		Valueman::GetInstance()->remove(key.c_str());

		int selnum = 0;
		for(int i=0; i<(int)m_finalresult.size(); i++)
		{
			if( m_finalresult[i].m_selected )
				selnum++;
		}

		if(selnum > 0)
		{
			for(int i=0; i<(int)m_finalresult.size(); i++)
			{
				if( m_finalresult[i].m_selected )
				{
					SCANINFO info;

					if(m_finalresult[i].m_type & TYPE_DIRECTORY)
					{
						info.type = TYPE_DIRECTORY;
						info.path = m_finalresult[i].m_fullpath;
						scaninfolist.push_back(info);

						bool cancel = Scandirectory( m_finalresult[i].m_fullpath.c_str(), totalnum , scaninfolist, true);
						if(cancel)
						{
							ClearSelected();
							return;
						}

					}
					else if(m_finalresult[i].m_type & TYPE_FILE)
					{
						info.type = TYPE_FILE;
						info.path = m_finalresult[i].m_excutepath;

						info.creationtime = m_finalresult[i].m_creationtime;
						info.LastAccessTime = m_finalresult[i].m_LastAccessTime;
						info.LastWriteTime = m_finalresult[i].m_LastWriteTime;

						scaninfolist.push_back(info);
					}
				}
			}

			// 선택된것 복사
			if(!scaninfolist.empty())
			{
				CopyDirectory(ret, scaninfolist);
				ClearSelected();
			}
		}
		else
		{
			// 현재 커서에 있는것 복사
			if(m_finalresult[m_selectedpos].m_type & TYPE_DIRECTORY)
			{
				// 현재 디렉토리 추가
				SCANINFO info;
				info.type = TYPE_DIRECTORY;
				info.path = m_finalresult[m_selectedpos].m_fullpath;
				scaninfolist.push_back(info);

				bool cancel = Scandirectory( m_finalresult[m_selectedpos].m_fullpath.c_str(), totalnum , scaninfolist, true);
				if(cancel) return;

				CopyDirectory(ret, scaninfolist);

			}
			else if(m_finalresult[m_selectedpos].m_type & TYPE_FILE)
			{
				SCANINFO info;
				info.type = TYPE_FILE;
				info.path = m_finalresult[m_selectedpos].m_excutepath;

				info.creationtime = m_finalresult[m_selectedpos].m_creationtime;
				info.LastAccessTime = m_finalresult[m_selectedpos].m_LastAccessTime;
				info.LastWriteTime = m_finalresult[m_selectedpos].m_LastWriteTime;

				scaninfolist.push_back(info);
				CopyDirectory(ret, scaninfolist);
			}
		}
	}
}

void	CDirectoryManager::ClearSelected()
{
	lastselectline = -1;
	for(int i=0; i<(int)m_finalresult.size(); i++)
	{
		m_finalresult[i].m_selected = false;
	}
}

bool sortfunc_name_min( FILEINFO& elem1, FILEINFO& elem2 )
{
	String k0 = elem1.m_name;
	String k1 = elem2.m_name;

	transform(k0.begin(), k0.end(), k0.begin(), toupper);
	transform(k1.begin(), k1.end(), k1.begin(), toupper);

	return k0 > k1;	// 알파벳순
}

bool sortfunc_name_max( FILEINFO& elem1, FILEINFO& elem2 )
{
	String k0 = elem1.m_name;
	String k1 = elem2.m_name;

	transform(k0.begin(), k0.end(), k0.begin(), toupper);
	transform(k1.begin(), k1.end(), k1.begin(), toupper);

	return k0 < k1;	// 알파벳순
}

bool sortfunc_time_min( FILEINFO& elem1, FILEINFO& elem2 )
{
	//long t = CompareFileTime(&elem1.m_creationtime, &elem2.m_creationtime);
	long t = CompareFileTime(&elem1.m_LastWriteTime, &elem2.m_LastWriteTime);
	return t < 0 ? true : false;

	//return elem1.m_creationtime.dwHighDateTime + elem1.m_creationtime.dwLowDateTime < elem2.m_creationtime.dwHighDateTime + elem2.m_creationtime.dwLowDateTime;
}

bool sortfunc_time_max( FILEINFO& elem1, FILEINFO& elem2 )
{
	//long t = CompareFileTime(&elem2.m_creationtime, &elem1.m_creationtime);
	long t = CompareFileTime(&elem2.m_LastWriteTime, &elem1.m_LastWriteTime);
	return t < 0 ? true : false;

	//return elem1.m_creationtime.dwHighDateTime + elem1.m_creationtime.dwLowDateTime > elem2.m_creationtime.dwHighDateTime + elem2.m_creationtime.dwLowDateTime;
}


bool sortfunc_size_min( FILEINFO& elem1, FILEINFO& elem2 )
{
	return elem1.m_filesize > elem2.m_filesize;
}

bool sortfunc_size_max( FILEINFO& elem1, FILEINFO& elem2 )
{
	return elem1.m_filesize < elem2.m_filesize;
}

void CDirectoryManager::SortList()
{
	struct compmin { 
		bool operator() (const String& left, const String& right) const 
		{return left>right;}
	}; 

	struct compmax { 
		bool operator() (const String& left, const String& right) const 
		{return left<right;}
	}; 


	// 정렬
	if(m_sortmethod == _SORT_NAME_MIN )
	{
		sort( m_directoryinfolist.begin(), m_directoryinfolist.end(), sortfunc_name_min );
		//sort( m_fileinfolist.begin(), m_fileinfolist.end(), sortfunc_name_min );

		std::map<String, std::vector<FILEINFO>, compmin>	templist;
		std::map<String, std::vector<FILEINFO>, compmin>::iterator tit;

		std::vector<FILEINFO>::iterator it = m_fileinfolist.begin();
		for(; it != m_fileinfolist.end(); it++)
		{
			String ext = it->m_ext;
			transform(ext.begin(), ext.end(), ext.begin(), toupper);

			tit = templist.find(ext);
			if( tit == templist.end() )
			{
				// 없슴
				std::vector<FILEINFO> flist;
				flist.push_back(*it);
				templist.insert( std::make_pair(ext, flist) );
			}
			else
			{
				// 있다. 추가
				tit->second.push_back(*it);
			}
		}

		for(tit = templist.begin(); tit != templist.end(); tit++)
		{
			sort( tit->second.begin(), tit->second.end(), sortfunc_name_min );
		}

		m_fileinfolist.clear();

		// copy
		for(tit = templist.begin(); tit != templist.end(); tit++)
		{
			std::vector<FILEINFO>::iterator it = tit->second.begin();
			for(; it != tit->second.end(); it++)
			{
				m_fileinfolist.push_back(*it);
			}
		}

	}
	else if(m_sortmethod == _SORT_NAME_MAX )
	{
		sort( m_directoryinfolist.begin(), m_directoryinfolist.end(), sortfunc_name_max );
		//sort( m_fileinfolist.begin(), m_fileinfolist.end(), sortfunc_name_max );
		std::map<String, std::vector<FILEINFO>, compmax>	templist;
		std::map<String, std::vector<FILEINFO>, compmax>::iterator tit;

		std::vector<FILEINFO>::iterator it = m_fileinfolist.begin();
		for(; it != m_fileinfolist.end(); it++)
		{
			String ext = it->m_ext;
			transform(ext.begin(), ext.end(), ext.begin(), toupper);

			tit = templist.find(ext);
			if( tit == templist.end() )
			{
				// 없슴
				std::vector<FILEINFO> flist;
				flist.push_back(*it);
				templist.insert( std::make_pair(ext, flist) );
			}
			else
			{
				// 있다. 추가
				tit->second.push_back(*it);
			}
		}

		for(tit = templist.begin(); tit != templist.end(); tit++)
		{
			sort( tit->second.begin(), tit->second.end(), sortfunc_name_max );
		}

		m_fileinfolist.clear();

		// copy
		for(tit = templist.begin(); tit != templist.end(); tit++)
		{
			std::vector<FILEINFO>::iterator it = tit->second.begin();
			for(; it != tit->second.end(); it++)
			{
				m_fileinfolist.push_back(*it);
			}
		}

	}
	else if(m_sortmethod == _SORT_TIME_MIN )
	{
		//sort( m_directoryinfolist.begin(), m_directoryinfolist.end(), sortfunc_time_min );
		sort( m_fileinfolist.begin(), m_fileinfolist.end(), sortfunc_time_min );
	}
	else if(m_sortmethod == _SORT_TIME_MAX )
	{
		//sort( m_directoryinfolist.begin(), m_directoryinfolist.end(), sortfunc_time_max );
		sort( m_fileinfolist.begin(), m_fileinfolist.end(), sortfunc_time_max );
	}
	else if(m_sortmethod == _SORT_SIZE_MIN )
	{
		//sort( m_directoryinfolist.begin(), m_directoryinfolist.end(), sortfunc_size_min );
		sort( m_fileinfolist.begin(), m_fileinfolist.end(), sortfunc_size_min );
	}
	else if(m_sortmethod == _SORT_SIZE_MAX )
	{
		//sort( m_directoryinfolist.begin(), m_directoryinfolist.end(), sortfunc_size_max );
		sort( m_fileinfolist.begin(), m_fileinfolist.end(), sortfunc_size_max );
	}

	//////////////////////////////////////////////////////////////////////////

	m_finalresult.clear();

	// 상위 디렉토리 --> ..
	if( m_parent.m_type != TYPE_NONE )
		m_finalresult.push_back(m_parent);

	// 디렉토리
	std::vector<FILEINFO>::iterator it1 = m_directoryinfolist.begin();
	for (; it1 != m_directoryinfolist.end(); it1++)
	{
		if( !(m_displayoption & _DISPLAY_HIDDEN) && ( it1->m_attribute & ATTR_HIDDEN ) )
			continue;

		m_finalresult.push_back(*it1);
	}

	// 파일
	std::vector<FILEINFO>::iterator it2 = m_fileinfolist.begin();
	for (; it2 != m_fileinfolist.end(); it2++)
	{
		if( !(m_displayoption & _DISPLAY_HIDDEN) && ( it2->m_attribute & ATTR_HIDDEN ) )
			continue;

		m_finalresult.push_back(*it2);
	}

	// 드라이브 정보
	for (size_t i=0; i<m_driveinfolist.size();i++)
	{
		m_finalresult.push_back(m_driveinfolist[i]);
	}

}

DWORD CALLBACK CDirectoryManager::progressRoutine(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize,
LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber, DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData )
{
	//SYSTEMTIME end;
	//GetLocalTime(&end);
	//LONGLONG msecs = subtract(&end, (SYSTEMTIME*) lpData);
	//cout << TotalBytesTransferred.QuadPart << "/" << TotalFileSize.QuadPart  << " bytes copied in " << " " << msecs  << "  ms ...." << endl;
	//_TRACE(L"%d\n", TotalBytesTransferred.QuadPart);

	CopyDialog *dialog = (CopyDialog*)lpData;

	wchar temp1[MAX_PATH];
	unicode::sprintf(temp1, _countof(temp1), L"%s / %s", utility::getStringSize(TotalBytesTransferred.QuadPart, 2).c_str(), utility::getStringSize(TotalFileSize.QuadPart, 2).c_str()); 
	dialog->Update(L"copysize", temp1);

	int currentprogress = (int)((double)TotalBytesTransferred.QuadPart / (double)(TotalFileSize.QuadPart) * 100.f);
	dialog->Update(L"currentprogress", unicode::format(L"%d",currentprogress));

	if( dialog->IsCancel() )
	{
		return PROGRESS_CANCEL;
	}

	return PROGRESS_CONTINUE;
}

int CDirectoryManager::ActionCopy(const wchar *src, const wchar *dest, int progress1, int totalprogress1, CopyDialog *dialog, unsigned long option)
{
	bool ret = true;

	String progress1str = unicode::format(L"%d", progress1);
	String totalstr = unicode::format(L"%d", totalprogress1);

	dialog->Update(L"totalprogressrange", totalstr);
	dialog->Update(L"currentprogressrange", L"100");

	dialog->Update(L"sourcepath", src);
	dialog->Update(L"destpath", dest);
	dialog->Update(L"totalprogress", progress1str.c_str());
	dialog->Update(L"copycount", unicode::format(L"%d/%d",progress1,totalprogress1));


	SetFileAttributes(dest, FILE_ATTRIBUTE_NORMAL);
	BOOL b = FALSE;
	BOOL result = CopyFileEx(src, dest, &progressRoutine , dialog, &b,  COPY_FILE_RESTARTABLE);
//	assert(result);

	DWORD errorcode = 0;
	if( result == FALSE )
	{
		 errorcode = GetLastError();
		 return _COPY_RESULT::CANT_COPY;
	}

	if( dialog->IsCancel() )
	{
		return _COPY_RESULT::CANCEL_COPY;
	}

/*
	__int64 srcfilesize = utility::GetFileLength(src);
	__int64 totalreadsize = 0;
	__int64 remainsize = srcfilesize;

	int currentprogress = 0;	// %

	String progress1str = unicode::format(L"%d", progress1);
	String totalstr = unicode::format(L"%d", totalprogress1);


	FILE *infp, *outfp;
	_wfopen_s(&infp, src, L"rb");
	_wfopen_s(&outfp, dest, L"wb");

	if( infp == NULL || outfp == NULL) return true;

	wchar temp1[MAX_PATH];

	dialog->Update(L"totalprogressrange", totalstr);
	dialog->Update(L"currentprogressrange", L"100");

	while(remainsize > 0)
	{
		int readsize = remainsize > COPYBUFFERSIZE ? COPYBUFFERSIZE : remainsize;
		totalreadsize += readsize;

		unicode::sprintf(temp1, _countof(temp1), L"%s / %s", utility::getStringSize(totalreadsize, 2).c_str(), utility::getStringSize(srcfilesize, 2).c_str()); 
		currentprogress = (int)((double)(totalreadsize) / (double)srcfilesize * 100.f);
		dialog->Update(L"copysize", temp1);
		dialog->Update(L"sourcepath", src);
		dialog->Update(L"destpath", dest);
		dialog->Update(L"totalprogress", progress1str.c_str());
		dialog->Update(L"currentprogress", unicode::format(L"%d",currentprogress));

		dialog->Update(L"copycount", unicode::format(L"%d/%d",progress1,totalprogress1));

		if( dialog->IsCancel() )
		{
			ret = false;
			break;
		}


		fread(copybuffer, 1, readsize, infp);
		fwrite(copybuffer, 1, readsize, outfp);

		remainsize -= readsize;
	}

	fclose(infp);
	fclose(outfp);

	if( ret == false )
	{
		// 취소시 작업하던것 삭제
		utility::removefile(dest, false);
	}
*/

	return _COPY_RESULT::NO_ERRROR;
}

void	CDirectoryManager::ActionCur(bool param)
{
	if( m_finalresult[m_selectedpos].m_type & TYPE_DRIVE)
	{
		// 드라이브 변경
		ChangeDrive(m_finalresult[m_selectedpos].m_fullpath.c_str());
	}
	else if( m_finalresult[m_selectedpos].m_type & TYPE_DIRECTORY)
	{
		ResetLastSelect();

		// 디렉토리 변경
		if( m_finalresult[m_selectedpos].m_name == L".." )
		{
			// 윗 디렉토리로 이동
			MoveToBackDir();
		}
		else
		{

			// change directory
			m_currentDir = m_finalresult[m_selectedpos].m_fullpath;
			m_currentDir = CheckValidDirectory(m_currentDir);

			CollectCurrentDirInfo();
			m_selectedpos = 0;

			// pathinfo_ini에 최근경로 갱신
			String k = String(m_currentDir, 0, 1);
			transform(k.begin(), k.end(), k.begin(), toupper);
			pathinfo_ini->Set(k.c_str(), m_currentDir.c_str());

			// last drive 갱신
			std::wstring drivestr = std::wstring(L"LASTDRIVE") + ( whatwindow == _LEFT_WINDOW ? L"_LEFT" : L"_RIGHT");
			pathinfo_ini->Set( drivestr.c_str(), m_currentDir.c_str());

			SavePathinfo();
			//filechecker->Reset( m_currentDir.c_str() );
			filemonitor->Change(m_currentDir.c_str());
		}
	}
	else if( m_finalresult[m_selectedpos].m_type & TYPE_FILE)
	{
		// file 실행
		String ext = m_finalresult[m_selectedpos].m_ext;
		transform(ext.begin(), ext.end(), ext.begin(), toupper);

		if( ext == L"EXE" || ext == L"MSI" || ext == L"BAT")
		{
			if( param )
			{
				ExecParam dialog(parentwindow);
				dialog.ShowModal();

				if( dialog.ret == wxID_OK )
				{
					String exec = m_finalresult[m_selectedpos].m_excutepath + L" " + dialog.param;
					String dir = utility::GetDrive(exec.c_str()) + utility::GetPath(exec.c_str());
					utility::CreateProcess(exec, dir);
				}
			}
			else
			{
				String dir = utility::GetDrive(m_finalresult[m_selectedpos].m_excutepath.c_str()) + utility::GetPath(m_finalresult[m_selectedpos].m_excutepath.c_str());
				utility::CreateProcess(m_finalresult[m_selectedpos].m_excutepath, dir);
			}
		}
		else
		{
			ShellExecute(NULL,
				L"open",
				m_finalresult[m_selectedpos].m_excutepath.c_str(),
				NULL,m_finalresult[m_selectedpos].m_fullpath.c_str(),SW_SHOW);
		}

	}
}

void	CDirectoryManager::ActionSetFileTime(const wchar *filename, FILETIME *ctime, FILETIME *latime, FILETIME *lwtime)
{
	// Get a handle to our file and with file_write_attributes access
	HANDLE handle = CreateFile(filename, 
		FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	// Set the file time on the file
	SetFileTime(handle, ctime, latime, lwtime);
	// Close our handle.
	CloseHandle(handle);
}

void	CDirectoryManager::ActionSetFileAttribute(const wchar *filename, unsigned long attriute)
{
	SetFileAttributes(filename, attriute);
}


void	CDirectoryManager::CopyDirectory(String dest, std::vector<SCANINFO> &scaninfolist, bool convert)
{
	filecheckerlock.Enter();

	int retoverwrite = 0;

	if(convert)
	{
		// 원본 경로를 목적지 경로 변환
		for (size_t i=0; i<scaninfolist.size(); i++)
		{
			String ret = utility::replace(scaninfolist[i].path, m_currentDir, dest);
			scaninfolist[i].destpath = ret;
		}
	}

	CopyDialog dialog(parentwindow, L"복사");
	for (size_t i=0; i<scaninfolist.size(); i++)
	{
		if( scaninfolist[i].type == TYPE_DIRECTORY )
		{
			// directory 만들기
			bool r = utility::makedirectory(scaninfolist[i].destpath.c_str());
		}
		else if( scaninfolist[i].type == TYPE_FILE )
		{
			bool checkexist = false;
			int checkoverwrite = 0;

			if( retoverwrite != ID_OK_ALL && retoverwrite != ID_SKIP_ALL )
			{
				// 모두 덮어쓰기면 검사 안한다.
				checkexist = utility::access(scaninfolist[i].destpath.c_str());
				if( checkexist == true )
				{
					dialog.Show(false);

					int srcsize = utility::GetFileLength(scaninfolist[i].path.c_str());
					int destsize = utility::GetFileLength(scaninfolist[i].destpath.c_str());

					SYSTEMTIME st1, st2;
					st1 = utility::getLastWriteTime(scaninfolist[i].path);
					st2 = utility::getLastWriteTime(scaninfolist[i].destpath);

					OverWriteDialog *overdialog = new OverWriteDialog(&dialog, L"같은 파일이 있습니다.", 
						scaninfolist[i].path.c_str(), scaninfolist[i].destpath.c_str(),
						srcsize, destsize, st1, st2);

					overdialog->ShowModal();
					checkoverwrite = overdialog->getResult();
					delete overdialog;

					if( checkoverwrite == ID_CANCEL)	// 취소
					{
						dialog.Destroy();
						filecheckerlock.Leave();
						return;
					}
					else if( checkoverwrite == ID_RENAME)
					{
						// 이름 바꿔서 복사 : ret가 false면 pass??
						String tempdrive = utility::GetDrive(scaninfolist[i].destpath.c_str());
						String temppath = utility::GetPath(scaninfolist[i].destpath.c_str());
						String name = utility::GetFileName(scaninfolist[i].destpath.c_str());
						
						RenameDialog *renamedialog = new RenameDialog(&dialog, L"새로운 이름으로 복사" , name.c_str(), L"", false);
						renamedialog->ShowModal();
						String retrename = Valueman::GetInstance()->getvalue(L"rename_name");
						if( !retrename.empty() )
						{
							Valueman::GetInstance()->remove(L"rename_name");
							scaninfolist[i].destpath = tempdrive+temppath + retrename;
						}
						else
						{
							checkoverwrite = ID_SKIP;
						}
					}

					dialog.Show(true);
				}
			}

			bool docopy = true;
			if( checkexist == true )
			{
				if( checkoverwrite == ID_SKIP || checkoverwrite == ID_SKIP_ALL || retoverwrite == ID_SKIP_ALL)
					docopy = false;

				if( checkoverwrite == ID_SKIP_ALL )
					retoverwrite = ID_SKIP_ALL;

				if( checkoverwrite == ID_OK_ALL)
					retoverwrite = ID_OK_ALL;
			}

			if(docopy)
			{
				int errorcode = _COPY_RESULT::NO_ERRROR;
				while (1)
				{
					if( errorcode == _COPY_RESULT::CANCEL_COPY)
						break;
					else if( errorcode == _COPY_RESULT::CANT_COPY)
					{
						// 사용중??

						String filename = utility::GetFileName(scaninfolist[i].path.c_str());
						String retrymsg = unicode::format(L"%s파일 복사 중 오류가 발생하였습니다.\n다시 시도 합니까?", filename.c_str());
						wxMessageDialog dial(NULL,
							retrymsg.c_str(), 
							L"오류", 
							wxCENTER | wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);

						int ret = dial.ShowModal();
						if( ret == wxID_YES )
							errorcode = _COPY_RESULT::NO_ERRROR;
						else
							errorcode = _COPY_RESULT::CANCEL_COPY;
					}
					else 
					{
						// 위치에 복사
						errorcode = ActionCopy(scaninfolist[i].path.c_str(), scaninfolist[i].destpath.c_str(), i+1,(int)scaninfolist.size(), &dialog);
						if( errorcode == _COPY_RESULT::NO_ERRROR )
							break;
					}
				}
			}
			
//			ActionSetFileTime(scaninfolist[i].destpath.c_str(), &scaninfolist[i].creationtime, &scaninfolist[i].LastAccessTime, &scaninfolist[i].LastWriteTime);
		}
	}

	dialog.Destroy();
	filecheckerlock.Leave();
}

// ctrl-v
void	CDirectoryManager::PasteFiles(const wxArrayString &files, String destpath)
{
	std::vector<SCANINFO> scaninfolist;

	int count = 0;
	for(size_t i=0; i<files.Count(); i++)
	{
		BOOL d = PathIsDirectory(files[i].c_str());
		int type = d ? TYPE_DIRECTORY : TYPE_FILE;

		if( type == TYPE_DIRECTORY )
		{
			SCANINFO s;
			s.path = files[i];
			s.type = type;

			String rpath = utility::GetDrive(s.path.c_str()) + utility::GetPath(s.path.c_str());
			String ret = utility::replace(s.path, rpath, L"");

			s.destpath = destpath + L"\\" + ret;
			scaninfolist.push_back(s);

			int t = 0;
			std::vector<SCANINFO> templist;
			Scandirectory(files[i].c_str(), count, templist, false);

			// 원본 경로를 목적지 경로 변환
			for (size_t i=0; i<templist.size(); i++)
			{
				String ret = utility::replace(templist[i].path, rpath, L"");
				templist[i].destpath = destpath + L"\\" + ret;
				scaninfolist.push_back(templist[i]);
			}

		}
		else if( type == TYPE_FILE )
		{
			SCANINFO s;
			s.type = type;
			s.path = files[i];
			String rpath = utility::GetDrive(s.path.c_str()) + utility::GetPath(s.path.c_str());
			String ret = utility::replace(s.path, rpath, L"");
			s.destpath = destpath + L"\\" + ret;

			scaninfolist.push_back(s);
		}
	}

	CopyDirectory(destpath, scaninfolist, false);
}

// 이동
void	CDirectoryManager::Move()
{

#if 1
	std::vector<SCANINFO> filelist;
	GetFileList(filelist, false);

	CDirectoryManager *nextdoor = ((MojoShell*)parentwindow)->GetNextDoor(whatwindow == _LEFT_WINDOW ? _RIGHT_WINDOW : _LEFT_WINDOW);
	String key = L"movedest";
	String nextdoordir = nextdoor->GetCurrentDir();
	EnterTextDialog *dialog = new EnterTextDialog( parentwindow, L"다른창으로 이동", L"이동 경로", nextdoordir.c_str(), key.c_str());
	dialog->ShowModal();
	delete dialog;

	String ret =  Valueman::GetInstance()->getvalue(key.c_str());
	Valueman::GetInstance()->remove(key.c_str());

	std::vector<String> files;
	for(size_t i = 0; i < filelist.size(); i++)
		files.push_back(filelist[i].path);

	MoveFiles(files, ret);

#else
	filecheckerlock.Enter();

	//utility::movefile(L"d:\\testmove", L"d:\\testmove2", L"testmove1" );
	std::vector<SCANINFO> filelist;
	GetFileList(filelist, false);

	CDirectoryManager *nextdoor = ((MojoShell*)parentwindow)->GetNextDoor(whatwindow == _LEFT_WINDOW ? _RIGHT_WINDOW : _LEFT_WINDOW);
	String key = L"movedest";
	String nextdoordir = nextdoor->GetCurrentDir();
	EnterTextDialog *dialog = new EnterTextDialog( parentwindow, L"다른창으로 이동", L"이동 경로", nextdoordir.c_str(), key.c_str());
	dialog->ShowModal();
	delete dialog;

	String ret =  Valueman::GetInstance()->getvalue(key.c_str());
	if( !ret.empty() && utility::access(ret.c_str()) == true)
	{
		Valueman::GetInstance()->remove(key.c_str());

		MoveFileDialog progressdialog( parentwindow, wxID_ANY); 
		progressdialog.Show(true);

		progressdialog.Update(String(L"target"), ret);

		for(size_t i = 0; i < filelist.size(); i++)
		{
			progressdialog.Update(String(L"filename"), filelist[i].path);

			int len = filelist.size();
			progressdialog.Update(String(L"filecount"), unicode::format(L"(%d/%d)", i+1, len));

			int fileprogress = (int)((float)(i+1) / (float)len * 100.f);
			progressdialog.Update(String(L"progress"), unicode::format(L"%d", fileprogress));

			wchar temp[1024];
			unicode::strcpy(temp, 1024, filelist[i].path.c_str());
			PathStripPath(temp);
			utility::movefile(filelist[i].path.c_str(), ret.c_str(), temp);

			if(progressdialog.iscancel)
			{
				ret = true;
				break;
			}
		}

		progressdialog.Destroy();
	}

	ClearSelected();
	filecheckerlock.Leave();
#endif
}

void	CDirectoryManager::MoveFiles(const std::vector<String> &filelist, String destpath)
{
	filecheckerlock.Enter();

	String ret =  destpath;
	if( !ret.empty() && utility::access(ret.c_str()) == true)
	{
		MoveFileDialog progressdialog( parentwindow, wxID_ANY); 
		progressdialog.Show(true);

		progressdialog.Update(String(L"target"), ret);

		for(size_t i = 0; i < filelist.size(); i++)
		{
			progressdialog.Update(String(L"filename"), filelist[i]);

			int len = filelist.size();
			progressdialog.Update(String(L"filecount"), unicode::format(L"(%d/%d)", i+1, len));

			int fileprogress = (int)((float)(i+1) / (float)len * 100.f);
			progressdialog.Update(String(L"progress"), unicode::format(L"%d", fileprogress));

			wchar temp[1024];
			unicode::strcpy(temp, 1024, filelist[i].c_str());
			PathStripPath(temp);
			utility::movefile(filelist[i].c_str(), ret.c_str(), temp);

			if(progressdialog.iscancel)
			{
				ret = true;
				break;
			}
		}

		progressdialog.Destroy();
	}

	filecheckerlock.Leave();
}


// 현재 라인 rename
void	CDirectoryManager::Rename()
{
	filecheckerlock.Enter();

	if( m_finalresult[m_selectedpos].m_type & TYPE_DIRECTORY )
	{
		String path, name, ext;
		unicode::splitPath(m_finalresult[m_selectedpos].m_fullpath, path, name, ext, true);

		RenameDialog *dialog = new RenameDialog(parentwindow, wxT("이름 바꾸기"), name, L"", false);
		dialog->ShowModal();

		String ret = Valueman::GetInstance()->getvalue(L"rename_name");
		if( !ret.empty() )
		{
			Valueman::GetInstance()->remove(L"rename_name");

			String oldpath = m_finalresult[m_selectedpos].m_fullpath;
			String newpath = path + ret;
			
			int r = _wrename( oldpath.c_str(), newpath.c_str() );
			if( r == -1 )	
			{
				wxMessageDialog *dial = new wxMessageDialog(parentwindow, 
					wxT("이름을 변경 할수 없습니다."), wxT("오류"), wxOK | wxICON_ERROR);
				dial->ShowModal();
			}
		}

	}
	else if( m_finalresult[m_selectedpos].m_type & TYPE_FILE )
	{
		wchar name[_MAX_PATH] = {0,};
		wchar ext[_MAX_EXT] = {0,};
		_wsplitpath_s(m_finalresult[m_selectedpos].m_excutepath.c_str(), NULL, 0, NULL, 0, name, _countof(name), ext, _countof(ext));

		if( ext[0] == '.' )
			unicode::strcpy(ext, _countof(ext), &ext[1]);
		
		RenameDialog *dialog = new RenameDialog(parentwindow, wxT("이름 바꾸기"), name, ext, true);
		int r = dialog->ShowModal();

		String ret = Valueman::GetInstance()->getvalue(L"rename_name");
		String retext = Valueman::GetInstance()->getvalue(L"rename_ext");

		if( !ret.empty() )
		{
			Valueman::GetInstance()->remove(L"rename_name");
			Valueman::GetInstance()->remove(L"rename_ext");

			String newpath = m_finalresult[m_selectedpos].m_fullpath + ret + L"." + retext;
			if( m_finalresult[m_selectedpos].m_excutepath != newpath )
			{
				int r = _wrename( m_finalresult[m_selectedpos].m_excutepath.c_str(), newpath.c_str() );
				if( r == -1 )	
				{
					wxMessageDialog *dial = new wxMessageDialog(parentwindow, 
						wxT("이름을 변경 할수 없습니다."), wxT("오류"), wxOK | wxICON_ERROR);
					dial->ShowModal();
				}
				//bool rr = utility::renamefile( m_finalresult[m_selectedpos].m_excutepath.c_str(), newpath.c_str() );
			}
		}
	}

	filecheckerlock.Leave();
}

void	CDirectoryManager::RefreshDirectory()
{
	CollectCurrentDirInfo();
	parentwindow->Refresh();
}


void	CDirectoryManager::SendMonitorInfo(unsigned long action, wchar *monitorpath, wchar *filename_0, wchar *filename_1)
{
// 	if(m_currentDir != monitorpath )
// 		return;

	filecheckerlock.Enter();

	String fullpath;
	int len = unicode::strlen(monitorpath);
	if(monitorpath[len-1] == '\\')
		fullpath = String(monitorpath) + filename_0;
	else
		fullpath = String(monitorpath) + String(L"\\") + filename_0;

	switch(action)
	{
		case FILE_ACTION_ADDED : 
			{
				FILEINFO info;
				info.m_name = filename_0;

				unsigned long flag = utility::GetPathAttribute(fullpath.c_str());
				info.m_attribute = (flag & ATTR_READONLY) ? ATTR_READONLY : 0;
				info.m_attribute |= (flag & ATTR_HIDDEN) ? ATTR_HIDDEN : 0;
				info.m_attribute |= (flag & ATTR_SYSTEM) ? ATTR_SYSTEM : 0;
				info.m_attribute |= (flag & ATTR_ARCHIVE) ?  ATTR_ARCHIVE : 0;
				utility::getFileTime(info.m_fullpath, info.m_creationtime, info.m_LastAccessTime, info.m_LastWriteTime);

				if( flag & FILE_ATTRIBUTE_DIRECTORY)
				{
					info.m_fullpath = fullpath;
					info.m_ext = L"[폴더]";
					info.m_type = TYPE_DIRECTORY;
					m_directoryinfolist.push_back(info);
				}
				else if( flag != INVALID_FILE_ATTRIBUTES )
				{
					String name, ext;
					info.m_type = TYPE_FILE;

					name = utility::GetName(fullpath.c_str());
					ext = utility::GetExtention(fullpath.c_str());
					ext.erase(ext.begin());

					info.m_fullpath = utility::GetDrive(fullpath.c_str()) + utility::GetPath(fullpath.c_str());
					info.m_name = name;
					info.m_ext = ext;
					info.m_excutepath = fullpath;

					//info.m_filesize = utility::GetFileLength(info.m_fullpath);

					m_fileinfolist.push_back(info);
				}

				SortList();
			}
			break;

		case FILE_ACTION_REMOVED : 
			{
				RemoveFromList(fullpath);
				RestoreLastSelect();
			}
			break;

		case FILE_ACTION_MODIFIED : 
			{
				FILEINFO info;
				unsigned long flag = utility::GetPathAttribute(fullpath.c_str());
				info.m_attribute = (flag & ATTR_READONLY) ? ATTR_READONLY : 0;
				info.m_attribute |= (flag & ATTR_HIDDEN) ? ATTR_HIDDEN : 0;
				info.m_attribute |= (flag & ATTR_SYSTEM) ? ATTR_SYSTEM : 0;
				info.m_attribute |= (flag & ATTR_ARCHIVE) ?  ATTR_ARCHIVE : 0;

				Sleep(10);
				info.m_filesize = utility::GetFileLength(fullpath.c_str());
				utility::getFileTime(fullpath, info.m_creationtime, info.m_LastAccessTime, info.m_LastWriteTime);

				EditInfoFromList(fullpath, &info);
			}
			break;

		case FILE_ACTION_RENAMED_OLD_NAME : 
			break;

		case FILE_ACTION_RENAMED_NEW_NAME : 
			{
				String newfullpath;
				int len = unicode::strlen(monitorpath);
				if(monitorpath[len-1] == '\\')
					newfullpath = String(monitorpath) + filename_1;
				else
					newfullpath = String(monitorpath) + String(L"\\") + filename_1;

				ReplacePath(fullpath, filename_0, newfullpath, filename_1);
				SortList();
			}
			break;
	}

	filecheckerlock.Leave();
	parentwindow->Refresh();
}

void	CDirectoryManager::StartChecker()
{
	//filechecker->Start( this, m_currentDir.c_str() );
	filemonitor->Start(m_currentDir.c_str());
}

void	CDirectoryManager::StopChecker()
{
	//filechecker->Stop();
}

int		CDirectoryManager::MouseClick(wxPoint _pos, int x, int y, int which)
{
	int select = -1;
	int t = (y-LINE_HEIGHT);

	if( t > 0)
	{
		int pos = t / LINE_HEIGHT;
		int lines = (int)m_finalresult.size() - m_displaytop;
		if( lines > pos )
			select = pos + m_displaytop;
	}

	if(m_displaylines <= select-m_displaytop)
		return -1;

	if(which == CDirectoryManager::_MOUSE_CLICK_LEFT)
	{
		if( select != m_selectedpos && select > -1)
			m_selectedpos = select;
		SelectImage();
	}
	else if(which == CDirectoryManager::_MOUSE_CLICK_RIGHT)
	{
		if( select != m_selectedpos && select > -1)
			m_selectedpos = select;

		((MojoShell*)parentwindow)->Refresh(false);

		HWND parentWindow = (HWND)((MojoShell*)parentwindow)->getHwnd();
		wxSize wsize = ((MojoShell*)parentwindow)->getWindowSize();
		wxSize csize = ((MojoShell*)parentwindow)->getCanvasSize();
		wxSize gap = wsize - csize;

		int xPos = _pos.x + x + gap.x;// -(whatwindow == _RIGHT_WINDOW ? SEPARATE_WIDTH : 0);
		int yPos = _pos.y + y + gap.y;// - LINE_HEIGHT;

		String path = L"";
		if( m_finalresult[m_selectedpos].m_type == TYPE_DRIVE)
		{
			path = m_finalresult[m_selectedpos].m_fullpath;
		}
		else if( m_finalresult[m_selectedpos].m_type == TYPE_DIRECTORY )
		{
			if( m_finalresult[m_selectedpos].m_fullpath != L".." )
				path = m_finalresult[m_selectedpos].m_fullpath;
		}
		else if( m_finalresult[m_selectedpos].m_type == TYPE_FILE )
		{
			path = m_finalresult[m_selectedpos].m_excutepath;
		}

		if(!path.empty())
			utility::openShellContextMenuForObject(path, xPos, yPos, parentWindow);

	}
	else if(which == CDirectoryManager::_MOUSE_DCLICK_LEFT)
	{
		if( select != m_selectedpos && select > -1)
			m_selectedpos = select;
		else if( select == m_selectedpos && select > -1)
		{
			// 현위치의것 실행
			ActionCur();
		}
	}
	else if(which == CDirectoryManager::_MOUSE_MOVE_DROP)
	{
		if( select != m_selectedpos && select > -1)
		{
			if( m_finalresult[select].m_type == TYPE_DIRECTORY || m_finalresult[select].m_type == TYPE_FILE )
			{
				lastselectline = select;
			}
		}
		else
			lastselectline = -1;
	}
	else if(which == CDirectoryManager::_MOUSE_CLICK_DROP)
	{
/*
		if( lastselectline != -1 )
		{
			if( m_finalresult[lastselectline].m_type == TYPE_DIRECTORY )
			{
				if( m_finalresult[lastselectline].m_name == L".." )
					dropTargetdir = L"..";
				else
					dropTargetdir = m_finalresult[lastselectline].m_fullpath;
			}
			else
				dropTargetdir.clear();

		}
		lastselectline = -1;
*/
	}
	else if(which == CDirectoryManager::_MOUSE_MOVE)
	{
		lastselectline = -1;
	}

	return select;
}

void	CDirectoryManager::MouseWheel(int delta)
{
	//_TRACE(L"-> %d\n", delta);
	if( delta > 0)
	{
		// up
		Up();
	}
	else
	{
		// down
		Down();
	}
}

void	CDirectoryManager::ExcuteInput()
{
	String key = L"execinput";
	EnterTextDialog dialog( parentwindow, L"직접 입력 실행", L"실행할 명령어를 넣어주세요.", L"", key.c_str());
	dialog.ShowModal();

	String ret =  Valueman::GetInstance()->getvalue(key.c_str());
	if( !ret.empty() )
	{
		Valueman::GetInstance()->remove(key.c_str());

		ShellExecute(NULL,
			L"open",
			ret.c_str(),
			NULL,
			m_currentDir.c_str(),
			SW_SHOW);

/*
		// CMD 작동안함
		STARTUPINFO StartupInfo = {0};
		PROCESS_INFORMATION ProcessInfo;
		StartupInfo.cb = sizeof( STARTUPINFO );
		::CreateProcess(ret.c_str(),
			NULL, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo);*/
	}
}

void	CDirectoryManager::ResetLastSelect()
{
	lastselectinfo.type = -1;
	lastselectinfo.name = L"";
	lastselectinfo.ext = L"";
}

// 현재 커서가 있는 이름을 찾아서 그 위치로 이동
void	CDirectoryManager::SetLastSelect()
{
	lastselectinfo.type = m_finalresult[m_selectedpos].m_type;
	lastselectinfo.name = m_finalresult[m_selectedpos].m_name;
	lastselectinfo.ext = m_finalresult[m_selectedpos].m_ext;

	SelectImage();
}

// 특정 위치로 커서 이동
void	CDirectoryManager::SetLastSelectInPos(int pos)
{
	lastselectinfo.type = m_finalresult[pos].m_type;
	lastselectinfo.name = m_finalresult[pos].m_name;
	lastselectinfo.ext = m_finalresult[pos].m_ext;

	SelectImage();
}

// 마지막 선택된거로 복구
void	CDirectoryManager::RestoreLastSelect()
{
	m_selectedpos = 0;
	int total = (int)m_finalresult.size();
	for(size_t i =0; i<m_finalresult.size(); i++)
	{
		if(lastselectinfo.type == TYPE_DIRECTORY)
		{
			if( m_finalresult[i].m_type == lastselectinfo.type && m_finalresult[i].m_name == lastselectinfo.name )
			{
				m_selectedpos = i;
				break;
			}
		}
		else
		{
			if( m_finalresult[i].m_type == lastselectinfo.type && m_finalresult[i].m_name == lastselectinfo.name
				&& m_finalresult[i].m_ext == lastselectinfo.ext)
			{
				m_selectedpos = i;
				break;
			}
		}
	}

	//  가능하다면 원래있던 자리로 표시
	if( m_displaylines < total )
	{
		if( m_selectedpos >= m_currentdisplaypos)
			m_displaytop = m_selectedpos - m_currentdisplaypos;
		else 
		{
			if( m_displaylines >= m_selectedpos )
			{
				m_displaytop = 0;
			}
			else
				m_displaytop = m_selectedpos;

		}
	}
	else
	{
		m_displaytop = 0;
	}

	SelectImage();

/*

	if( m_displaytop > m_selectedpos )
	{
		// 위로 올라가야함
		m_displaytop = m_selectedpos;
	}
	else 
	{
		if( m_displaytop + m_displaylines < m_selectedpos )
		{
			m_displaytop = m_selectedpos - m_displaylines + 1;
		}
	}
*/
}

//////////////////////////////////////////////////////////////////////////


void	CDirectoryManager::DropFiles(int x, int y, const wxArrayString& filenames)
{
	int  selline = lastselectline;

	DragDropDialog dial( NULL, wxID_ANY, L"");
	//wxMessageDialog dial(NULL, L"복사하시겠습니까?", L"복사", wxCENTER | wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);

	if(selline != -1)
	{
		if( m_finalresult[selline].m_type == TYPE_FILE )
		{
			String dir = utility::GetDrive(filenames[0]) + utility::GetPath(filenames[0]);
			if( dir[dir.size()-1] == '\\' ) 
				dir = String(dir, 0, dir.size()-1);

			if( dir != m_currentDir)
			{
				int ret = dial.ShowModal();
				if( ret == ID_DRAGDROP_COPY )
				{
					PasteFiles(filenames, m_currentDir);
				}
				else if( ret == ID_DRAGDROP_MOVE )
				{
					// MOVE
					std::vector<String> filelist;
					for (int i=0; i<(int)filenames.size(); i++)
						filelist.push_back(String(filenames[i].c_str()));

					MoveFiles(filelist, m_currentDir);
				}
			}
		}
		else if( m_finalresult[selline].m_type == TYPE_DIRECTORY )
		{
			int ret = dial.ShowModal();
			if( ret == ID_DRAGDROP_COPY )
			{
				if( m_finalresult[selline].m_name == L".." )
					PasteFiles(filenames, utility::GetParentPath(m_currentDir.c_str()));
				else
					PasteFiles(filenames, m_finalresult[selline].m_fullpath);
			}
			else if( ret == ID_DRAGDROP_MOVE )
			{
				// MOVE
				std::vector<String> filelist;
				for (int i=0; i<(int)filenames.size(); i++)
					filelist.push_back(String(filenames[i].c_str()));

				MoveFiles(filelist, m_currentDir);
			}
		}
	}
	else
	{
		String dir = utility::GetDrive(filenames[0]) + utility::GetPath(filenames[0]);
		if( dir[dir.size()-1] == '\\' ) 
			dir = String(dir, 0, dir.size()-1);

		if( dir != m_currentDir)
		{
			int ret = dial.ShowModal();
			if( ret == ID_DRAGDROP_COPY )
			{
				PasteFiles(filenames, m_currentDir);
			}
			else if( ret == ID_DRAGDROP_MOVE )
			{
				// MOVE
				std::vector<String> filelist;
				for (int i=0; i<(int)filenames.size(); i++)
					filelist.push_back(String(filenames[i].c_str()));

				MoveFiles(filelist, m_currentDir);
			}

		}
	}

/*
	int select = -1;
	int t = (y-LINE_HEIGHT);
	if( t > 0)
	{
		int pos = t / LINE_HEIGHT;
		int lines = (int)m_finalresult.size() - m_displaytop;
		if( lines > pos )
			select = pos + m_displaytop;
	}

	String destpath;
	if( m_selectedpos > -1 )
	{
		if( m_finalresult[m_selectedpos].m_type == TYPE_DIRECTORY )
		{
			if( m_finalresult[m_selectedpos].m_fullpath != L".." )
				destpath = m_finalresult[m_selectedpos].m_fullpath;
		}
		else if( m_finalresult[m_selectedpos].m_type == TYPE_FILE )
		{
			destpath = m_finalresult[m_selectedpos].m_fullpath;
		}
	}
	else
	{
		destpath = m_currentDir;
	}

	if( destpath.size() > 0 )
		PasteFiles(filenames, destpath);
		*/
}

void	CDirectoryManager::GetDragFiles(std::vector<String> &lists)
{
	// 선택된것들
	for(int i=0; i<(int)m_finalresult.size(); i++)
	{
		if( m_finalresult[i].m_selected )
		{
			if(m_finalresult[i].m_type & TYPE_DIRECTORY)
			{
				if(m_finalresult[i].m_name != L"..")
					lists.push_back(m_finalresult[i].m_fullpath);
			}
			else if(m_finalresult[i].m_type & TYPE_FILE)
			{
				lists.push_back(m_finalresult[i].m_excutepath);
			}
		}
	}

	// 선택된것 없을때
	if(lists.empty() == true)
	{
		if(m_finalresult[m_selectedpos].m_type & TYPE_DIRECTORY)
		{
			if(m_finalresult[m_selectedpos].m_name != L"..")
				lists.push_back(m_finalresult[m_selectedpos].m_fullpath);
		}
		else if(m_finalresult[m_selectedpos].m_type & TYPE_FILE)
		{
			lists.push_back(m_finalresult[m_selectedpos].m_excutepath);
		}
	}
}

void	CDirectoryManager::SaveSortInfo()
{
	String softstr = std::wstring(L"SORT") + ( whatwindow == _LEFT_WINDOW ? L"_LEFT" : L"_RIGHT");

	if( m_sortmethod == _SORT_NAME_MIN || m_sortmethod == _SORT_NAME_MAX )
	{
		pathinfo_ini->Set( softstr.c_str(), m_sortmethod == _SORT_NAME_MIN ? L"name-min" : L"name-max" );
	}
	else if( m_sortmethod == _SORT_TIME_MIN || m_sortmethod == _SORT_TIME_MAX )
	{
		pathinfo_ini->Set( softstr.c_str(), m_sortmethod == _SORT_TIME_MIN ? L"time-min" : L"time-max" );
	}
	else if( m_sortmethod == _SORT_SIZE_MIN || m_sortmethod == _SORT_SIZE_MAX )
	{
		pathinfo_ini->Set( softstr.c_str(), m_sortmethod == _SORT_SIZE_MIN ? L"size-min" : L"size-max" );
	}

	SavePathinfo();
}


void	CDirectoryManager::CompressFiles(std::vector<SCANINFO> &compresslist)
{
	CDirectoryManager *nextdoor = ((MojoShell*)parentwindow)->GetNextDoor(whatwindow == _LEFT_WINDOW ? _RIGHT_WINDOW : _LEFT_WINDOW);
	String nextdoordir = nextdoor->GetCurrentDir();

	Compress dialog(parentwindow, wxID_ANY); 
	dialog.ShowModal();
	int ret = dialog.getResult();
	String compressfilename = dialog.compressfilename;
	int method = dialog.compressmethod;
	int archivetype = dialog.compressarchive;
	String password = dialog.compresspassword;

	String compfile;
	bool iscancel = false;

	if( ret == ID_OK && compressfilename != L"" )
	{
		std::vector<String> dirlist;
		dirlist.push_back(nextdoordir);
		dirlist.push_back(GetCurrentDir());

		DestDialog destdialog(parentwindow, L"압축", L"압축 경로", dirlist, false);
		if( destdialog.ShowModal() == wxID_CANCEL) 
			return;

		String destdir = destdialog.returnstr;

		CompressProgress *progressdialog = new CompressProgress( parentwindow, wxID_ANY); 
		progressdialog->Show(true);

		String compfile = destdir + L"\\" + compressfilename + archiveext[archivetype];;
		progressdialog->Update(L"zipfilename", (compressfilename + archiveext[archivetype]).c_str());

		Compressor *compressor = new Compressor();
		compressor->Init();
		compressor->BegineCompressFile(compfile, password, method, archivetype);

		for (size_t i=0; i<compresslist.size(); i++)
		{
			String zippath;
			String getdir = utility::GetDrive(compresslist[i].path.c_str()) + utility::GetPath(compresslist[i].path.c_str());

			if( getdir[getdir.size()-1] == '\\' ) 
			{
				getdir = String(getdir, 0, getdir.size()-1);
			}

			// 저장될 path
			zippath = String(compresslist[i].path, m_currentDir.size()+1, compresslist[i].path.size()- m_currentDir.size()-1);

			if( compresslist[i].type == TYPE_DIRECTORY )
			{
				compressor->AddCompressFile(compresslist[i].path, zippath);
			}
			else if( compresslist[i].type == TYPE_FILE )
			{
				compressor->AddCompressFile(compresslist[i].path, zippath);
			}
		}
		compressor->DoCompress(progressdialog);
		compressor->EndCompressFile();
		compressor->UnInit();
		delete compressor;

		progressdialog->Destroy();

		// 취소해서 삭제
		if( iscancel == true)
			utility::removefile(compfile.c_str(), false);

		ClearSelected();
	}


/*
	Compress *dialog = new Compress( parentwindow, wxID_ANY); 
	dialog->ShowModal();
	int ret = dialog->getResult();

	String compfile;
	bool iscancel = false;

	if( ret == ID_OK && dialog->compressfilename != L"" )
	{
		CompressProgress *progressdialog = new CompressProgress( parentwindow, wxID_ANY); 
		progressdialog->Show(true);

		int method = dialog->compressmethod;
		String password = dialog->compresspassword;

		compfile = m_currentDir + L"\\" + dialog->compressfilename + L".zip";
		progressdialog->Update(L"zipfilename", (dialog->compressfilename + L".zip").c_str());

		CZipper *zipper = new CZipper((wchar*)compfile.c_str(), NULL, FALSE, method);

		for (size_t i=0; i<compresslist.size(); i++)
		{
			String zippath;
			String getdir = utility::GetDrive(compresslist[i].path.c_str()) + utility::GetPath(compresslist[i].path.c_str());

			if( getdir[getdir.size()-1] == '\\' ) 
			{
				getdir = String(getdir, 0, getdir.size()-1);
			}

			if( m_currentDir != getdir )
			{
				zippath = String(getdir,m_currentDir.size()+1, getdir.size() - m_currentDir.size()-1) + L"/";
			}

			if( compresslist[i].type == TYPE_DIRECTORY )
			{

				zipper->OpenFileToZip((wchar*)compresslist[i].path.c_str(), (wchar*)zippath.c_str(),NULL, 0);
				zipper->CloseFileToZip();
			}
			else if( compresslist[i].type == TYPE_FILE )
			{
				if( password.size() > 0 )
				{
					unsigned long crc = utility::CalcCRC32(compresslist[i].path.c_str());
					zipper->OpenFileToZip((wchar*)compresslist[i].path.c_str(), (wchar*)zippath.c_str(), (wchar*)password.c_str(), crc);
				}
				else
				{
					zipper->OpenFileToZip((wchar*)compresslist[i].path.c_str(), (wchar*)zippath.c_str(),NULL, 0);
				}

				iscancel = DoCompress(progressdialog, zipper, compresslist[i].path, i, (int)compresslist.size());
				if( iscancel == true )
				{
					break;	// 취소
				}
				zipper->CloseFileToZip();
			}
		}
		
		delete zipper;
		progressdialog->Destroy();

		// 취소해서 삭제
		if( iscancel == true)
			utility::removefile(compfile.c_str(), false);
	}*/
}

/*
bool CDirectoryManager::DoCompress(CompressProgress *progressdialog, CZipper *zipper, String fullpath, int cfilenum, int totalfilenum)
{
	bool ret = false;

	char readbuffer[COPYBUFFERSIZE];
	__int64 len = utility::GetFileLength(fullpath.c_str());
	__int64 remainsize = len;

	FILE *infp;
	_wfopen_s(&infp, fullpath.c_str(), L"rb");

	progressdialog->Update(L"currentfile", fullpath.c_str());
	progressdialog->Update(L"filecount", unicode::format(L"(%d/%d)", cfilenum+1, totalfilenum+1));
	
	if( infp != NULL)
	{
		while(remainsize > 0)
		{
			int readsize = remainsize > COPYBUFFERSIZE ? COPYBUFFERSIZE : remainsize;
			fread(readbuffer, 1, readsize, infp);
			zipper->AddDataToZip(readbuffer, readsize);
			remainsize -= readsize;

			int progress = (int)( (float)(len-remainsize) / (float)len * 100.f);
			progressdialog->Update(L"zipprogress", unicode::format(L"%d", progress));

			int fileprogress = (int)((float)cfilenum / (float)totalfilenum * 100.f);
			progressdialog->Update(L"totalprogress", unicode::format(L"%d", fileprogress));

			int _len = (int)len;
 			progressdialog->Update(L"progresssize", unicode::format(L"(%d/%d)", (int)(len-remainsize), _len));

			if(progressdialog->iscancel)
			{
				ret = true;
				break;
			}
		}
		fclose(infp);
	}
	return ret;
}
*/

void	CDirectoryManager::DeCompressFiles(String path)
{
	bool makedir = false;
	String destdir;
	CDirectoryManager *nextdoor = ((MojoShell*)parentwindow)->GetNextDoor(whatwindow == _LEFT_WINDOW ? _RIGHT_WINDOW : _LEFT_WINDOW);
	String nextdoordir = nextdoor->GetCurrentDir();

	std::vector<String> dirlist;
	dirlist.push_back(nextdoordir);
	dirlist.push_back(GetCurrentDir());

	DestDialog dialog(parentwindow, L"압축 해제", L"압축 해제 경로", dirlist);
	if( dialog.ShowModal() == wxID_CANCEL) 
		return;

	destdir = dialog.returnstr;
	makedir = dialog.makedir;

	if( !destdir.empty() && utility::access(destdir.c_str()) == true)
	{
		UnCompressProgress *progressdialog = new UnCompressProgress(parentwindow);
		progressdialog->Show(true);

		String password;
		progressdialog->Update(L"zipfilename", path);

		Compressor *compressor = new Compressor();
		compressor->Init();
		compressor->BegineDeCompressFile(path);
		bool needpasswd = compressor->IsEncrypt();
		if( needpasswd )
		{
			progressdialog->Show(false);

			InputPasswd *inputdialog = new InputPasswd(parentwindow);
			inputdialog->ShowModal();

			if( inputdialog->ret == wxID_OK )
			{
				password = inputdialog->param.empty() == true ? L"" : inputdialog->param;
			}
			progressdialog->Show(true);
		}

		if( makedir )
		{
			destdir = destdir + L"\\" + utility::GetName(path.c_str());
			utility::makedirectory(destdir.c_str());
		}

		compressor->DoDeCompress(progressdialog, destdir, password);
		//UnZipper::Unzip(progressdialog, (wchar*)compresslist[i].path.c_str(), (wchar*)destdir.c_str(), false, password);

		compressor->UnInit();
		delete compressor;

		progressdialog->Destroy();

	}
/*
	String key = L"uncompdest";
	CDirectoryManager *nextdoor = ((MojoShell*)parentwindow)->GetNextDoor(whatwindow == _LEFT_WINDOW ? _RIGHT_WINDOW : _LEFT_WINDOW);
	String nextdoordir = nextdoor->GetCurrentDir();

	EnterTextDialog *dialog = new EnterTextDialog( parentwindow, L"압축 해제", L"압축 해제 경로", nextdoordir.c_str(), key.c_str());
	dialog->ShowModal();
	delete dialog;

	String ret =  Valueman::GetInstance()->getvalue(key.c_str());
	if( !ret.empty() && utility::access(ret.c_str()) == true)
	{
		Valueman::GetInstance()->remove(key.c_str());

		UnCompressProgress *progressdialog = new UnCompressProgress(parentwindow);
		progressdialog->Show(true);

		String destdir = ret;
		for (size_t i=0; i<compresslist.size(); i++)
		{
			if( compresslist[i].type == TYPE_FILE )
			{
				String ext = utility::GetExtention(compresslist[i].path.c_str());
				if( ext == L".zip" || ext == L".apk")
				{
					const wchar *password = NULL;
					progressdialog->Update(L"zipfilename", compresslist[i].path);

					// check need password
					int retpw = UnZipper::IsEncryptZip((wchar*)compresslist[i].path.c_str());

					if( retpw == CHECK_ZIP_ENCRYPT )
					{
						progressdialog->Show(false);

						InputPasswd *inputdialog = new InputPasswd(parentwindow);
						inputdialog->ShowModal();

						if( inputdialog->ret == wxID_OK )
						{
							password = inputdialog->param.empty() == true ? NULL : inputdialog->param.c_str();
						}

						progressdialog->Show(true);
					}

					UnZipper::Unzip(progressdialog, (wchar*)compresslist[i].path.c_str(), (wchar*)destdir.c_str(), false, password);
					progressdialog->Destroy();
				}
			}
		}
	}
*/
}

void	CDirectoryManager::ChangeAttribute()
{
	// Change Attribute
	String attributename;
	int attr = m_finalresult[m_selectedpos].m_attribute;
	if(m_finalresult[m_selectedpos].m_type == TYPE_DIRECTORY )
	{
		if( m_finalresult[m_selectedpos].m_fullpath != "")
			attributename = m_finalresult[m_selectedpos].m_fullpath;
	}
	else if(m_finalresult[m_selectedpos].m_type == TYPE_FILE )
	{
		attributename = m_finalresult[m_selectedpos].m_excutepath;
	}

	int sel = 0;
	for(int i=0; i<(int)m_finalresult.size(); i++)
	{
		if( m_finalresult[i].m_selected )
		{
			if(m_finalresult[i].m_type & TYPE_FILE)
				sel++;
		}
	}

	if(attributename != "" || sel > 0)
	{
		AttributeDialog dialog( parentwindow, wxID_ANY, attr, attributename.c_str() ); 
		dialog.ShowModal();

		int retdialog = dialog.getResult(); 
		int attributeresult = dialog.GetAttribute();

		if( retdialog == ID_OK )
		{
			utility::changeAttiribute(attributename.c_str(), attributeresult);
		}
		else if( retdialog == ID_OK_ALL)
		{
			for(int i=0; i<(int)m_finalresult.size(); i++)
			{
				if( m_finalresult[i].m_selected )
				{
					if(m_finalresult[i].m_type & TYPE_FILE)
					{
						utility::changeAttiribute(m_finalresult[i].m_excutepath.c_str(), attributeresult);
						int attr = m_finalresult[m_selectedpos].m_attribute;
						//dialog.Update(attr, m_finalresult[i].m_excutepath);
					}
				}
			}

			//dialog.Destroy();
			ClearSelected();
		}
		else if( retdialog == ID_SKIP)
		{
		}
		else if( retdialog == ID_CANCEL)
		{

		}
	}

}

// SVN 처리
void CDirectoryManager::DoSVN()
{
	SVNDialog *dialog = new SVNDialog( parentwindow ); 
	dialog->ShowModal();
}

void	CDirectoryManager::DoSVNCommand(String command)
{
	String path = ConfigManager::GetInstance()->GetTortoisePath();
	if( path == L"" )
	{
		 wxMessageBox(L"Warning!", L"SVN을 찾을 수 없습니다.", wxOK);
		 return;
	}

	if( command == L"update" )
	{
		String exec = path;
		String param = m_currentDir;

		if(exec.size() > 0)
		{
			String result = L"\"" + exec + L"\"" + L" " + L"/command:update /path:" + L"\"" + param + L"\"";
			utility::CreateProcess(result);
		}
	}
	else if( command == L"commit" )
	{
		String exec = path;
		String param = m_currentDir;

		if(exec.size() > 0)
		{
			String result = L"\"" + exec + L"\"" + L" " + L"/command:commit /path:" + L"\"" + param + L"\"";
			utility::CreateProcess(result);
		}
	}

}

void	CDirectoryManager::DoExplorer()
{
	String exe = String(L"explorer.exe") + L" " + m_currentDir;
	utility::CreateProcess(exe);
}


// 현재 상태에 따른 파일 리스트를 준비 한다
void CDirectoryManager::GetFileList(std::vector<SCANINFO> &filelist, bool deepscan)
{
	int selnum = 0;
	for(int i=0; i<(int)m_finalresult.size(); i++)
	{
		if( m_finalresult[i].m_selected )
		{
			// 선택된 것들 있다..
			if(m_finalresult[i].m_type & TYPE_DIRECTORY)
			{
				// 하윆까지 스캔이 필요
				if( deepscan )
				{
					int totalnum = 0;
					std::vector<SCANINFO> scaninfolist;
					bool cancel = Scandirectory( m_finalresult[i].m_fullpath.c_str(), totalnum , scaninfolist, true);

					for(size_t j=0; j<scaninfolist.size(); j++)
					{
						if( scaninfolist[j].type == TYPE_FILE )
						{
							filelist.push_back(scaninfolist[j]);
							selnum++;
						}
					}
				}
				else
				{
					SCANINFO info;
					info.type = TYPE_DIRECTORY;
					info.path = m_finalresult[i].m_fullpath;
					filelist.push_back(info);
				}

			}
			else if(m_finalresult[i].m_type & TYPE_FILE)
			{
				SCANINFO info;
				info.type = TYPE_FILE;
				info.path = m_finalresult[i].m_excutepath;
				info.creationtime = m_finalresult[i].m_creationtime;
				info.LastAccessTime = m_finalresult[i].m_LastAccessTime;
				info.LastWriteTime = m_finalresult[i].m_LastWriteTime;

				filelist.push_back(info);
				selnum++;
			}
		}
	}

	if(selnum == 0)
	{
		// 현재의 것 압축
		if( m_finalresult[m_selectedpos].m_type & TYPE_DIRECTORY || m_finalresult[m_selectedpos].m_type & TYPE_FILE )
		{
			if(m_finalresult[m_selectedpos].m_type & TYPE_DIRECTORY)
			{
				if( deepscan )
				{
					int totalnum = 0;
					std::vector<SCANINFO> scaninfolist;
					bool cancel = Scandirectory( m_finalresult[m_selectedpos].m_fullpath.c_str(), totalnum , scaninfolist, true);

					for(size_t j=0; j<scaninfolist.size(); j++)
					{
						if( scaninfolist[j].type == TYPE_FILE || scaninfolist[j].type == TYPE_DIRECTORY )
						{
							filelist.push_back(scaninfolist[j]);
						}
					}
				}

				SCANINFO info;
				info.type = TYPE_DIRECTORY;
				info.path = m_finalresult[m_selectedpos].m_fullpath;
				filelist.push_back(info);
			}
			else if(m_finalresult[m_selectedpos].m_type & TYPE_FILE)
			{
				SCANINFO info;
				info.type = TYPE_FILE;
				info.path = m_finalresult[m_selectedpos].m_excutepath;
				info.creationtime = m_finalresult[m_selectedpos].m_creationtime;
				info.LastAccessTime = m_finalresult[m_selectedpos].m_LastAccessTime;
				info.LastWriteTime = m_finalresult[m_selectedpos].m_LastWriteTime;
				filelist.push_back(info);						
			}
		}
	}
}

void	CDirectoryManager::SelectImage()
{
	String imagepreview = ConfigManager::GetInstance()->GetCommonOption(L"imagepreview");
	if( imagepreview == L"true")
	{
		if( m_finalresult[m_selectedpos].m_type == TYPE_FILE )
		{
			String fullpath = m_finalresult[m_selectedpos].m_excutepath;
			ImageLoader::GetInstance()->LoadImage(fullpath);
		}
		else
		{
			ImageLoader::GetInstance()->Reset();
		}
	}
}


// 유효한 디렉토리인가 검사 / 유효하지 않으면 가능한 디렉토리 리턴
wchar *	CDirectoryManager::CheckValidDirectory(String path)
{
	static String checkpath;
	checkpath.clear();

	std::vector<String> tokens;
	unicode::Tokenize(path, tokens, L"\\");

	int n = (int)tokens.size();
	while(n)
	{
		for (int i=0; i<n; i++)
		{
			checkpath += tokens[i] + (i+1<n?L"\\":L"");
		}

		bool ret = utility::access(checkpath.c_str());
		if( ret )
		{
			return (wchar*)checkpath.c_str();
		}

		checkpath.clear();
		n--;
	}

	checkpath = L"C:";
	return (wchar*)checkpath.c_str();
}

void	CDirectoryManager::RemoveFromList(String path)
{
//	_TRACE(L"remove %s\n", path.c_str());

	std::vector<FILEINFO>::iterator itdir =	m_directoryinfolist.begin();
	for(; itdir != m_directoryinfolist.end(); itdir++)
	{
		if( itdir->m_fullpath == path )
		{
			m_directoryinfolist.erase(itdir);
			break;
		}
	}

	std::vector<FILEINFO>::iterator itf = m_fileinfolist.begin();
	for(; itf != m_fileinfolist.end(); itf++)
	{
		if( itf->m_excutepath == path )
		{
			m_fileinfolist.erase(itf);
			break;
		}
	}

	std::vector<FILEINFO>::iterator itfile = m_finalresult.begin();
	for(; itfile != m_finalresult.end(); itfile++)
	{
		if( itfile->m_type == TYPE_DIRECTORY )
		{
			if( itfile->m_fullpath == path )
			{
				m_finalresult.erase(itfile);
				break;
			}
		}
		else if( itfile->m_type == TYPE_FILE )
		{
			if( itfile->m_excutepath == path )
			{
				m_finalresult.erase(itfile);
				break;
			}
		}
	}
}

void	CDirectoryManager::EditInfoFromList(String path, FILEINFO *info)
{
	std::vector<FILEINFO>::iterator itdir =	m_directoryinfolist.begin();
	for(; itdir != m_directoryinfolist.end(); itdir++)
	{
		if( itdir->m_fullpath == path )
		{
			itdir->m_attribute = info->m_attribute;
			itdir->m_creationtime = info->m_creationtime;
			itdir->m_LastAccessTime = info->m_LastAccessTime;
			itdir->m_LastWriteTime = info->m_LastWriteTime;
			//itdir->m_filesize = info->m_filesize;
			break;
		}
	}

	std::vector<FILEINFO>::iterator itf = m_fileinfolist.begin();
	for(; itf != m_fileinfolist.end(); itf++)
	{
		if( itf->m_excutepath == path )
		{
			itf->m_attribute = info->m_attribute;
			itf->m_creationtime = info->m_creationtime;
			itf->m_LastAccessTime = info->m_LastAccessTime;
			itf->m_LastWriteTime = info->m_LastWriteTime;
			itf->m_filesize = info->m_filesize;
			break;
		}
	}

	std::vector<FILEINFO>::iterator itfile = m_finalresult.begin();
	for(; itfile != m_finalresult.end(); itfile++)
	{
		if( itfile->m_type == TYPE_DIRECTORY )
		{
			if( itfile->m_fullpath == path )
			{
				itfile->m_attribute = info->m_attribute;
				itfile->m_creationtime = info->m_creationtime;
				itfile->m_LastAccessTime = info->m_LastAccessTime;
				itfile->m_LastWriteTime = info->m_LastWriteTime;
				itfile->m_filesize = info->m_filesize;
				break;
			}
		}
		else if( itfile->m_type == TYPE_FILE )
		{
			if( itfile->m_excutepath == path )
			{
				itfile->m_attribute = info->m_attribute;
				itfile->m_creationtime = info->m_creationtime;
				itfile->m_LastAccessTime = info->m_LastAccessTime;
				itfile->m_LastWriteTime = info->m_LastWriteTime;
				itfile->m_filesize = info->m_filesize;
				break;
			}
		}
	}
}

void	CDirectoryManager::ReplacePath(String oldpath, String oldname, String newpath, String newname)
{
	std::vector<FILEINFO>::iterator itdir =	m_directoryinfolist.begin();
	for(; itdir != m_directoryinfolist.end(); itdir++)
	{
		if( itdir->m_fullpath == oldpath )
		{
			itdir->m_name = newname;
			itdir->m_fullpath = newpath;
			break;
		}
	}

	// 파일
	std::vector<FILEINFO>::iterator itf = m_fileinfolist.begin();
	for(; itf != m_fileinfolist.end(); itf++)
	{
		if( itf->m_excutepath == oldpath )
		{
			String name, ext;
 			name = utility::GetName(newpath.c_str());
 			ext = utility::GetExtention(newpath.c_str());
			ext.erase(ext.begin());

 			itf->m_name = name;
			itf->m_ext = ext;
 			itf->m_fullpath = utility::GetDrive(newpath.c_str()) + utility::GetPath(newpath.c_str());
 			itf->m_excutepath = newpath;
			break;
		}
	}

	std::vector<FILEINFO>::iterator itfile = m_finalresult.begin();
	for(; itfile != m_finalresult.end(); itfile++)
	{
		if( itfile->m_type == TYPE_DIRECTORY )
		{
			if( itfile->m_fullpath == oldpath )
			{
				itfile->m_name = newname;
				itfile->m_fullpath = newpath;
				break;
			}
		}
		else if( itfile->m_type == TYPE_FILE )
		{
			if( itfile->m_excutepath == oldpath )
			{
				String name, ext;
				name = utility::GetName(newpath.c_str());
				ext = utility::GetExtention(newpath.c_str());
				ext.erase(ext.begin());

				itfile->m_name = name;
				itfile->m_ext = ext;
				itfile->m_fullpath = utility::GetDrive(newpath.c_str()) + utility::GetPath(newpath.c_str());
				itfile->m_excutepath = newpath;
				break;
			}
		}
	}
}
