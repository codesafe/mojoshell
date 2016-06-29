#include "main.h"
#include "canvas.h"
#include "trace.h"
#include "shellwindow.h"
#include "configmanager.h"
#include "colormanager.h"
#include "imageloader.h"
#include "ftp.h"
#include "toolbar.h"

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_APP(CApp)

bool CApp::OnInit()
{
	//_CrtSetBreakAlloc(2905);
	//timeBeginPeriod(1);
	srand(timeGetTime());
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	wxInitAllImageHandlers();
	wxLog::SetLogLevel(0);

	wxSize	ssize(MOJOWINDOW_SIZEX, MOJOWINDOW_SIZEY);
	wxPoint pos(MOJOWINDOW_POS_X , MOJOWINDOW_POS_Y);
	frame = new MojoShell(_T("Mojo Shell"), wxID_ANY, pos, ssize);;
	frame->SetIcon(wxICON(MOJOSHELL_ICON));
 
	//wxTheClipboard->UsePrimarySelection();

	//frame->Centre();
	frame->Show(TRUE);

	SetTopWindow(frame);
	return true;
}

int CApp::OnExit()
{
	ImageLoader::GetInstance()->Close();

	//delete frame;

	//timeEndPeriod(1);
	//CoUninitialize();
	return 0;
}


//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MojoShell, wxFrame)

	EVT_CLOSE(MojoShell::OnClose)
	EVT_SIZE(MojoShell::OnSize)
	EVT_MOVE(MojoShell::OnMove)
	EVT_PAINT(MojoShell::OnPaint)
	EVT_SET_FOCUS(MojoShell::OnSetFocus)
	EVT_KILL_FOCUS(MojoShell::OnKillFocus)

	EVT_KEY_DOWN( MojoShell::OnKeyDown )
	EVT_KEY_UP( MojoShell::OnKeyUp )
	EVT_LEFT_DOWN( MojoShell::OnLeftClick )
	EVT_LEFT_UP( MojoShell::OnLeftClickUp )
	EVT_RIGHT_DOWN( MojoShell::OnRightClick )
	EVT_LEFT_DCLICK( MojoShell::OnLeftDClick )
	EVT_MOUSEWHEEL( MojoShell::OnMouseWheel)
	EVT_MOTION( MojoShell::OnMouseMove)

	EVT_ERASE_BACKGROUND(MojoShell::OnEraseBackGround)
	EVT_DROP_FILES(MojoShell::OnDropFile)

END_EVENT_TABLE()

MojoShell::MojoShell(const wxString& title, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxFrame(NULL, id, title, pos, size, style),
m_backBuffer(NULL)
{
	bool ret = ConfigManager::GetInstance()->Open();

	if( ret )
	{
		m_windowsize.x = ConfigManager::GetInstance()->GetConfigInt(L"sizex") > 0 ? ConfigManager::GetInstance()->GetConfigInt(L"sizex") : MOJOWINDOW_SIZEX;
		m_windowsize.y = ConfigManager::GetInstance()->GetConfigInt(L"sizey") > 0 ? ConfigManager::GetInstance()->GetConfigInt(L"sizey") : MOJOWINDOW_SIZEY;
		m_windowpos.x = ConfigManager::GetInstance()->GetConfigInt(L"posx") > 0 ? ConfigManager::GetInstance()->GetConfigInt(L"posx") : 0;
		m_windowpos.y = ConfigManager::GetInstance()->GetConfigInt(L"posy") > 0 ? ConfigManager::GetInstance()->GetConfigInt(L"posy") : 0;
	}
	else
	{
		m_windowsize.x = MOJOWINDOW_SIZEX;
		m_windowsize.y = MOJOWINDOW_SIZEY;
		m_windowpos.x = 0;
		m_windowpos.y = 0;
	}

	pathinfo_ini.LoadINI(L"pathinfo.ini");

	m_directoryMan[_LEFT_WINDOW].Init(this, _LEFT_WINDOW, &pathinfo_ini);
	m_directoryMan[_RIGHT_WINDOW].Init(this, _RIGHT_WINDOW, &pathinfo_ini);

	m_currentselectwindow = _LEFT_WINDOW;
	m_directoryMan[m_currentselectwindow].SetWindowActive(true);

	menu_bar = new wxMenuBar;
	menu_file = new wxMenu;
	menu_folder = new wxMenu;
	menu_edit = new wxMenu;
	menu_compress = new wxMenu;
	menu_option = new wxMenu;
	menu_help = new wxMenu;
	menu_view = new wxMenu;

	//////////////////////////////////////////////////////////////////////////	FILE

	menu_file->Append(ID_MENU_OPEN, wxT("열기/실행 (Enter)"));
	menu_file->Append(ID_MENU_OPEN_OPTION, wxT("옵션과 함께 열기 (Ctrl-Enter)"));
	menu_file->Append(ID_MENU_COPY, wxT("파일 복사 (Alt-C)"));
	menu_file->Append(ID_MENU_MOVE, wxT("파일 이동 (Alt-M)"));
	menu_file->AppendSeparator();
	menu_file->Append(ID_MENU_TRASHBIN, wxT("파일 휴지통에 버리기 (Delete)"));
	menu_file->Append(ID_MENU_DELETE, wxT("파일 바로 삭제 (Shift-Delete)"));
	menu_file->AppendSeparator();
	menu_file->Append(ID_MENU_RENAME, wxT("파일 이름 바꾸기 (Alt-R)"));
	menu_file->Append(ID_MENU_ATTRIBUTE, wxT("파일 속성 바꾸기 (Ctrl-Z)"));
	menu_file->AppendSeparator();
	menu_file->Append(wxID_EXIT, wxT("종료(&Q)"));

	Connect(ID_MENU_OPEN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(ID_MENU_OPEN_OPTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(ID_MENU_COPY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(ID_MENU_MOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(ID_MENU_TRASHBIN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(ID_MENU_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(ID_MENU_RENAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(ID_MENU_ATTRIBUTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnQuit));

	////////////////////////////////////////////////////////////////////////// FOLDER

	menu_folder->Append(ID_FOLDER_MAKE, wxT("새 폴더 만들기 (Alt-K)"));
	menu_folder->AppendSeparator();
	menu_folder->Append(ID_MENU_COPY, wxT("폴더 복사 (Alt-C)"));
	menu_folder->Append(ID_MENU_MOVE, wxT("폴더 이동 (Alt-M)"));
	menu_folder->AppendSeparator();
	menu_folder->Append(ID_MENU_TRASHBIN, wxT("폴더 휴지통에 버리기 (Delete)"));
	menu_folder->Append(ID_MENU_DELETE, wxT("폴더 바로 삭제 (Shift-Delete)"));
	menu_folder->AppendSeparator();
	menu_folder->Append(ID_MENU_RENAME, wxT("폴더 이름 바꾸기 (Alt-R)"));
	menu_folder->AppendSeparator();
	menu_folder->Append(ID_FOLDER_MOVE_PARENT, wxT("부모 폴더로 이동 (.)"));
	menu_folder->Append(ID_FOLDER_MOVE_ROOT, wxT("최상위 폴더로 이동 (\\)"));

	Connect(ID_FOLDER_MAKE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFolderCommand));
	Connect(ID_MENU_COPY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(ID_MENU_MOVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(ID_MENU_TRASHBIN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(ID_MENU_DELETE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(ID_MENU_RENAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFileCommand));
	Connect(ID_FOLDER_MOVE_PARENT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFolderCommand));
	Connect(ID_FOLDER_MOVE_ROOT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFolderCommand));

	////////////////////////////////////////////////////////////////////////// EDIT

	menu_edit->Append(ID_EDIT_COPY, wxT("클립보드에 복사 (Ctrl-C)"));
	menu_edit->Append(ID_EDIT_PASTE, wxT("클립보드 내용 붙여넣기 (Ctrl-V)"));
	menu_edit->AppendSeparator();
	menu_edit->Append(ID_EDIT_SELECT_SINGLE, wxT("선택/해제 (Space)"));
	menu_edit->Append(ID_EDIT_SELECT_ALL, wxT("전체 선택 (Num *)"));
	menu_edit->Append(ID_EDIT_SELECT_SAME, wxT("현재 위치와 같은 종류 선택 (Num /)"));
	menu_edit->Append(ID_EDIT_SELECT_ADV, wxT("고급 선택 (Num +)"));
	menu_edit->AppendSeparator();
	menu_edit->Append(ID_EDIT_COMMAND, wxT("직접 명령 실행 (/)"));
	menu_edit->AppendSeparator();
	menu_edit->Append(ID_EDIT_EXPLORER, wxT("현재폴더 탐색기로 열기 (Ctrl-E)"));

	Connect(ID_EDIT_COPY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnEditCommand));
	Connect(ID_EDIT_PASTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnEditCommand));
	Connect(ID_EDIT_SELECT_SINGLE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnEditCommand));
	Connect(ID_EDIT_SELECT_ALL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnEditCommand));
	Connect(ID_EDIT_SELECT_SAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnEditCommand));
	Connect(ID_EDIT_SELECT_ADV, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnEditCommand));
	Connect(ID_EDIT_COMMAND, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnEditCommand));
	Connect(ID_EDIT_EXPLORER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnEditCommand));

	////////////////////////////////////////////////////////////////////////// VIEW

	menu_view->Append(ID_VIEW_SORT_NAME, wxT("이름 순서로 정렬(토글) (Alt-E)"));
	menu_view->Append(ID_VIEW_SORT_TIME, wxT("시간 순서로 정렬(토글) (Alt-T)"));
	menu_view->Append(ID_VIEW_SORT_SIZE, wxT("크기 순서로 정렬(토글) (Alt-S)"));
	menu_view->AppendSeparator();
	menu_view->Append(ID_VIEW_HIDDEN, wxT("숨김속성 보기/감추기(토글) (Alt-Z)"));

	Connect(ID_VIEW_SORT_NAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnViewCommand));
	Connect(ID_VIEW_SORT_TIME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnViewCommand));
	Connect(ID_VIEW_SORT_SIZE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnViewCommand));
	Connect(ID_VIEW_HIDDEN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnViewCommand));

	////////////////////////////////////////////////////////////////////////// COMPRESS

	menu_compress->Append(ID_COMPRESS_ZIP, wxT("압축하기 (Ctrl-A)"));
	menu_compress->Append(ID_COMPRESS_UNZIP, wxT("압축풀기 (Ctrl-X)"));

	Connect(ID_COMPRESS_ZIP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnCompressCommand));
	Connect(ID_COMPRESS_UNZIP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnCompressCommand));

	////////////////////////////////////////////////////////////////////////// OPTION

	menu_option->Append(ID_OPTION_OPTION, wxT("일반 설정"));
	//menu_option->Append(ID_OPTION_FUNCTION, wxT("펑션키 설정"));

	Connect(ID_OPTION_OPTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnNormalOption));
	//Connect(ID_OPTION_FUNCTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnFunctionKey));

	////////////////////////////////////////////////////////////////////////// HELP

	menu_help->Append(ID_HELP_HELP, wxT("도움말"));
	menu_help->Append(ID_HELP_HOTKEY, wxT("단축키 안내"));
	menu_help->AppendSeparator();
	menu_help->Append(ID_HELP_UPDATE, wxT("업데이트 검사"));
	menu_help->AppendSeparator();
	menu_help->Append(ID_HELP_ABOUT, wxT("Mojo Shell 정보"));

	Connect(ID_HELP_HELP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnHelpHelp));
	Connect(ID_HELP_HOTKEY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnHelpNotkey));
	Connect(ID_HELP_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnHelpUpdate));
	Connect(ID_HELP_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnHelpAbout));

	//////////////////////////////////////////////////////////////////////////

	// Tool bar

	wxBitmap svnupdate(wxT("tortoisesvn-update.png"), wxBITMAP_TYPE_PNG);
	wxBitmap svncommit(wxT("tortoisesvn-commit.png"), wxBITMAP_TYPE_PNG);
	wxBitmap explore(wxT("icon_explore.png"), wxBITMAP_TYPE_PNG);
	wxBitmap option(wxT("icon_option.png"), wxBITMAP_TYPE_PNG);

	wxToolBar *toolbar = CreateToolBar();
	//toolbar->SetToolBitmapSize( wxSize( 16, 16 ) );

	toolbar->AddSeparator();
	toolbar->AddTool(ID_SVN_MENU_UPDATE, wxT("SVN Update"), svnupdate, wxT("SVN Update"));
	toolbar->AddTool(ID_SVN_MENU_COMMIT, wxT("SVN Commit"), svncommit, wxT("SVN Commit"));
	toolbar->AddSeparator();
	toolbar->AddTool(ID_SVN_MENU_EXPLORE, wxT("Open Explore"), explore, wxT("Open Explore"));
	toolbar->AddTool(ID_SVN_MENU_OPTION, wxT("Option"), option, wxT("SVN Option"));

	toolbar->Realize();

	Connect(ID_SVN_MENU_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnSVNMenuCommand));
	Connect(ID_SVN_MENU_COMMIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnSVNMenuCommand));
	Connect(ID_SVN_MENU_EXPLORE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnSVNMenuCommand));
	Connect(ID_SVN_MENU_OPTION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MojoShell::OnSVNMenuCommand));

	//////////////////////////////////////////////////////////////////////////

	menu_bar->Append(menu_file, wxT("파일(&F)"));
	menu_bar->Append(menu_folder, wxT("폴더(&O)"));
	menu_bar->Append(menu_edit, wxT("편집(&E)"));
	menu_bar->Append(menu_view, wxT("보기(&V)"));
	menu_bar->Append(menu_compress, wxT("압축(&A)"));
	menu_bar->Append(menu_option, wxT("옵션(&O)"));
	menu_bar->Append(menu_help, wxT("도움말(&H)"));

	SetMenuBar(menu_bar);

	SetSize(m_windowsize);
	//SetClientSize(m_windowsize);
	SetPosition(m_windowpos);
	//SetIcon(wxICON(mondrian));

	mainHwhd = wxWindow::GetHWND();
	leftmousedown = false;

//	DragAcceptFiles(true);
	SetDropTarget(this);

	lastselectedwindow = (int)_LEFT_WINDOW;
	getdrag = false;

	ImageLoader::GetInstance()->Init();

	clickSeparate = false;
	separatePos = -1;

	//Ftp::GetInstance()->Open(L"10.30.168.160",L"codesafe",L"6502");
	//SetCursor(wxCursor(wxCURSOR_SIZEWE));
}


MojoShell::~MojoShell()
{
}

void	MojoShell::KeyBoardSimul(int key, int att)
{
	wxKeyEvent keyevent;
	keyevent.m_keyCode = key;
	if( att & _KEY_CTRL )
	{
		keyevent.SetControlDown(true);
	}
	else if( att & _KEY_ALT )
	{
		keyevent.SetAltDown(true);
	}
	else if( att & _KEY_SHIFT )
	{
		keyevent.SetShiftDown(true);
	}

	OnKeyDown(keyevent);
}

void	MojoShell::OnFileCommand(wxCommandEvent &event)
{
	int id = event.GetId();
	switch (id)
	{
		case ID_MENU_OPEN :
				KeyBoardSimul(WXK_RETURN, _KEY_NONE);
			break;
		case ID_MENU_OPEN_OPTION :
				KeyBoardSimul(WXK_RETURN, _KEY_CTRL);
			break;
		case ID_MENU_COPY :
				KeyBoardSimul('C', _KEY_ALT);
			break;
		case ID_MENU_MOVE :
				KeyBoardSimul('M', _KEY_ALT);
			break;
		case ID_MENU_TRASHBIN :
				KeyBoardSimul(WXK_DELETE, _KEY_NONE);
			break;
		case ID_MENU_DELETE :
				KeyBoardSimul(WXK_DELETE, _KEY_SHIFT);
			break;
		case ID_MENU_RENAME :
				KeyBoardSimul('R', _KEY_ALT);
			break;
		case ID_MENU_ATTRIBUTE :
				KeyBoardSimul('Z', _KEY_CTRL);
			break;
	}
}

void	MojoShell::OnFolderCommand(wxCommandEvent &event)
{
	int id = event.GetId();
	switch (id)
	{
		case ID_FOLDER_MAKE :
				KeyBoardSimul('K', _KEY_ALT);
			break;
		case ID_FOLDER_MOVE_PARENT :
				KeyBoardSimul('.', _KEY_NONE);
			break;
		case ID_FOLDER_MOVE_ROOT :
				KeyBoardSimul('\\', _KEY_NONE);
			break;
	}
}

void	MojoShell::OnEditCommand(wxCommandEvent &event)
{
	int id = event.GetId();
	switch (id)
	{
		case ID_EDIT_COPY :
				KeyBoardSimul('C', _KEY_CTRL);
			break;
		case ID_EDIT_PASTE :
				KeyBoardSimul('V', _KEY_CTRL);
			break;
		case ID_EDIT_SELECT_SINGLE :
				KeyBoardSimul(WXK_SPACE, _KEY_NONE);
			break;
		case ID_EDIT_SELECT_ALL :
				KeyBoardSimul(WXK_NUMPAD_MULTIPLY, _KEY_NONE);
			break;
		case ID_EDIT_SELECT_SAME :
				KeyBoardSimul(WXK_NUMPAD_DIVIDE, _KEY_NONE);
			break;
		case ID_EDIT_SELECT_ADV :
			break;
		case ID_EDIT_COMMAND :
				KeyBoardSimul('/', _KEY_NONE);
			break;
		case ID_EDIT_EXPLORER :
				KeyBoardSimul('E', _KEY_CTRL);
			break;
	}
}

void	MojoShell::OnViewCommand(wxCommandEvent &event)
{
	int id = event.GetId();
	switch (id)
	{
		case ID_VIEW_SORT_NAME :
				KeyBoardSimul('E', _KEY_ALT);
			break;

		case ID_VIEW_SORT_TIME :
				KeyBoardSimul('T', _KEY_ALT);
			break;

		case ID_VIEW_SORT_SIZE :
				KeyBoardSimul('S', _KEY_ALT);
			break;
		case ID_VIEW_HIDDEN :
				KeyBoardSimul('Z', _KEY_ALT);
			break;
	}
}

void	MojoShell::OnCompressCommand(wxCommandEvent &event)
{
	int id = event.GetId();
	switch (id)
	{
		case ID_COMPRESS_ZIP :
				KeyBoardSimul('A', _KEY_CTRL);
			break;

		case ID_COMPRESS_UNZIP :
				KeyBoardSimul('X', _KEY_CTRL);
			break;
	}
}

void	MojoShell::OnFunctionKey(wxCommandEvent &WXUNUSED(event))
{
	Function dialog(this, wxID_ANY); 
	dialog.ShowModal();
}

void	MojoShell::OnNormalOption(wxCommandEvent &WXUNUSED(event))
{
	Options dialog(this, wxID_ANY); 
	dialog.ShowModal();
	if( dialog.ret == wxID_OK )
	{
		ColorManager::GetInstance()->SaveXML();
	}
}

// Quit from Menu
void MojoShell::OnQuit(wxCommandEvent &WXUNUSED(event))
{
	//wxPuts(wxT("Exit!\n"));
	Close(true);
}

void	MojoShell::OnHelpHelp(wxCommandEvent &WXUNUSED(event))
{

}

void	MojoShell::OnHelpNotkey(wxCommandEvent &WXUNUSED(event))
{

}

void	MojoShell::OnHelpUpdate(wxCommandEvent &WXUNUSED(event))
{

}

void	MojoShell::OnHelpAbout(wxCommandEvent &WXUNUSED(event))
{
	AboutDialog *dialog = new AboutDialog( this, wxID_ANY); 
	dialog->ShowModal();
}

void	MojoShell::OnSVNMenuCommand(wxCommandEvent &event)
{
	int id = event.GetId();
	switch (id)
	{
		case ID_SVN_MENU_UPDATE :
			KeyBoardSimul('L', _KEY_CTRL);
			break;

		case ID_SVN_MENU_COMMIT :
			KeyBoardSimul('P', _KEY_CTRL);
			break;

		case ID_SVN_MENU_EXPLORE :
			KeyBoardSimul('E', _KEY_CTRL);
			break;

		case ID_SVN_MENU_OPTION :
			//KeyBoardSimul('P', _KEY_CTRL);
			break;
	}
}


//////////////////////////////////////////////////////////////////////////

void	MojoShell::OnLeave()
{
// 	getdrag = false;
// 	leftmousedown = false;
}

/*
wxDragResult	MojoShell::OnEnter(wxCoord x, wxCoord y, wxDragResult def)
{
	return OnDragOver2(x,y,def);
}

wxDragResult	MojoShell::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
	return def;
}
*/

void MojoShell::OnDropFile(wxDropFilesEvent &event)
{
	wxPoint p = event.GetPosition();
	int n = event.GetNumberOfFiles();
	wxString *str = event.GetFiles();

}

// Drag
wxDragResult 	MojoShell::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	getdrag = true;
	leftmousedown = true;

	//_TRACE(L"drag %d %d : %d\n", x, y, (int)def);
	SelectWindowByPos(x, y, CDirectoryManager::_MOUSE_MOVE_DROP);
	Refresh(false);
	return def;
}

// Drop!!
bool MojoShell::OnDropFiles(wxCoord x, wxCoord y,const wxArrayString& filenames)
{
	getdrag = false;
	leftmousedown = false;
	if( filenames.empty() ) return true;

	SelectWindowByPos(x, y, CDirectoryManager::_MOUSE_CLICK_DROP);
	m_directoryMan[lastselectedwindow].DropFiles(x, y, filenames);

	m_directoryMan[_LEFT_WINDOW].ClearSelected();
	m_directoryMan[_RIGHT_WINDOW].ClearSelected();

	Refresh(false);
	return true;
}

void	MojoShell::OnSize(wxSizeEvent& event)
{
//	wxPoint pos = GetPosition();
	canvassize = GetClientSize();
	windowsize = GetSize();

 	if (m_backBuffer != NULL)
	{
		delete m_backBuffer;
		m_backBuffer = new wxBitmap(canvassize.x, canvassize.y);
	}
	else
	{
		m_backBuffer = new wxBitmap(canvassize.x, canvassize.y);
	}

	m_directoryMan[_LEFT_WINDOW].SetWindowSize(canvassize);
	m_directoryMan[_RIGHT_WINDOW].SetWindowSize(canvassize);

	//_TRACE(L"size %d, %d\n", canvassize.x, canvassize.y);

	Refresh(true);
	//m_windowsize = csize;
}

void	MojoShell::OnMove(wxMoveEvent& event)
{
	wxPoint pos = GetPosition();
//	wxSize csize = GetClientSize();

//	m_windowpos = pos;
	//_TRACE(L"pos %d, %d\n", pos.x, pos.y);
}

void	MojoShell::OnClose(wxCloseEvent& WXUNUSED(event))
{
	if( m_backBuffer )
		delete m_backBuffer;

	SaveConfig();

//	Destroy();
	exit(0);
}

void	MojoShell::OnSetFocus(wxFocusEvent &event)
{
	//_TRACE(L"OnSetFocus\n");
	//wxKeyEvent ke;
	//m_directoryMan[m_currentselectwindow].PressKeyboard( CDirectoryManager::_KEY_CLEAR, ke );
	Refresh(false);
}

void	MojoShell::OnKillFocus(wxFocusEvent &event)
{

	if( getdrag )
	{
		m_directoryMan[_LEFT_WINDOW].ClearSelected();
		m_directoryMan[_RIGHT_WINDOW].ClearSelected();
	}

	getdrag = false;
	leftmousedown = false;

	m_directoryMan[_LEFT_WINDOW].keymodifyer = 0;
	m_directoryMan[_RIGHT_WINDOW].keymodifyer = 0;
	Refresh(false);
}

void	MojoShell::OnKeyDown(wxKeyEvent& event)
{
	if(event.GetKeyCode() == WXK_TAB )
	{
		if( m_currentselectwindow == _LEFT_WINDOW )
		{
			m_directoryMan[m_currentselectwindow].SetWindowActive(false);
			m_currentselectwindow = _RIGHT_WINDOW; 
			m_directoryMan[m_currentselectwindow].SetWindowActive(true);
		}
		else if( m_currentselectwindow == _RIGHT_WINDOW )
		{
			m_directoryMan[m_currentselectwindow].SetWindowActive(false);
			m_currentselectwindow = _LEFT_WINDOW; 
			m_directoryMan[m_currentselectwindow].SetWindowActive(true);
		}
		return;
	}

	m_directoryMan[m_currentselectwindow].PressKeyboard( CDirectoryManager::_KEY_DOWN, event );
	Refresh(false);
}

void	MojoShell::OnKeyUp(wxKeyEvent& event)
{
	m_directoryMan[m_currentselectwindow].PressKeyboard(CDirectoryManager::_KEY_UP, event );
	Refresh(false);
}

void	MojoShell::OnLeftClick(wxMouseEvent& event)
{
	int x = event.GetX();
	int y = event.GetY();
	drag_x = x;
	//_TRACE(L"%d, %d\n", x,y);

	if( CheckInSeparate(x) )
	{
		clickSeparate = true;
		_TRACE(L"clickSeparate ");
	}
	else
	{
		drag_select = SelectWindowByPos(x, y, CDirectoryManager::_MOUSE_CLICK_LEFT);
	}

	Refresh(false);
	leftmousedown = true;
	getdrag = false;
}


void	MojoShell::OnLeftClickUp(wxMouseEvent& event)
{
	leftmousedown = false;
	getdrag = false;
	clickSeparate = false;
}

void	MojoShell::OnRightClick(wxMouseEvent& event)
{
	int x = event.GetX();
	int y = event.GetY();
	//_TRACE(L"%d, %d\n", x,y);

	SelectWindowByPos(x, y, CDirectoryManager::_MOUSE_CLICK_RIGHT);
	Refresh(false);
}

void	MojoShell::OnLeftDClick(wxMouseEvent& event)
{
	int x = event.GetX();
	int y = event.GetY();
	if(x < m_windowsize.x/2)
		m_directoryMan[_LEFT_WINDOW].MouseClick(m_windowpos, x,y,CDirectoryManager::_MOUSE_DCLICK_LEFT);
	else
		m_directoryMan[_RIGHT_WINDOW].MouseClick(m_windowpos, x,y,CDirectoryManager::_MOUSE_DCLICK_LEFT);

	Refresh(false);
}

void	MojoShell::OnMouseWheel(wxMouseEvent& event)
{
	//int delta = event.GetWheelDelta();
	int delta = event.GetWheelRotation();
	m_directoryMan[m_currentselectwindow].MouseWheel(delta);

	Refresh(false);
}

void	MojoShell::OnMouseMove(wxMouseEvent& event)
{
 	int x = event.GetX();
 	int y = event.GetY();

	int sel = m_directoryMan[m_currentselectwindow].MouseClick(m_windowpos, x, y, leftmousedown ? CDirectoryManager::_MOUSE_MOVE_DROP : CDirectoryManager::_MOUSE_MOVE);

	if(leftmousedown && (drag_select != sel || abs(drag_x - x) > 10) && getdrag == false)
	{
		GetDragFiles();
	}
	else
	{
		if( clickSeparate )
		{
// 			separatePos = x;
// 			Refresh(true);
		}
		else
		{
/*
			if( CheckInSeparate(x) )
				SetCursor(wxCursor(wxCURSOR_SIZEWE));
			else
				SetCursor(wxCursor(wxCURSOR_ARROW));
*/
		}
	}
}

bool	MojoShell::CheckInSeparate(int x)
{
	wxSize clientsize = GetClientSize();
	if( (clientsize.x / 2) - (SEPARATE_WIDTH/2) <= x && (clientsize.x / 2) + (SEPARATE_WIDTH/2) >= x )
		return true;
	else
		return false;
}

// flicker free
void	MojoShell::OnEraseBackGround(wxEraseEvent& event)
{
}

void	MojoShell::OnPaint(wxPaintEvent &event)
{
	wxSize size = GetSize();
	wxSize csize = GetClientSize();
	wxPoint pos = GetPosition();
//	wxPoint pos = GetScreenPosition();

// 	_TRACE(L"pos %d, %d\n", pos.x, pos.y);
// 	_TRACE(L"size %d, %d\n", size.GetWidth(), size.GetHeight());
// 	_TRACE(L"csize %d, %d\n", csize.GetWidth(), csize.GetHeight());

	//if(separatePos < 0)
		separatePos = (csize.x / 2);

	m_windowpos = pos;
	m_windowsize = size;

//  leftwindow->render();
// 	rightwindow->render();
	Render();

//	_TRACE(L"Render\n");
}

void	MojoShell::Render()
{
	wxSize clientsize = GetClientSize();

	wxPaintDC dc( this );
	if (m_backBuffer == NULL)
		m_backBuffer= new wxBitmap(clientsize.x, clientsize.y);

	wxMemoryDC memdc;
	memdc.SelectObject(*m_backBuffer);
	memdc.SetBackground(*wxBLACK_BRUSH);
	memdc.Clear();

	unsigned long bgcolor = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"MAINBG");

	Canvas c(&memdc, clientsize.x, clientsize.y);
	c.Clear(bgcolor);

	wxPoint p(0,0);

	m_directoryMan[_LEFT_WINDOW].RenderContents(p, clientsize, &c);

	wxPoint p2(clientsize.x/2,0);
	m_directoryMan[_RIGHT_WINDOW].RenderContents(p2, clientsize, &c);
	m_directoryMan[_RIGHT_WINDOW].DrawSeparate(separatePos, clientsize, &c);

	//dc.Clear();
	wxPoint pa = GetClientAreaOrigin();
	dc.Blit(0, pa.y, clientsize.x, clientsize.y, &memdc, 0, 0);		// 전체화면 dump
	//_TRACE(L"paint %d, %d\n", size.x, size.y);
}

void	MojoShell::SaveConfig()
{
	ConfigManager::GetInstance()->SetConfig(L"sizex", unicode::format(L"%d", m_windowsize.x));
	ConfigManager::GetInstance()->SetConfig(L"sizey", unicode::format(L"%d", m_windowsize.y));
	ConfigManager::GetInstance()->SetConfig(L"posx", unicode::format(L"%d", m_windowpos.x));
	ConfigManager::GetInstance()->SetConfig(L"posy", unicode::format(L"%d", m_windowpos.y));
	ConfigManager::GetInstance()->Save();
}

// 옆창을 구하자
CDirectoryManager *MojoShell::GetNextDoor(int where)
{
	return &m_directoryMan[where];
}

int		MojoShell::SelectWindowByPos(int x, int y, int method)
{
	int ret = -1;
	if(x < m_windowsize.x/2)
	{
		if(m_currentselectwindow != _LEFT_WINDOW) 
		{
			m_directoryMan[m_currentselectwindow].SetWindowActive(false);
			m_currentselectwindow = _LEFT_WINDOW; 
			m_directoryMan[m_currentselectwindow].SetWindowActive(true);
		}

		ret = m_directoryMan[_LEFT_WINDOW].MouseClick(m_windowpos, x, y, method);
		lastselectedwindow = _LEFT_WINDOW;
	}
	else
	{
		if(m_currentselectwindow != _RIGHT_WINDOW) 
		{
			m_directoryMan[m_currentselectwindow].SetWindowActive(false);
			m_currentselectwindow = _RIGHT_WINDOW; 
			m_directoryMan[m_currentselectwindow].SetWindowActive(true);
		}

		ret = m_directoryMan[_RIGHT_WINDOW].MouseClick(m_windowpos, x, y, method);
		lastselectedwindow = _RIGHT_WINDOW;
	}

	return ret;
}

void MojoShell::GetDragFiles()
{
	dragfilelist.clear();
	m_directoryMan[lastselectedwindow].GetDragFiles(dragfilelist);

	if( dragfilelist.empty() == false)
	{
		wxFileDataObject dragData;
		for (size_t i = 0; i<dragfilelist.size(); i++)
		{
			//dragData.AddFile(L"d:\\2013-08-24");
			dragData.AddFile(dragfilelist[i]);
		}
		
		wxDropSource dragSource( this );
		dragSource.SetData( dragData );
		wxDragResult result = dragSource.DoDragDrop( wxDrag_CopyOnly );
		dragSource.SetCursor(result, wxCursor(wxCURSOR_ARROW));

		getdrag = true;
	}

}