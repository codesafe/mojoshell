#ifndef _DIRECTORY_MANAGER_H_
#define _DIRECTORY_MANAGER_H_

#include "predef.h"
#include "canvas.h"
#include "customdialog.h"
#include "ui.h"
#include "lock.h"
//#include "./zip/Zipper.h"
//#include "./zip/UnZipper.h"

#define LINE_HEIGHT		15
//#define COPYBUFFERSIZE	1024 * 32
#define COPYBUFFERSIZE	16384
#define SEPARATE_WIDTH	6

#define KEYSCANCODE	512


//class FileChecker;
class FileMonitor;

struct  SCANINFO
{
	int		type;
	String	path;
	String	destpath;
	FILETIME	creationtime;
	FILETIME	LastAccessTime;
	FILETIME	LastWriteTime;
};

struct SIMPLEINFO
{
	int		type;
	String	name;
	String	ext;
} ;



class CDirectoryManager
{
public :
	enum
	{
		_LEFT_WINDOW,
		_RIGHT_WINDOW,
		_MAX_WINDOW
	};

	enum
	{
		_DISPLAY_CAPITAL	= 1 << 1,	// 대문자
		_DISPLAY_HIDDEN		= 1 << 2 ,	// 숨김 보이기

	};

	enum _SORTMETHOD
	{
		_SORT_NAME_MIN,		// 알파벳순 작은거 부터
		_SORT_NAME_MAX,
		_SORT_TIME_MIN,		// 생성 시간별
		_SORT_TIME_MAX,		
		_SORT_SIZE_MIN,		// 크기별
		_SORT_SIZE_MAX,		
	};

	enum _KEYBOARD_ACTION
	{
		_KEY_DOWN,
		_KEY_UP,
		_KEY_CLEAR,
		_KEY_KEEP,
		_KEY_ACTION_NONE
	};

	enum _MOUSECLICK
	{
		_MOUSE_MOVE,
		_MOUSE_CLICK_LEFT,
		_MOUSE_CLICK_RIGHT,
		_MOUSE_DCLICK_LEFT,
		_MOUSE_MOVE_DROP,
		_MOUSE_CLICK_DROP,
	};

	int		keymodifyer;

public :
	CDirectoryManager();
	~CDirectoryManager();

	void	Init(wxFrame *p, int what,CINI *ini);
	void	CollectDriveinfo();
	void	CollectCurrentDirInfo();
	void	AnalizeDirectory(const wchar_t *path);
	void	ResetParam();
	void	ClearSelected();

	void	Up();
	void	Down();
	void	Home();
	void	End();

	void	PressKeyboard(int action, wxKeyEvent& event);

	void	RenderContents(wxPoint p, wxSize size, Canvas *canvas);
	void	DrawDirinfo(wxPoint p, wxSize size, Canvas *canvas);
	void	DrawFileinfo(wxPoint p, wxSize size, Canvas *c);
	void	DrawFile(wxPoint p, wxSize size, Canvas *c);
	void	DrawSeparate(int x, wxSize size, Canvas *canvas);
	void	DrawImage(wxPoint p, wxSize size, Canvas *c);


	void	SetWindowActive(bool t);
	void	SetWindowSize(wxSize size) { m_windowSize = size; }
	int		MouseClick(wxPoint pos, int x, int y, int which);
	void	MouseWheel(int delta);

	void	RefreshDirectory();	
	void	SendMonitorInfo(unsigned long action, wchar *monitorpath, wchar *filename_0, wchar *filename_1);
	void	DropFiles(int x, int y, const wxArrayString& filenames);

	String	GetCurrentDir(){ return m_currentDir; }
	void	GetDragFiles(std::vector<String> &lists);

private :

	void	SelectImage();

	wchar*	CheckValidDirectory(String path);
	void	ChooseBestPos(String lastdir);

	void	Pagedown();
	void	Pageup();
	void	BackToRoot();
	void	MoveToBackDir();
	void	CalcDisplayInfo();
	void	SavePathinfo();

	void	ChangeDrive(const wchar *drive);
	void	Deletefile(bool recyclebin);
	void	DeleteSelectedFile(bool recyclebin);
	void	MakeDirectory();
	void	Rename();
	void	ExcuteInput();

	void	Copy();
	void	CopyDirectory(String dest, std::vector<SCANINFO> &scaninfolist, bool convert = true);
	void	PasteFiles(const wxArrayString &files, String destpath);

	void	Move();
	void	MoveFiles(const std::vector<String> &filelist, String destpath);

	bool	DeleteDirectory(const wchar *lpFolder, DeleteDialog *dialog,bool recyclebin, int totalnum, int &progress);

	bool	Scandirectory(const wchar *path, int &count, std::vector<SCANINFO> &scaninfolist, bool showdialog = false);
	bool	_Scandirectory(const wchar *path, int &count, std::vector<SCANINFO> &scaninfolist, ScanDialog *dialog);

	void	StartChecker();
	void	StopChecker();

	void	SortList();
	int		ActionCopy(const wchar *src, const wchar *dest, int progress1, int totalprogress1, CopyDialog *dialog, unsigned long option = 0xffffffff);
	void	ActionCur(bool param=false);

	void	ActionSetFileTime(const wchar *filename, FILETIME *ctime, FILETIME *latime, FILETIME *lwtime);
	void	ActionSetFileAttribute(const wchar *filename, unsigned long attriute);

	bool	ActionKey(int action, int keycode, int modfier);
	bool	ActionSHIFTKey(int action, int keycode, int modfier);
	bool	ActionCTRLKey(int action, int keycode, int modfier);
	bool	ActionALTKey(int action, int keycode, int modfier);

	void	SaveSortInfo();
	void	CompressFiles(std::vector<SCANINFO> &compresslist);
	//bool	DoCompress(CompressProgress *progressdialog, CZipper *zipper, String fullpath, int cfilenum, int totalfilenum);
	void	DeCompressFiles(String path);
	void	ChangeAttribute();

	void	DoSVN();
	void	DoSVNCommand(String command);

	void	DoExplorer();

	void	GetFileList(std::vector<SCANINFO> &filelist, bool deepscan);

	static DWORD CALLBACK progressRoutine(
		LARGE_INTEGER TotalFileSize,
		LARGE_INTEGER TotalBytesTransferred,
		LARGE_INTEGER StreamSize,
		LARGE_INTEGER StreamBytesTransferred,
		DWORD dwStreamNumber,
		DWORD dwCallbackReason,
		HANDLE hSourceFile,
		HANDLE hDestinationFile,
		LPVOID lpData
		);

	FILEINFO				m_parent;	// .. 표시

	String					m_currentDir;
	std::vector<String>		m_filelists;

	std::vector<FILEINFO>	m_driveinfolist;
	std::vector<FILEINFO>	m_directoryinfolist;
	std::vector<FILEINFO>	m_fileinfolist;

	void	EditInfoFromList(String path, FILEINFO *info);
	void	RemoveFromList(String path);
	void	ReplacePath(String oldpath, String oldname, String newpath, String newname);

	//////////////////////////////////////////////////////////////////////////

	void	ResetLastSelect();
	void	SetLastSelect();
	void	SetLastSelectInPos(int pos);
	void	RestoreLastSelect();
	void	DoQuickSearch();
	void	RollQuickSearchFront();
	void	RollQuickSearchBack();
	void	ResetQuickSearch() 
	{
		quicksearch.clear();
		quicksearchlist.clear();
	}

	SIMPLEINFO	lastselectinfo;

	//////////////////////////////////////////////////////////////////////////
	
	int						m_displayoption;
	int						m_sortmethod;

	wxSize					m_windowSize;

	int						m_displaytop;
	int						m_displaylines;

	int						m_selectedpos;
	int						m_currentdisplaypos;


	// 디스플레이될 결과 모음
	std::vector<FILEINFO>	m_finalresult;

	CINI		*pathinfo_ini;

	String		quicksearch;
	unsigned long quicksearchTime;

	std::list<SIMPLEINFO>		quicksearchlist;

	//////////////////////////////////////////////////////////////////////////

	CCriticalSection	filecheckerlock;
	//FileChecker*	filechecker;
	FileMonitor*	filemonitor; 


	bool			activewindow;	// 활성화?
	int				whatwindow;		// 내가 왼쪽? 오른쪽?
	wxFrame*		parentwindow;

	wxBitmap		*selectbitmap;

	int				lastselectline;
	//String			dropTargetdir;

};



#endif