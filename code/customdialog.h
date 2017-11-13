#ifndef _CUSTOMDIALOG_H_
#define _CUSTOMDIALOG_H_

#include "predef.h"
#include <wx/wx.h>

#include <wx/statline.h>
#include <wx/evtloop.h>

#include "stringutil.h"

#define ID_OK			1000
#define ID_CANCEL		1001
#define	ID_OK_ALL		1003
#define	ID_SKIP			1004
#define	ID_SKIP_ALL		1005
#define	ID_RENAME		1006

#define	ID_ARCHIVE		1007
#define	ID_READONLY		1008
#define	ID_HIDDEN		1009
#define	ID_SYSTEM		1010

// svn
#define ID_SVN_UPDATE			1100
#define ID_SVN_UPDATE_CONFIG	1101
#define ID_SVN_COMMIT			1110
#define ID_SVN_COMMIT_CONFIG	1111
#define ID_SVN_CLEANUP			1120
#define ID_SVN_CLEANUP_CONFIG	1121

#define ID_MAKE_DIR				1200
#define ID_CONFIGCOLOR			1201
#define ID_COLORBOX				1202
#define ID_COLOR_ADD			1206
#define ID_COLOR_EDIT			1207
#define ID_COLOR_REMOVE			1208

#define ID_SVN_ADD				1210
#define ID_SVN_EDIT				1211
#define ID_SVN_REMOVE			1212
#define ID_FUNCTION_EDIT		1213
#define ID_IMAGEPREVIEW			1214

#define ID_SELECTFONT			1220

#define ID_DRAGDROP_COPY		1300
#define ID_DRAGDROP_MOVE		1301

#define	DIALOG_YES		1 << 1
#define	DIALOG_ALLYES	1 << 2
#define	DIALOG_SKIP		1 << 3
#define	DIALOG_SKIPALL	1 << 4
#define	DIALOG_CANCEL	1 << 5
#define	DIALOG_RENAME	1 << 6

#include "wx/control.h"

class StaticTextDC : public wxPanel
{
public :
	StaticTextDC(wxWindow *parent, const wxString& label, const wxPoint& pos = wxDefaultPosition, const wxSize& size=wxDefaultSize);
	virtual ~StaticTextDC();

	void SetLabel(const wxString& label);
	void OnSize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent &event);

	DECLARE_EVENT_TABLE()

private :
	int				max_width;
	wxString		m_text;
	wxBitmap*		m_backBuffer;

};


//////////////////////////////////////////////////////////////////////////

struct ColorCheckListItem 
{
public:
	ColorCheckListItem(int type, wxString text, wxString tag, wxColor color, wxColor bgcolor, wxColor fontcolor)
	{
		Type = type;
		Text = text;
		Tag = tag;
		Color = color;
		BgColor = bgcolor;
		FontColor = fontcolor;
	}

	int         TopY;
	int         Height;
	
	int			Type;
	wxString    Text;
	wxString    Tag;

	wxColor		Color;
	wxColor		BgColor;
	wxColor		FontColor;
};


class Options;

class ColorCheckListBox : public wxVListBox
{
public:
	ColorCheckListBox(Options *p, wxWindow *parent, int id, int type);

	void	OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const ;
	void	OnDrawBackground(wxDC &dc, const wxRect &rect, size_t n) {}

	void	OnLeftButtonDown(wxMouseEvent & event);
	void	OnDoubleClick(wxMouseEvent & event);
	void	OnKillFocus(wxFocusEvent &event);

	void	AppendItem(wxString str, wxString tag, wxColour color, wxColour bgcolor, wxColour fontcolor);
	bool	CheckBoxContainsPoint(ColorCheckListItem & item, wxPoint p);

	wxCoord    OnMeasureItem(size_t n) const { return m_ItemHeight; }
	void	Reset();
	String	GetSelectedItem(int i)
	{
		return String(m_List[i].Tag);
	}

private:
	int		m_FontSize;
	wxSize	m_CheckBoxSize;
	wxSize	m_CheckBoxIndent;
	int		m_TextIndent;
	int		m_ItemHeight;

	int		Type;
	std::vector<ColorCheckListItem> m_List;
	Options	*m_parent;

};


//////////////////////////////////////////////////////////////////////////

class StaticText : public wxStaticText
{
public :
	StaticText (wxWindow *parent, const wxString& label, int maxwidth, const wxPoint& pos = wxDefaultPosition);
	virtual ~StaticText() {};

	void SetText(const wxString& label);
private :
	int max_width;
};


//////////////////////////////////////////////////////////////////////////


class MessageDialog : public wxDialog
{
public:
	MessageDialog(wxWindow *parent, const wxString & title, const wxString & msg, const wxString & text);

	bool	Show( bool show );
	void	Update(const wxString & text);

	void	ProgressSet(int s);
	void	UpdateProgress(int pg);
	void	OnCancel(wxCommandEvent & event);

	bool	IsCancel() { return iscancel; }

	void ReenableOtherWindows();

	DECLARE_EVENT_TABLE()

	wxStaticText *tx;
	wxGauge *prgbar;
	wxButton *cancel; 

	wxWindow *m_parentTop;
	wxWindowDisabler *m_winDisabler;
	bool	iscancel;
};

// 디렉토리 생성
class MakeDirDialog : public wxDialog
{
public:
	MakeDirDialog(wxWindow *parent, const wxString& title);

	void	OnOk(wxCommandEvent & event);
	void	OnCancel(wxCommandEvent & event);

	wxTextCtrl *tc;
	String	returnstr;
};

#if 0 
// 이름 바꾸기 
class RenameDialog : public wxDialog
{
public:
	RenameDialog(wxWindow *parent, const wxString& title, const wxString& name, const wxString& ext, bool showext);

	void	OnOk(wxCommandEvent & event);
	void	OnCancel(wxCommandEvent & event);

	wxTextCtrl *tc;
	wxTextCtrl *te;
	bool extenable;
};

#else

class RenameDialog : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap4;
	wxStaticText* m_staticText4;
	wxTextCtrl* tc;
	wxTextCtrl* te;
	wxButton* m_button6;
	wxButton* m_button7;

public:

	RenameDialog(wxWindow* parent, const wxString& title, const wxString& name, const wxString& ext, bool showext);
	~RenameDialog();

	void	OnOk(wxCommandEvent & event);
	void	OnCancel(wxCommandEvent & event);

	bool extenable;
};

#endif

class EnterTextDialog : public wxDialog
{
protected:
	wxStaticBitmap* m_bitmap4;
	wxStaticText* m_staticText4;
	wxTextCtrl* inputtext;
	wxButton* m_button6;
	wxButton* m_button7;

public:
	EnterTextDialog(wxWindow *parent, const wxString& title, const wxString& text, const wxString& name, const wxString& key);

	void	OnOk(wxCommandEvent & event);
	void	OnCancel(wxCommandEvent & event);

	String	returnstr;
	String	returnkey;
};


//////////////////////////////////////////////////////////////////////////

class DestDialog : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap7;
	wxStaticText* m_staticText42;
	wxListBox* m_listBox1;
	wxCheckBox* m_checkBox10;
	wxButton* m_ok;
	wxButton* m_cancel;

public:

	DestDialog( wxWindow* parent, const wxString& title, const wxString& text, std::vector<String> &dirlist, bool bmakedir = true );
	~DestDialog();

	void	OnOk(wxCommandEvent & event);
	void	OnCancel(wxCommandEvent & event);
	void	OnCheckBox(wxCommandEvent & event);

	String	returnstr;
	bool	makedir;

};


//////////////////////////////////////////////////////////////////////////


class TextCtrl : public wxTextCtrl
{
public :
	TextCtrl(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,long style = 0);

	bool SetBackgroundColour(const wxColour& colour);
	bool AcceptsFocus();
};



class CopyDialog : public wxDialog
{
public:
	CopyDialog(wxWindow *parent, const wxString& title);
	void OnCancel(wxCommandEvent & event);

	void Update(String com, String v);
	bool IsCancel();

	bool Show( bool show );
	void ReenableOtherWindows();

private :
	wxTextCtrl *sourcepath;
	wxTextCtrl *destpath;
	wxTextCtrl *copysizetext;
	wxTextCtrl *copycount;

	wxGauge *totalprogress;
	wxGauge *currentprogress;
	wxButton *cancel; 

	bool iscancel;
	wxWindowDisabler *winDisabler;

};


class OverWriteDialog : public wxDialog
{
public:
	OverWriteDialog(wxWindow *parent, const wxString& title, const wchar *source, const wchar *dest,
		int srcsize, int destsize, SYSTEMTIME &srctime, SYSTEMTIME &desttime,unsigned long option = 0xFFFFFFFF);

	void OnOk(wxCommandEvent & event);
	void OnOkAll(wxCommandEvent & event);
	void OnSkip(wxCommandEvent & event);
	void OnSkipAll(wxCommandEvent & event);
	void OnCancel(wxCommandEvent & event);
	void OnRename(wxCommandEvent & event);
	
	void close();
	int getResult() { return ret; }

private :

	wxButton *skip; 
	wxButton *skipall; 
	wxButton *ok;		// overwrite
	wxButton *okall;	// overwrite all
	wxButton *cancel; 
	wxButton *rename; 

	int		ret;
};


//////////////////////////////////////////////////////////////////////////

class ScanDialog : public wxDialog
{
public :
	ScanDialog(wxWindow *parent, const wxString& title, const wxString& txt);
	void	OnCancel(wxCommandEvent & event);
	void	Update(String str, String txt);

	bool Show( bool show );
	void ReenableOtherWindows();
	bool	IsCancel() { return _cancel; }

private :
	//wxStaticText *text;
	//wxStaticText *text2;
	StaticTextDC *text;
	StaticTextDC *text2;
	wxButton *cancel; 

	wxWindowDisabler *winDisabler;
	bool _cancel;
};

//////////////////////////////////////////////////////////////////////////

class AttributeDialog : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap7;
	wxStaticText* m_staticText42;
	wxStaticText* m_staticText1;
	wxCheckBox* m_checkBox1;
	wxCheckBox* m_checkBox11;
	wxCheckBox* m_checkBox12;
	wxCheckBox* m_checkBox13;
	wxCheckBox* m_checkBox5;
	wxCheckBox* m_checkBox51;
	wxCheckBox* m_checkBox52;
	wxCheckBox* m_checkBox53;
	wxCheckBox* m_checkBox20;
	wxButton* m_button1;
	wxButton* m_button11;
	wxButton* m_button12;
	wxButton* m_button13;

	int		newattribute;
	int		ret;

public:

	AttributeDialog( wxWindow* parent, wxWindowID id = wxID_ANY, int old_attr=0, const wxString& name=wxT(""), const wxString& title = wxT("속성 바꾸기"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 415,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~AttributeDialog();

	int GetAttribute();
	int getResult() { return ret; }

	void OnOk(wxCommandEvent & event);
	void OnOkAll(wxCommandEvent & event);
	void OnSkip(wxCommandEvent & event);
	void OnCancel(wxCommandEvent & event);

	void Update(int old_attr, const wxString& name);
	void OnCheckBox(wxCommandEvent & event);
};


//////////////////////////////////////////////////////////////////////////

class MoveFileDialog : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap7;
	wxStaticText* m_staticText42;
	wxStaticText* m_targetdir;
	wxStaticText* m_filename;
	wxStaticText* m_filecount;
	wxGauge* m_gauge3;
	wxButton* m_button7;

	wxWindowDisabler *winDisabler;
	void ReenableOtherWindows();

public:

	MoveFileDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("파일 이동 중"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~MoveFileDialog();

	bool Show( bool show );
	void OnCancel(wxCommandEvent & event);
	void Update(String com, String v);

	bool iscancel;
};


class Function : public wxDialog 
{
private:
	wxDECLARE_NO_COPY_CLASS(Function);
	wxDECLARE_EVENT_TABLE();

protected:
	wxListCtrl* m_funclist;
	//MyListCtrl* m_funclist;
	wxButton* m_ok;
	wxButton* m_cancel;

	void SetUpList();

public:

	Function( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("기능기 지정"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 550,400 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~Function();

	void	OnDoubleClick(wxListEvent& event);

	void	OnOk(wxCommandEvent & event);
	void	OnCancel(wxCommandEvent & event);
};


class FunctionInput : public wxDialog 
{
private:
	wxDECLARE_NO_COPY_CLASS(FunctionInput);
	wxDECLARE_EVENT_TABLE();

protected:
	wxStaticBitmap* m_bitmap1;
	wxStaticText* m_staticText18;
	wxStaticLine* m_staticline1;
	wxStaticText* m_staticText4;
	wxTextCtrl* m_name;
	wxStaticText* m_staticText41;
	wxFilePickerCtrl* m_filePicker1;
	wxStaticText* m_staticText42;
	wxTextCtrl* m_param;
	wxComboBox* m_paramcombo;
	wxStaticLine* m_staticline2;
	wxStaticText* m_staticText19;
	wxButton* m_ok;
	wxButton* m_cancel;

public:

	FunctionInput( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("기능키 편집"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 532,272 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~FunctionInput();

	void	OnComboBoxUpdate( wxCommandEvent& event );
	void	Setup(int sel);

	void	OnOk(wxCommandEvent & event);
	void	OnCancel(wxCommandEvent & event);

	int		selected;
	int		ret; 
};

//////////////////////////////////////////////////////////////////////////

class DeleteDialog : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap2;
	wxStaticText* m_staticText22;
	wxTextCtrl* m_filename;
	wxTextCtrl* m_progressnum;
	wxGauge* m_progress;
	wxButton* m_button25;

public:

	DeleteDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("삭제"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 471,207 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~DeleteDialog();

	bool	Show( bool show );
	void	OnCancel(wxCommandEvent & event);
	void	ReenableOtherWindows();
	void	Update(String com, String v);
	bool	IsCancel();

	bool iscancel;
	wxWindowDisabler *winDisabler;

};


//////////////////////////////////////////////////////////////////////////

#define  MAX_BTN	5

class SVNDialog : public wxDialog 
{
private:

protected:
		wxStaticText* m_staticText27;
		wxChoice* m_choice2;
		wxBitmapButton* m_svn_update[MAX_BTN];
		wxBitmapButton* m_svn_commit[MAX_BTN];
		wxBitmapButton* m_svn_cleanup[MAX_BTN];
		wxButton* m_cancel;

public:

	SVNDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("SVN"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 357,238 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~SVNDialog();

	void	OnClose(wxCommandEvent & event);
	void	OnCommand(wxCommandEvent & event);

	std::vector<SVNInfo> svninfo;
};

//////////////////////////////////////////////////////////////////////////

class SVNConfig : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap1;
	wxStaticText* m_staticText18;
	wxStaticLine* m_staticline1;
	wxStaticText* m_staticText38;
	wxTextCtrl* m_textCtrl12;
	wxStaticText* m_staticText27;
	wxFilePickerCtrl* m_filePicker2;
	wxStaticText* m_staticText28;
	wxDirPickerCtrl* m_dirPicker1;
	wxButton* m_ok;
	wxButton* m_cancel;

public:

	SVNConfig( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("SVN Update 설정"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 396,156 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~SVNConfig();

	void	OnCommand(wxCommandEvent & event);
	void	SetValue(String name, String exec, String path);

	String	editinfo;
};

//////////////////////////////////////////////////////////////////////////

class ExecParam : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap4;
	//wxStaticText* m_staticText4;
	StaticTextDC* m_staticText4;
	wxTextCtrl* m_param;
	wxButton* m_button6;
	wxButton* m_button7;

public:

	ExecParam( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("인자 포함하여 실행하기"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~ExecParam();

	void	OnOk(wxCommandEvent & event);
	void	OnCancel(wxCommandEvent & event);

	String	param;
	int		ret;
};

//////////////////////////////////////////////////////////////////////////


class Compress : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap5;
	wxStaticText* m_staticText4;
	wxStaticLine* m_staticline1;
	wxTextCtrl* m_textCtrl1;
	wxStaticText* m_staticText8;
	wxChoice* m_choice1;
	wxStaticText* m_staticText82;
	wxChoice* m_archive;
	wxStaticText* m_staticText81;
	wxTextCtrl* m_textCtrl5;
	wxButton* m_button6;
	wxButton* m_button7;

public:
	Compress( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("압축"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~Compress();

	void OnOk(wxCommandEvent & event);
	void OnCancel(wxCommandEvent & event);
	int getResult() { return ret; }

	int		ret;
	int		compressmethod;
	int		compressarchive;
	String	compressfilename;
	String	compresspassword;

};


class CompressProgress : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap5;
	wxStaticText* m_staticText4;
	wxStaticLine* m_staticline1;
	wxStaticText* m_staticText1;
	wxStaticText* m_staticText16;
	wxStaticText* m_staticText21;
	wxStaticText* m_staticText22;
	wxStaticText* m_staticText14;
	wxGauge* m_gauge1;
	wxGauge* m_gauge11;
	wxButton* m_button7;

	wxWindowDisabler *winDisabler;
	void ReenableOtherWindows();

public:

	CompressProgress( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("압축 진행 중"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~CompressProgress();

	bool Show( bool show );
	void OnCancel(wxCommandEvent & event);
	void Update(String com, String v);

	bool iscancel;
};


class UnCompressProgress : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap5;
	wxStaticText* m_staticText4;
	wxStaticLine* m_staticline1;
	wxStaticText* m_staticText1;
	wxStaticText* m_staticText16;
	wxStaticText* m_staticText21;
	wxStaticText* m_staticText22;
	wxStaticText* m_staticText14;
	wxGauge* m_gauge1;
	wxGauge* m_gauge11;
	wxButton* m_button7;

public:

	UnCompressProgress( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("압축 푸는 중"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~UnCompressProgress();

	bool	Show( bool show );
	void	OnCancel(wxCommandEvent & event);
	void	ReenableOtherWindows();
	void	Update(String com, String v);

	bool iscancel;
	wxWindowDisabler *winDisabler;
};

//////////////////////////////////////////////////////////////////////////

class InputPasswd : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap4;
	wxStaticText* m_staticText4;
	wxTextCtrl* m_param;
	wxButton* m_button6;
	wxButton* m_button7;

public:

	InputPasswd( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("암호 입력"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~InputPasswd();

	void OnOk(wxCommandEvent & event);
	void OnCancel(wxCommandEvent & event);

	String	param;
	int		ret;
};

//////////////////////////////////////////////////////////////////////////

class Options : public wxDialog 
{
private:
	wxDECLARE_NO_COPY_CLASS(Options);
	wxDECLARE_EVENT_TABLE();

protected:
	wxNotebook* m_notebook1;
	wxPanel* m_color;
	ColorCheckListBox* m_listBox6;
	ColorCheckListBox* m_listBox3;
	wxButton* m_button24;
	wxButton* m_button25;
	wxButton* m_button26;
	wxPanel* m_svn;
	wxListBox* m_listBox4;
	wxButton* m_button29;
	wxButton* m_button30;
	wxButton* m_button31;
	wxPanel* m_function;
	wxListCtrl* m_listCtrl3;
	wxPanel* m_option;
	wxCheckBox* m_showimage;
	wxStaticText* m_staticText49;
	wxButton* m_selectfont;
	wxButton* m_ok;
	wxButton* m_cancel;
public:

	Options( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
	~Options();

	void OnOk(wxCommandEvent & event);
	void OnCancel(wxCommandEvent & event);
	void OnBtnDown(wxCommandEvent & event);
	void OnSvnBtnDown(wxCommandEvent & event);
	void OnImageviewCheckBox(wxCommandEvent & event);
	void OnSelectFont(wxCommandEvent & event);

	void OnDoubleClick(wxListEvent& event);

	void LoadSystemConfig();
	void LoadExtConfig();
	void LoadSVNConfig();
	void LoadFunction();
	void LoadCommonConfig();

	int ret;
};

//////////////////////////////////////////////////////////////////////////

class AboutDialog : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap3;
	wxStaticText* m_staticText23;
	wxStaticText* m_staticText24;
	wxStaticText* m_staticText241;
	wxStaticText* m_staticText2411;
	wxHyperlinkCtrl* m_hyperlink1;

public:

	AboutDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
	~AboutDialog();

};

//////////////////////////////////////////////////////////////////////////





class DragDropDialog : public wxDialog 
{
private:

protected:
	wxStaticBitmap* m_bitmap4;
	wxStaticText* m_staticText4;
	wxButton* m_move;
	wxButton* m_copy;
	wxButton* m_cancel;

public:

	DragDropDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
	~DragDropDialog();

	void OnCopy(wxCommandEvent & event);
	void OnMove(wxCommandEvent & event);
	void OnCancel(wxCommandEvent & event);
};


#endif