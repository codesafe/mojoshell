#ifndef _MOJODIR_H_
#define _MOJODIR_H_

#include "predef.h"
#include "directoryman.h"
#include "functionkey.h"
#include "ui.h"

#define MOJOWINDOW_POS_X		400
#define MOJOWINDOW_POS_Y		100

#define MOJOWINDOW_SIZEX		800
#define MOJOWINDOW_SIZEY		600

#define ID_MENU				2000

// FILE
#define		ID_FILE_MENU			ID_MENU + 100
#define		ID_MENU_OPEN			ID_FILE_MENU + 1
#define		ID_MENU_OPEN_OPTION		ID_FILE_MENU + 2
#define		ID_MENU_COPY			ID_FILE_MENU + 3
#define		ID_MENU_MOVE			ID_FILE_MENU + 4
#define		ID_MENU_TRASHBIN		ID_FILE_MENU + 5
#define		ID_MENU_DELETE			ID_FILE_MENU + 6
#define		ID_MENU_RENAME			ID_FILE_MENU + 7
#define		ID_MENU_ATTRIBUTE		ID_FILE_MENU + 8
// FOLDER
#define		ID_FOLDER_MENU			ID_MENU + 200
#define		ID_FOLDER_MAKE			ID_FOLDER_MENU + 1
#define		ID_FOLDER_MOVE_PARENT	ID_FOLDER_MENU + 2
#define		ID_FOLDER_MOVE_ROOT		ID_FOLDER_MENU + 3
// EDIT
#define		ID_EDIT_MENU			ID_MENU + 300
#define		ID_EDIT_COPY			ID_EDIT_MENU + 1
#define		ID_EDIT_PASTE			ID_EDIT_MENU + 2
#define		ID_EDIT_SELECT_SINGLE	ID_EDIT_MENU + 3
#define		ID_EDIT_SELECT_ALL		ID_EDIT_MENU + 4
#define		ID_EDIT_SELECT_SAME		ID_EDIT_MENU + 5
#define		ID_EDIT_SELECT_ADV		ID_EDIT_MENU + 6
#define		ID_EDIT_COMMAND			ID_EDIT_MENU + 7
#define		ID_EDIT_EXPLORER		ID_EDIT_MENU + 8

// VIEW
#define		ID_VIEW_MENU			ID_MENU + 400
#define		ID_VIEW_SORT_NAME		ID_VIEW_MENU + 1
#define		ID_VIEW_SORT_TIME		ID_VIEW_MENU + 2
#define		ID_VIEW_SORT_SIZE		ID_VIEW_MENU + 3
#define		ID_VIEW_HIDDEN			ID_VIEW_MENU + 4

// COMPRESS
#define		ID_COMPRESS_MENU		ID_MENU + 500
#define		ID_COMPRESS_ZIP			ID_COMPRESS_MENU + 1
#define		ID_COMPRESS_UNZIP		ID_COMPRESS_MENU + 2
// OPTION
#define		ID_OPTION_MENU			ID_MENU + 600
#define		ID_OPTION_OPTION		ID_OPTION_MENU + 1
#define		ID_OPTION_FUNCTION		ID_OPTION_MENU + 2

// HELP
#define		ID_HELP_MENU			ID_MENU + 700
#define		ID_HELP_HELP			ID_HELP_MENU + 1
#define		ID_HELP_HOTKEY			ID_HELP_MENU + 2
#define		ID_HELP_UPDATE			ID_HELP_MENU + 3
#define		ID_HELP_ABOUT			ID_HELP_MENU + 4

// SVN
#define		ID_SVN_MENU					ID_MENU + 800
#define		ID_SVN_MENU_UPDATE			ID_SVN_MENU + 1
#define		ID_SVN_MENU_COMMIT			ID_SVN_MENU + 2
#define		ID_SVN_MENU_EXPLORE			ID_SVN_MENU + 3
#define		ID_SVN_MENU_OPTION			ID_SVN_MENU + 4


//////////////////////////////////////////////////////////////////////////


class Shellwindow;
class MojoShell;
class CApp : public wxApp 
{
public:
	CApp() {}
	virtual bool OnInit();
	virtual int OnExit();

private:

	MojoShell	*frame;
};


class MojoShell : public wxFrame, wxFileDropTarget
{
public :
	enum
	{
		_LEFT_WINDOW,
		_RIGHT_WINDOW,
		_MAX_WINDOW
	};


public:
	MojoShell(const wxString& title, wxWindowID id, const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);
	~MojoShell();

// 	wxDragResult	OnEnter(wxCoord x, wxCoord y, wxDragResult def);
 	void	OnLeave();
// 	wxDragResult	OnData(wxCoord x, wxCoord y, wxDragResult def);
	void			OnDropFile(wxDropFilesEvent &event);

	bool			OnDropFiles(wxCoord x, wxCoord y,const wxArrayString& filenames);
	wxDragResult 	OnDragOver(wxCoord x, wxCoord y, wxDragResult def);

	void	OnPaint(wxPaintEvent &event);
	void	OnSize(wxSizeEvent& event);
	void	OnMove(wxMoveEvent& event);
	void	OnClose(wxCloseEvent& WXUNUSED(event));
	void	OnSetFocus(wxFocusEvent &event);
	void	OnKillFocus(wxFocusEvent &event);

	void	OnKeyDown(wxKeyEvent& event);
	void	OnKeyUp(wxKeyEvent& event);
	void	OnLeftClick(wxMouseEvent& event);
	void	OnLeftClickUp(wxMouseEvent& event);
	void	OnLeftDClick(wxMouseEvent& event);
	void	OnRightClick(wxMouseEvent& event);
	void	OnMouseWheel(wxMouseEvent& event);
	void	OnMouseMove(wxMouseEvent& event);

	void	OnEraseBackGround(wxEraseEvent& event);


	void	KeyBoardSimul(int key, int att);
	void	OnFileCommand(wxCommandEvent &event);
	void	OnFolderCommand(wxCommandEvent &event);
	void	OnEditCommand(wxCommandEvent &event);
	void	OnViewCommand(wxCommandEvent &event);
	void	OnCompressCommand(wxCommandEvent &event);

	void	OnNormalOption(wxCommandEvent &WXUNUSED(event));
	void	OnFunctionKey(wxCommandEvent &WXUNUSED(event));
	void	OnQuit(wxCommandEvent &WXUNUSED(event));

	void	OnHelpHelp(wxCommandEvent &WXUNUSED(event));
	void	OnHelpNotkey(wxCommandEvent &WXUNUSED(event));
	void	OnHelpUpdate(wxCommandEvent &WXUNUSED(event));
	void	OnHelpAbout(wxCommandEvent &WXUNUSED(event));

	void	OnSVNMenuCommand(wxCommandEvent &WXUNUSED(event));

	WXHWND	getHwnd() { return mainHwhd; }
	wxSize	getWindowSize() { return windowsize; }
	wxSize	getCanvasSize() { return canvassize; }

	CDirectoryManager *GetNextDoor(int where);

	DECLARE_EVENT_TABLE()

private :

	int		SelectWindowByPos(int x, int y, int method);
	void	Render();
	void	SaveConfig();

	CINI				pathinfo_ini;

	wxTimer*			m_checkdirtimer;
	wxSize				m_windowsize;
	wxPoint				m_windowpos;

	int					m_currentselectwindow;
	CDirectoryManager	m_directoryMan[_MAX_WINDOW];
	wxBitmap*			m_backBuffer;

	wxMenuBar*			menu_bar;
	wxMenu *			menu_file;
	wxMenu *			menu_folder;
	wxMenu *			menu_edit;
	wxMenu *			menu_view;
	wxMenu *			menu_compress;
	wxMenu *			menu_option;
	wxMenu *			menu_help;

	WXHWND				mainHwhd;
	wxSize				windowsize;
	wxSize				canvassize;

// 	Shellwindow			*leftwindow;
// 	Shellwindow			*rightwindow;
	bool				leftmousedown;

	UI					m_ui;

private :
	bool	CheckInSeparate(int x);
	bool	clickSeparate;
	int		separatePos;

	void	GetDragFiles();
	int		lastselectedwindow;
	std::vector<String>	dragfilelist;

	int		drag_select;
	int		drag_x;
	bool	getdrag;
};



#endif