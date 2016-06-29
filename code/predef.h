#ifndef _PREDEF_H_
#define _PREDEF_H_

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// property grid
// #include "wx/propgrid/propgrid.h"
// #include "wx/propgrid/advprops.h"

#include <wx/splitter.h>
#include <wx/filepicker.h>
#include <wx/xml/xml.h>
#include <wx/clipbrd.h>
#include <wx/dnd.h>
#include <wx/treectrl.h>
#include <wx/dirctrl.h>
#include <wx/dir.h>
//#include "wx/slider.h"
#include <wx/icon.h>
#include <wx/wfstream.h>
//#include "wx/zipstrm.h"
#include <wx/txtstrm.h>
#include <wx/hyperlink.h>

#include <wx/listctrl.h>
#include <wx/listbase.h>
#include <wx/display.h>
#include <wx/icon.h>
#include <wx/notebook.h>

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/fontpicker.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/radiobut.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/vlbox.h>
#include <wx/colordlg.h>
#include <wx/protocol/ftp.h>
#include <wx/msw/registry.h>
#include <wx/fontdlg.h>

#include <Windows.h>
#include <string>
#include <vector>
#include <map>
#include <list>

#include "ini.h"

typedef wchar_t wchar;
#define String	std::wstring

#define SHELL_FONTSIZE	9

#define TYPE_NONE		0
#define TYPE_DRIVE		1 << 31
#define	TYPE_DIRECTORY	1 << 30
#define	TYPE_FILE		1 << 29

#define ATTR_READONLY	1 << 0
#define ATTR_HIDDEN		1 << 1	
#define ATTR_SYSTEM		1 << 2
#define ATTR_ARCHIVE	1 << 5

enum _COPY_RESULT
{
	NO_ERRROR = 0,
	CANT_COPY,
	CANCEL_COPY
};


enum _KEYBOARD
{
	_KEY_NONE	= 0,
	_KEY_CTRL	= 1 << 1,
	_KEY_ALT	= 1 << 2,
	_KEY_SHIFT	= 1 << 3
};


struct  FILEINFO
{
	bool			m_selected;
	int				m_type;
	int				m_attribute;
	FILETIME		m_creationtime;
	FILETIME		m_LastAccessTime;
	FILETIME		m_LastWriteTime;

	__int64			m_filesize;

	String			m_name;
	String			m_ext;
	String			m_fullpath;		// 실제로 이동할 경로
	String			m_excutepath;	// 경로 + 파일명 (실행할것)

	FILEINFO()
	{
		m_selected = false;
		m_filesize = 0;
		m_type = TYPE_NONE;
		m_attribute = 0;
	}
};

const String functioncommand[] = { L"$(DRIVE)" , L"$(DIR)" , L"$(FULLPATH)", L"$(FILENAME)" };

#define ARCHIVE_ZIP		0
#define ARCHIVE_7Z		1
#define ARCHIVE_LZH		2

const String archiveext[] = { L".zip", L".7z", L".lzh" };

struct SVNInfo 
{
	String	name;
	String	exec;
	String	path;
};

#define GRID_DIVIDE		15
#define IMAGEVIEW_X		GRID_DIVIDE*11
#define IMAGEVIEW_Y		GRID_DIVIDE*11

#define	IMAGEVIEW_MAX_X		2048
#define	IMAGEVIEW_MAX_Y		2048

#endif