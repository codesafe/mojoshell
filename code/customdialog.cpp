#include "customdialog.h"
#include "valueman.h"
#include "utility.h"
#include "functionkey.h"
#include "configmanager.h"
#include "colormanager.h"
#include "canvas.h"
#include "font.h"

ColorCheckListBox::ColorCheckListBox(Options *p, wxWindow *parent, int id, int type)
: wxVListBox(parent, id, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE)
{
	m_parent = p;
	Type = type;
	SetItemCount(0);
	wxFont font = wxFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));

	m_FontSize          = font.GetPointSize();	  
	m_CheckBoxIndent.x    = 3;
	m_CheckBoxIndent.y    = 3;
	m_ItemHeight       = m_FontSize*2;// + m_CheckBoxIndent.y;//m_FontSize*3 + m_CheckBoxIndent.y;
	m_CheckBoxSize.x    = m_ItemHeight - m_CheckBoxIndent.y*2;
	m_CheckBoxSize.y    = m_CheckBoxSize.x;
	m_TextIndent       = 5;

	Connect(id, wxEVT_LEFT_DOWN, wxMouseEventHandler(ColorCheckListBox::OnLeftButtonDown));
	Connect(id, wxEVT_LISTBOX_DCLICK, wxMouseEventHandler(ColorCheckListBox::OnDoubleClick));
	Connect(id, wxEVT_KILL_FOCUS, wxFocusEventHandler(ColorCheckListBox::OnKillFocus));
}

bool ColorCheckListBox::CheckBoxContainsPoint(ColorCheckListItem & item, wxPoint p)
{
/*
	if(p.x > m_CheckBoxIndent.x && p.x < (m_CheckBoxIndent.x + m_CheckBoxSize.x)) 
	{
		if(p.y > (item.TopY + m_CheckBoxIndent.y) && p.y < (item.TopY + item.Height - m_CheckBoxIndent.y)) {
			return true;   
		}
	}      
*/

	return false;
}

// 다른곳으로 이동시 선택된것 Clear하도록
void	ColorCheckListBox::OnKillFocus(wxFocusEvent &event)
{
//	SetSelection(-1);
	Refresh();
}

void	ColorCheckListBox::OnLeftButtonDown(wxMouseEvent & event)
{
	event.Skip();
//	Refresh();
}

void ColorCheckListBox::OnDoubleClick(wxMouseEvent & event)
{   
#if 0
	bool clicked = false;   
	for(std::vector<ColorCheckListItem>::iterator it = m_List.begin(); it != m_List.end(); it++) 
	{
		wxPoint point(event.GetX(), event.GetY());   
/*
		if(CheckBoxContainsPoint(*it, point)) 
		{
			if((*it).Checked) 
				(*it).Checked = false;
			else 
				(*it).Checked = true;            

			clicked = true;
			break;
		}
*/
	}

	if(!clicked) 
	{
		event.Skip();
	} 
 	else 
 		Refresh();
#endif

	int sel = GetSelection();
	wxColourData data;
	data.SetChooseFull(TRUE);
	for (int i = 0; i < 16; i++)
	{
		wxColour colour(i*16, i*16, i*16);
		data.SetCustomColour(i, colour);
	}

	data.SetColour(m_List[sel].Color);

	wxColourDialog dialog(this, &data);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxColourData retData = dialog.GetColourData();
		wxColour col = retData.GetColour();

		unsigned long c = (col.Red() << 16) | (col.Green() << 8) | col.Blue();
		ColorManager::GetInstance()->SetColor(m_List[sel].Type, String(m_List[sel].Tag.c_str()), c);
		if(m_List[sel].Type == SYSTEM_COLOR)
			m_parent->LoadSystemConfig();
		else if(m_List[sel].Type == EXT_COLOR)
			m_parent->LoadExtConfig();
	}
}

void ColorCheckListBox::AppendItem(wxString str, wxString tag, wxColor color, wxColour bgcolor, wxColour fontcolor)
{   
	ColorCheckListItem item(Type, str, tag, color, bgcolor, fontcolor);

	if(m_List.size() == 0) 
		item.TopY = 0;
	else 
	{
		ColorCheckListItem lastItem = *(m_List.end()-1); // "m_List.end()" it's not the last element        
		item.TopY = lastItem.TopY + m_ItemHeight;
	}

	item.Height = m_ItemHeight;
	m_List.push_back(item);
	SetItemCount(m_List.size());
}

void ColorCheckListBox::OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const 
{
	if(n > m_List.size()-1)
		return; 

	size_t starttop = GetVisibleBegin();
	size_t startend = GetVisibleEnd();

	ColorCheckListItem item = m_List.at(n);
	int display_y = m_ItemHeight * (n-starttop);

	if(display_y > GetSize().GetHeight()) 
	{
		return;
	}
	else 
	{
		// Render BG
		dc.SetBrush(wxBrush(item.BgColor));

		int sel = GetSelection();
		if( sel != n )
		{
			dc.DrawRectangle(
				wxRect(0, display_y,
				rect.GetWidth(), 
				rect.GetHeight()));
		}

		dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));   
/*
		if(item.Checked) {
			dc.SetBrush(*wxWHITE_BRUSH);
			// draw a small triangle at the corner of the rectangle :)
			dc.DrawRectangle(wxRect(m_CheckBoxIndent.x, 
				item.TopY + m_CheckBoxIndent.y, 
				m_CheckBoxSize.x, m_CheckBoxSize.y));
			dc.SetPen(wxPen(wxColor(item.Color), 1, wxSOLID));   
			dc.SetBrush(wxBrush(item.Color));
			int s = m_CheckBoxSize.x/2;    // side of the triangle                
			wxPoint plist[] = {   
				wxPoint(m_CheckBoxIndent.x+1, item.TopY + m_CheckBoxIndent.y+1), 
				wxPoint(m_CheckBoxIndent.x+s, item.TopY + m_CheckBoxIndent.y+1),
				wxPoint(m_CheckBoxIndent.x+1, item.TopY + m_CheckBoxIndent.y+s)};

			dc.DrawPolygon(3, plist);
		}
		else */
		{
			dc.SetBrush(wxBrush(item.Color));
			dc.DrawRectangle(
				wxRect(m_CheckBoxIndent.x, 
				display_y + m_CheckBoxIndent.y, 
				//item.TopY + m_CheckBoxIndent.y, 
				m_CheckBoxSize.x, m_CheckBoxSize.y));
		}


		int textY = display_y + 2;//(m_ItemHeight/2 - m_FontSize/2);
		//int textY = item.TopY + (m_ItemHeight/2 - m_FontSize/2);
		int textX = m_CheckBoxIndent.x + m_CheckBoxSize.x + m_TextIndent;

		dc.SetTextForeground(wxColor(item.FontColor));
		dc.DrawText(item.Text, textX, textY);
	}
}

void	ColorCheckListBox::Reset()
{
	Clear();
	m_List.clear();
}

//////////////////////////////////////////////////////////////////////////

StaticText::StaticText(wxWindow *parent, const wxString& label, int maxwidth, const wxPoint& pos) : wxStaticText(parent, wxID_ANY, label, pos)
{
	max_width = maxwidth;
	SetText(label);
}

void StaticText::SetText(const wxString& label)
{
#if 0
	String text = label;
	int i= 0;
	while(1)
	{
		wxSize s = this->GetSize();
		if( s.GetWidth() < maxwidth )
		{
			break;
		}
		String t = String(text,0,text.size()-i) + L" ...";
		this->SetLabel(t);
		i++;
	}

#else
	String text = label;
	this->SetLabel(text);
	int i= 0;
	while(1)
	{
		wxSize s = this->GetSize();
		if( s.GetWidth() < max_width )
		{
			break;
		}
		int half = text.size()/2;
		String t = String(text,0,half-i) + L" ... " + String(text,half+i, text.size()-half+i) ;
		this->SetLabel(t);
		i++;
	}

#endif

}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MessageDialog, wxDialog)
	EVT_BUTTON(ID_CANCEL, MessageDialog::OnCancel)
END_EVENT_TABLE()

MessageDialog::MessageDialog(wxWindow *parent, const wxString & title, const wxString & msgtext, const wxString & text) : 
wxDialog(parent, -1, title, wxDefaultPosition, wxSize(500, 170), wxCAPTION | wxSYSTEM_MENU)
{
	m_parentTop = parent;
	while ( m_parentTop && m_parentTop->GetParent() )
	{
		m_parentTop = m_parentTop->GetParent();
	}

	iscancel = false;
	wxPanel *panel = new wxPanel(this, -1);
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText *msg = new wxStaticText( panel, wxID_ANY, msgtext, wxPoint(15, 15)); 
	//wxStaticLine *sl1 = new wxStaticLine(panel, wxID_ANY, wxPoint(15, 35), wxSize(470,1));
	tx = new wxStaticText( panel, wxID_ANY, text, wxPoint(15, 40));

	wxFont font = msg->GetFont();
	//font.SetPointSize(10);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	msg->SetFont(font);

	prgbar = new wxGauge(panel, wxID_ANY, 100, wxPoint(15, 60),  wxSize(460,20),
		wxGA_SMOOTH, wxDefaultValidator, wxT(""));

	cancel = new wxButton(this, ID_CANCEL, wxT("취소"), wxDefaultPosition, wxSize(170, 30));
	cancel->SetDefault();
	hbox->Add(cancel, 1);

	//vbox->Add(panel, 1);
	vbox->Add(panel, 1,wxEXPAND|wxALL);
	vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	SetSizer(vbox);

	//SetAutoLayout(TRUE);
	//Layout();


	//if ( style & wxPD_APP_MODAL )
	{
		m_winDisabler = new wxWindowDisabler(this);
	}
	// 	else
	// 	{
	// 		if ( m_parentTop )
	// 			m_parentTop->Enable(FALSE);
	// 		m_winDisabler = NULL;
	// 	}

	//ShowModal();
	Show(true);
	Enable(true);
	//Destroy(); 
	wxYield();
}



bool MessageDialog::Show( bool show )
{
	// reenable other windows before hiding this one because otherwise
	// Windows wouldn't give the focus back to the window which had
	// been previously focused because it would still be disabled
	if(!show)
		ReenableOtherWindows();

	return wxDialog::Show(show);
}

void MessageDialog::ReenableOtherWindows()
{
	//if ( GetWindowStyle() & wxPD_APP_MODAL )
	{
		delete m_winDisabler;
		m_winDisabler = (wxWindowDisabler *)NULL;
	}
// 	else
// 	{
// 		if ( m_parentTop )
// 			m_parentTop->Enable(TRUE);
// 	}
}

void	MessageDialog::Update(const wxString & text)
{
	tx->SetLabel(text);
	//Refresh();
	wxYield();
}

void	MessageDialog::ProgressSet(int s)
{
	prgbar->SetRange(s);
	//wxYield();
}

void	MessageDialog::UpdateProgress(int pg)
{
	//pg = pg > 100 ? 100 : pg;
	prgbar->SetValue(pg);
	wxYield();
}

void	MessageDialog::OnCancel(wxCommandEvent & event)
{
	iscancel = true;
}

//////////////////////////////////////////////////////////////////////////


MakeDirDialog::MakeDirDialog(wxWindow *parent, const wxString & title) : wxDialog(parent, -1, title, wxDefaultPosition, wxSize(550, 150))
{
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *hbox0 = new wxBoxSizer(wxHORIZONTAL);

	vbox->Add(new wxStaticText( this, -1, wxT("폴더를 만듭니다. 이름을 넣어주세요.")), 0, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);

	tc = new wxTextCtrl(this, -1, wxT("폴더"), wxDefaultPosition, wxSize(400, 20));
	hbox0->Add(tc, 1, wxLEFT| wxRIGHT, 10);

	wxButton *okButton = new wxButton(this, wxID_OK, wxT("확인"), wxDefaultPosition, wxSize(100, 30));
	wxButton *closeButton = new wxButton(this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxSize(100, 30));
	okButton->SetDefault();

	hbox->Add(okButton, 0);
	hbox->Add(closeButton, 0, 10);

	vbox->Add(hbox0, 0, wxUP, 10);
	vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	SetSizer(vbox);
	vbox->SetSizeHints(this);
	Centre();

	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MakeDirDialog::OnOk));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MakeDirDialog::OnCancel));
}

void	MakeDirDialog::OnOk(wxCommandEvent & event)
{
	returnstr = tc->GetValue();
	Valueman::GetInstance()->addvalue(L"makedir", returnstr.c_str());
	Destroy();
}

void	MakeDirDialog::OnCancel(wxCommandEvent & event)
{
	Valueman::GetInstance()->remove(L"makedir");
	Destroy();
}

//////////////////////////////////////////////////////////////////////////	이름 수정

#if 0

RenameDialog::RenameDialog(wxWindow *parent, const wxString & title, const wxString& name, const wxString& ext, bool showext) 
: wxDialog(parent, -1, title, wxDefaultPosition, wxSize(550, 150))
{
	extenable = showext;
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox0 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	vbox->Add(new wxStaticText( this, -1, wxT("새로운 이름을 넣어주세요.")), 0, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);

	tc = new wxTextCtrl(this, -1, name, wxDefaultPosition, wxSize(400, 20));
	hbox0->Add(tc, 1, wxLEFT| wxRIGHT, 10);

	if( showext )
	{
		te = new wxTextCtrl(this, -1, ext);
		hbox0->Add(te, 0, wxEXPAND | wxRIGHT, 10);
	}

	wxButton *okButton = new wxButton(this, wxID_OK, wxT("확인"), wxDefaultPosition, wxSize(100, 30));
	wxButton *closeButton = new wxButton(this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxSize(100, 30));
	okButton->SetDefault();

	hbox->Add(okButton, 0);
	hbox->Add(closeButton, 0, 10);

	vbox->Add(hbox0, 0, wxUP, 10);
	vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	vbox->SetSizeHints(this);
	SetSizer(vbox);
	Centre();

	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(RenameDialog::OnOk));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(RenameDialog::OnCancel));
}


void	RenameDialog::OnOk(wxCommandEvent & event)
{
	Valueman::GetInstance()->addvalue(L"rename_name", tc->GetValue());
	if( extenable )
		Valueman::GetInstance()->addvalue(L"rename_ext", te->GetValue());

	Destroy();
}

void	RenameDialog::OnCancel(wxCommandEvent & event)
{
	Valueman::GetInstance()->remove(L"rename");
	Destroy();
}

#else


RenameDialog::RenameDialog(wxWindow* parent, const wxString& title, const wxString& name, const wxString& ext, bool showext)
	: wxDialog( parent, wxID_ANY, title, wxDefaultPosition, wxSize( -1,-1 ), wxDEFAULT_DIALOG_STYLE )
{
	extenable = showext;
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	bSizer8->SetMinSize( wxSize( 400,-1 ) ); 
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer48;
	bSizer48 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap4 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("f2.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer48->Add( m_bitmap4, 0, wxALL, 5 );

	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("새로운 이름을 넣어주세요."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	m_staticText4->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer48->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer9->Add( bSizer48, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer92;
	bSizer92 = new wxBoxSizer( wxHORIZONTAL );

	tc = new wxTextCtrl( this, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer92->Add( tc, 1, wxALIGN_LEFT|wxALIGN_RIGHT|wxALL, 5 );

	if( showext )
	{
		te = new wxTextCtrl( this, wxID_ANY, ext, wxDefaultPosition, wxDefaultSize, 0 );
		bSizer92->Add( te, 0, wxALIGN_RIGHT|wxALL, 5 );
	}


	bSizer9->Add( bSizer92, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_button6 = new wxButton( this, wxID_OK, wxT("확인"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button6, 0, wxALL, 5 );
	m_button6->SetDefault();

	m_button7 = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button7, 0, wxALL, 5 );


	bSizer9->Add( bSizer10, 1, wxALIGN_CENTER, 5 );


	bSizer8->Add( bSizer9, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();
	bSizer8->Fit( this );

	this->Centre( wxBOTH );

	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(RenameDialog::OnOk));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(RenameDialog::OnCancel));
}

RenameDialog::~RenameDialog()
{
}

void	RenameDialog::OnOk(wxCommandEvent & event)
{
	Valueman::GetInstance()->addvalue(L"rename_name", tc->GetValue());
	if( extenable )
		Valueman::GetInstance()->addvalue(L"rename_ext", te->GetValue());

	Destroy();
}

void	RenameDialog::OnCancel(wxCommandEvent & event)
{
	Valueman::GetInstance()->remove(L"rename");
	Destroy();
}

#endif

//////////////////////////////////////////////////////////////////////////	선택된 1개 복사


EnterTextDialog::EnterTextDialog(wxWindow *parent, const wxString & title, const wxString& text,  const wxString& name, const wxString& key)
: wxDialog(parent, -1, title, wxDefaultPosition, wxSize(550, 150))
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	bSizer8->SetMinSize( wxSize( 400,-1 ) ); 
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer48;
	bSizer48 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap4 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("f2.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer48->Add( m_bitmap4, 0, wxALL, 5 );

	m_staticText4 = new wxStaticText( this, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	m_staticText4->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer48->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer9->Add( bSizer48, 0, wxEXPAND, 5 );

	inputtext = new wxTextCtrl( this, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, 0 );
	inputtext->SetMaxLength( 256 ); 

	bSizer9->Add( inputtext, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_button6 = new wxButton( this, wxID_OK, wxT("확인"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button6, 0, wxALL, 5 );
	m_button6->SetDefault();

	m_button7 = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button7, 0, wxALL, 5 );


	bSizer9->Add( bSizer10, 1, wxALIGN_CENTER, 5 );


	bSizer8->Add( bSizer9, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();
	bSizer8->Fit( this );

	this->Centre( wxBOTH );

	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(EnterTextDialog::OnOk));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(EnterTextDialog::OnCancel));

	returnkey = key;
}

void	EnterTextDialog::OnOk(wxCommandEvent & event)
{
	returnstr = inputtext->GetValue();
	Valueman::GetInstance()->addvalue(returnkey.c_str(), returnstr.c_str());
	Destroy();
}

void	EnterTextDialog::OnCancel(wxCommandEvent & event)
{
	Valueman::GetInstance()->remove(returnkey.c_str());
	Destroy();
}

//////////////////////////////////////////////////////////////////////////


DestDialog::DestDialog( wxWindow* parent, const wxString& title, const wxString& text, std::vector<String> &dirlist, bool bmakedir) 
: wxDialog( parent, wxID_ANY, title, wxDefaultPosition, wxSize( -1, 200 ), wxDEFAULT_DIALOG_STYLE )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer85;
	bSizer85 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer86;
	bSizer86 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap7 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("f2.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer86->Add( m_bitmap7, 1, wxALL, 5 );

	m_staticText42 = new wxStaticText( this, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText42->Wrap( -1 );
	m_staticText42->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer86->Add( m_staticText42, 10, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer85->Add( bSizer86, 1, wxALL|wxEXPAND, 5 );

	m_listBox1 = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
	for(size_t i=0;i<dirlist.size();i++)
		m_listBox1->Append( dirlist[i] );
	m_listBox1->SetSelection(0);
	bSizer85->Add( m_listBox1, 0, wxEXPAND|wxALL, 5 );


	bSizer6->Add( bSizer85, 0, wxEXPAND, 5 );

	if( bmakedir )
	{
		wxBoxSizer* bSizer851;
		bSizer851 = new wxBoxSizer( wxVERTICAL );

		m_checkBox10 = new wxCheckBox( this, ID_MAKE_DIR, wxT("폴더 만들고 풀기"), wxDefaultPosition, wxDefaultSize, 0 );
		bSizer851->Add( m_checkBox10, 0, wxALL|wxEXPAND, 5 );
		m_checkBox10->SetValue(true);

		bSizer6->Add( bSizer851, 0, wxEXPAND, 5 );
	}

	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxHORIZONTAL );

	m_ok = new wxButton( this, wxID_OK, wxT("확인"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer28->Add( m_ok, 0, wxALIGN_RIGHT|wxALL, 5 );
	m_ok->SetDefault();

	m_cancel = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer28->Add( m_cancel, 0, wxALIGN_RIGHT|wxALL, 5 );


	bSizer6->Add( bSizer28, 0, wxALIGN_RIGHT, 5 );


	this->SetSizer( bSizer6 );
	this->Layout();
	bSizer6->Fit( this );

	this->Centre( wxBOTH );

	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DestDialog::OnOk));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DestDialog::OnCancel));
	Connect(ID_MAKE_DIR, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(DestDialog::OnCheckBox));

	returnstr = dirlist[0];
	makedir = true;

}

DestDialog::~DestDialog()
{
}

void	DestDialog::OnOk(wxCommandEvent & event)
{
	int selected = m_listBox1->GetSelection();
	returnstr = m_listBox1->GetString(selected);
	EndModal(wxID_OK);
}

void	DestDialog::OnCancel(wxCommandEvent & event)
{
	returnstr = L"";
	EndModal(wxID_CANCEL);
}

void	DestDialog::OnCheckBox(wxCommandEvent & event)
{
	makedir = m_checkBox10->GetValue();
}

//////////////////////////////////////////////////////////////////////////


CopyDialog::CopyDialog(wxWindow *parent, const wxString & title) : wxDialog(parent, -1, title, wxDefaultPosition, wxSize(550, 150))
{
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	sourcepath = new wxTextCtrl( this, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_NOHIDESEL);
	destpath = new wxTextCtrl( this, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_NOHIDESEL);
	copysizetext = new wxTextCtrl( this, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_NOHIDESEL);
	copycount = new wxTextCtrl( this, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_NOHIDESEL);


	totalprogress = new wxGauge(this, wxID_ANY, 100, wxPoint(15, 60),  wxSize(460,20), wxGA_SMOOTH, wxDefaultValidator, wxT(""));
	currentprogress = new wxGauge(this, wxID_ANY, 100, wxPoint(15, 60),  wxSize(460,20), wxGA_SMOOTH, wxDefaultValidator, wxT(""));

	totalprogress->SetRange(1000);
	currentprogress->SetRange(1000);

	cancel = new wxButton(this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxSize(100, 30));
	cancel->SetDefault();


	vbox->Add(sourcepath, 0, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);
	vbox->Add(destpath, 0,  wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);

	vbox->Add(totalprogress, 0,  wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);
	vbox->Add(currentprogress, 0,  wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);

	vbox->Add(copysizetext, 0,  wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);
	vbox->Add(copycount, 0,  wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);
	

	hbox->Add(cancel, 0, wxRIGHT, 10);
	//vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);
	vbox->Add(hbox, 0, wxALIGN_RIGHT | wxTOP | wxBOTTOM, 10);

	vbox->SetSizeHints(this);

	SetSizer(vbox);

	winDisabler = new wxWindowDisabler(this);
	
	Centre();
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CopyDialog::OnCancel));

	Show(true);

	iscancel = false;

	wxYield();
}


void CopyDialog::OnCancel(wxCommandEvent & event)
{
	//Destroy();
	iscancel = true;
}


void CopyDialog::Update(String com, String v)
{
	if( com == L"sourcepath" )
	{
		sourcepath->SetLabel(v.c_str());
	}
	else if( com == L"destpath" )
	{
		destpath->SetLabel(v.c_str());
	}
	else if( com == L"totalprogress" )
	{
		int vv = unicode::atoi( v.c_str() );
		totalprogress->SetValue(vv);
	}
	else if( com == L"currentprogress" )
	{
		int vv = unicode::atoi( v.c_str() );
		currentprogress->SetValue(vv);
	}
	else if( com == L"totalprogressrange" )
	{
		int r = unicode::atoi( v.c_str() );
		totalprogress->SetRange(r);
	}
	else if( com == L"currentprogressrange" )
	{
		int r = unicode::atoi( v.c_str() );
		currentprogress->SetRange(r);
	}
	else if( com == L"copysize" )
	{
		copysizetext->SetLabel(v.c_str());
	}
	else if (com == L"copycount")
	{
		copycount->SetLabel(v.c_str());
	}

	wxYield();
}

bool CopyDialog::IsCancel()
{
	return iscancel;
}

bool CopyDialog::Show( bool show )
{
	if(!show)
		ReenableOtherWindows();

	return wxDialog::Show(show);
}

void CopyDialog::ReenableOtherWindows()
{
	delete winDisabler;
	winDisabler = (wxWindowDisabler *)NULL;
}

//////////////////////////////////////////////////////////////////////////


ScanDialog::ScanDialog(wxWindow *parent, const wxString& title, const wxString& txt)
: wxDialog(parent, -1, title, wxDefaultPosition, wxSize(550, 160))
{
	_cancel = false;
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	//wxStaticText *msg = new wxStaticText( this, -1, txt);
	StaticTextDC *msg = new StaticTextDC( this, txt, wxDefaultPosition, wxSize(450, 15));

// 	wxFont font = msg->GetFont();
// 	font.SetPointSize(10);
// 	font.SetWeight(wxFONTWEIGHT_BOLD);
// 	msg->SetFont(font);
	vbox->Add(msg, 0, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 15);

	//text = new wxStaticText( this, -1, txt, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_NOHIDESEL);
	text = new StaticTextDC( this, txt, wxDefaultPosition, wxSize(450, 15));
	vbox->Add(text, 0, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 15);
	//vbox->Add(text, 0, wxLEFT | wxRIGHT | wxUP | wxDOWN, 15);

	//text2 = new wxStaticText( this, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_NOHIDESEL);
	text2 = new StaticTextDC( this, L"", wxDefaultPosition, wxSize(100, 15));
	vbox->Add(text2, 0, wxEXPAND | wxLEFT | wxRIGHT /*| wxUP*/, 15);

 	cancel = new wxButton(this, ID_CANCEL, wxT("취소"), wxDefaultPosition, wxSize(80, 30));
 	cancel->SetDefault();
 	hbox->Add(cancel, 1, wxLEFT, 5);

	vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);
	//vbox->FitInside(this);

	SetSizer(vbox);
	Centre();

	winDisabler = new wxWindowDisabler(this);

	Show(true);
	Enable();

	Connect(ID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ScanDialog::OnCancel));
	wxYield();
}

void	ScanDialog::Update(String str, String txt)
{
	if( str == L"name" )
	{
		text->SetLabel(txt.c_str());
	}
	else if( str == L"count" )
	{
		text2->SetLabel(txt.c_str());
	}
	wxYield();
}

void ScanDialog::OnCancel(wxCommandEvent & event)
{
	_cancel = true;
}

bool ScanDialog::Show( bool show )
{
	if(!show)
		ReenableOtherWindows();

	return wxDialog::Show(show);
}

void ScanDialog::ReenableOtherWindows()
{
		delete winDisabler;
		winDisabler = (wxWindowDisabler *)NULL;
}

//////////////////////////////////////////////////////////////////////////

TextCtrl::TextCtrl(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style )
: wxTextCtrl(parent, id, title, pos, size, style)
{

}

bool TextCtrl::SetBackgroundColour(const wxColour& colour)
{

	return true;
}

bool TextCtrl::AcceptsFocus()
{
	return false;
}


//////////////////////////////////////////////////////////////////////////

OverWriteDialog::OverWriteDialog(wxWindow *parent, const wxString& title, const wchar *source, const wchar *dest, 
int srcsize, int destsize, SYSTEMTIME &srctime, SYSTEMTIME &desttime, unsigned long option)
: wxDialog(parent, -1, title, wxDefaultPosition, wxSize(550, 210))
{
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText *msg = new wxStaticText( this, -1, L"같은 이름의 파일이 있습니다. 덮어 쓸까요?");
	vbox->Add(msg, 0, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);

	String sourceinfo = unicode::format(L"원본 : %s", source);
	String destinfo = unicode::format(L"대상 : %s", dest);

	String sinfo = unicode::format(L"%s    %d년 %d월 %d일 %s %02d:%02d:%02d", utility::getStringSize(srcsize,3).c_str(), 
		srctime.wYear, srctime.wMonth, srctime.wDay, 
		srctime.wHour>12?L"PM":L"AM", srctime.wHour>12?srctime.wHour-12:srctime.wHour, srctime.wMinute, srctime.wSecond );

	String dinfo = unicode::format(L"%s    %d년 %d월 %d일 %s %02d:%02d:%02d", utility::getStringSize(destsize,3).c_str(), 
		desttime.wYear, desttime.wMonth, desttime.wDay, 
		desttime.wHour>12?L"PM":L"AM", desttime.wHour>12?desttime.wHour-12:desttime.wHour, desttime.wMinute, desttime.wSecond );

	StaticText *si = new StaticText( this, sinfo, 500);
	StaticText *di = new StaticText( this, dinfo, 500);
	wxFont font = si->GetFont();
	font.SetPointSize(8);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	si->SetFont(font);
	di->SetFont(font);

	vbox->Add(new StaticText( this, sourceinfo, 500), 0, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);
	vbox->Add(si, 0, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);
	vbox->Add(new wxStaticLine( this, wxID_ANY, wxPoint(15, 35), wxSize(470,1)), 0, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 5);

	vbox->Add(new StaticText( this, destinfo, 500), 0, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);
	vbox->Add(di, 0, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 10);
	vbox->Add(new wxStaticLine( this, wxID_ANY, wxPoint(15, 35), wxSize(470,1)), 0, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 5);


	ok = new wxButton(this, ID_OK, wxT("예(&Y)"), wxDefaultPosition, wxSize(80, 30));
	okall = new wxButton(this, ID_OK_ALL, wxT("모두예(&A)"), wxDefaultPosition, wxSize(80, 30));
	skip = new wxButton(this, ID_SKIP, wxT("통과(&S)"), wxDefaultPosition, wxSize(80, 30));
	skipall = new wxButton(this, ID_SKIP_ALL, wxT("모두통과(&K)"), wxDefaultPosition, wxSize(80, 30));
	cancel = new wxButton(this, wxID_CANCEL, wxT("취소(&C)"), wxDefaultPosition, wxSize(80, 30));
	rename = new wxButton(this, ID_RENAME, wxT("이름변경(&R)"), wxDefaultPosition, wxSize(80, 30));
	ok->SetDefault();

	if( !(option & DIALOG_YES ) )
		ok->Disable();
	if( !(option & DIALOG_ALLYES ) )
		okall->Disable();
	if( !(option & DIALOG_SKIP ) )
		skip->Disable();
	if( !(option & DIALOG_SKIPALL ) )
		skipall->Disable();
	if( !(option & DIALOG_CANCEL ) )
		cancel->Disable();
	if( !(option & DIALOG_RENAME ) )
		rename->Disable();

	hbox->Add(ok, 1);
	hbox->Add(okall, 1);
	hbox->Add(skip, 1);
	hbox->Add(skipall, 1);
	hbox->Add(cancel, 1);
	hbox->Add(rename, 1);

	//vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 5);
	vbox->Add(hbox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxUP, 5);

	SetSizer(vbox);
	Centre();

	Connect(ID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(OverWriteDialog::OnOk));
	Connect(ID_OK_ALL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(OverWriteDialog::OnOkAll));
	Connect(ID_SKIP, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(OverWriteDialog::OnSkip));
	Connect(ID_SKIP_ALL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(OverWriteDialog::OnSkipAll));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(OverWriteDialog::OnCancel));
	Connect(ID_RENAME, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(OverWriteDialog::OnRename));

	ret = ID_OK;
}

void OverWriteDialog::close()
{
	Destroy();
}

void OverWriteDialog::OnOk(wxCommandEvent & event)
{
	// 이것만 복사
	ret = ID_OK;
	close();
}

void OverWriteDialog::OnOkAll(wxCommandEvent & event)
{
	// 모두 복사
	ret = ID_OK_ALL;
	close();
}

void OverWriteDialog::OnSkip(wxCommandEvent & event)
{
	// 이것만 통과
	ret = ID_SKIP;
	close();
}

void OverWriteDialog::OnSkipAll(wxCommandEvent & event)
{
	// 모두 통과
	ret = ID_SKIP_ALL;
	close();
}

void OverWriteDialog::OnCancel(wxCommandEvent & event)
{
	// 여기서 멈춤
	ret = ID_CANCEL;
	close();
}

void OverWriteDialog::OnRename(wxCommandEvent & event)
{
	// 여기서 멈춤
	ret = ID_RENAME;
	close();
}

//////////////////////////////////////////////////////////////////////////


AttributeDialog::AttributeDialog( wxWindow* parent, wxWindowID id, int old_attr, const wxString& name, const wxString& title, const wxPoint& pos, const wxSize& size, long style) 
: wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer86;
	bSizer86 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap7 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("f2.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer86->Add( m_bitmap7, 1, wxALL, 5 );

	m_staticText42 = new wxStaticText( this, wxID_ANY, wxT("속성을 바꿉니다."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText42->Wrap( -1 );
	m_staticText42->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer86->Add( m_staticText42, 10, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer1->Add( bSizer86, 1, wxEXPAND, 5 );

	m_staticText1 = new wxStaticText( this, wxID_ANY, name, wxDefaultPosition, wxSize( -1,-1 ), wxST_NO_AUTORESIZE|wxDOUBLE_BORDER );
	m_staticText1->Wrap( -1 );
	m_staticText1->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );

	bSizer1->Add( m_staticText1, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("현재") ), wxVERTICAL );

	m_checkBox1 = new wxCheckBox( this, wxID_ANY, wxT("기록"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox1->SetValue( (old_attr & ATTR_ARCHIVE) ? true : false );
	m_checkBox1->Disable();	
	sbSizer1->Add( m_checkBox1, 0, wxALL, 5 );

	m_checkBox11 = new wxCheckBox( this, wxID_ANY, wxT("읽기"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox11->SetValue( (old_attr & ATTR_READONLY) ? true : false);
	m_checkBox11->Disable();	
	sbSizer1->Add( m_checkBox11, 0, wxALL, 5 );

	m_checkBox12 = new wxCheckBox( this, wxID_ANY, wxT("숨김"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox12->SetValue((old_attr & ATTR_HIDDEN) ? true : false);
	m_checkBox12->Disable();
	sbSizer1->Add( m_checkBox12, 0, wxALL, 5 );

	m_checkBox13 = new wxCheckBox( this, wxID_ANY, wxT("시스템"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox13->SetValue((old_attr & ATTR_SYSTEM) ? true : false);
	m_checkBox13->Disable();
	sbSizer1->Add( m_checkBox13, 0, wxALL, 5 );



	bSizer2->Add( sbSizer1, 1, wxALL, 5 );

	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("새 속성") ), wxVERTICAL );

	m_checkBox5 = new wxCheckBox( this, ID_ARCHIVE, wxT("기록"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer2->Add( m_checkBox5, 0, wxALL, 5 );

	m_checkBox51 = new wxCheckBox( this, ID_READONLY, wxT("읽기"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer2->Add( m_checkBox51, 0, wxALL, 5 );

	m_checkBox52 = new wxCheckBox( this, ID_HIDDEN, wxT("숨김"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer2->Add( m_checkBox52, 0, wxALL, 5 );

	m_checkBox53 = new wxCheckBox( this, ID_SYSTEM, wxT("시스템"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer2->Add( m_checkBox53, 0, wxALL, 5 );


	bSizer2->Add( sbSizer2, 1, wxALL, 5 );


	bSizer1->Add( bSizer2, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	m_checkBox20 = new wxCheckBox( this, wxID_ANY, wxT("하위 폴더 포함"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( m_checkBox20, 0, wxALL, 5 );


	bSizer3->Add( bSizer4, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );

	m_button1 = new wxButton( this, ID_OK_ALL, wxT("모두바꿈(&L)"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer5->Add( m_button1, 1, wxALL, 5 );

	m_button11 = new wxButton( this, wxID_OK, wxT("바꿈(&C)"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer5->Add( m_button11, 1, wxALL, 5 );

	m_button12 = new wxButton( this, ID_SKIP, wxT("통과(&S)"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer5->Add( m_button12, 1, wxALL, 5 );

	m_button13 = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer5->Add( m_button13, 1, wxALL, 5 );


	bSizer3->Add( bSizer5, 0, wxALL|wxEXPAND, 5 );


	bSizer1->Add( bSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );


	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );

	this->Centre( wxBOTH );

	newattribute = 0;

	Connect(ID_ARCHIVE, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(AttributeDialog::OnCheckBox));
	Connect(ID_READONLY, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(AttributeDialog::OnCheckBox));
	Connect(ID_HIDDEN, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(AttributeDialog::OnCheckBox));
	Connect(ID_SYSTEM, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(AttributeDialog::OnCheckBox));

	Connect(ID_OK_ALL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AttributeDialog::OnOkAll));
	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AttributeDialog::OnOk));
	Connect(ID_SKIP, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AttributeDialog::OnSkip));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AttributeDialog::OnCancel));

}

AttributeDialog::~AttributeDialog()
{
}

int AttributeDialog::GetAttribute()
{
	return newattribute;
}

void AttributeDialog::OnOk(wxCommandEvent & event)
{
	ret = ID_OK;
	Destroy();
}

void AttributeDialog::OnOkAll(wxCommandEvent & event)
{
	ret = ID_OK_ALL;
	Destroy();
}

void AttributeDialog::OnSkip(wxCommandEvent & event)
{
	ret = ID_SKIP;
	Destroy();
}

void AttributeDialog::OnCancel(wxCommandEvent & event)
{
	ret = ID_CANCEL;
	Destroy();
}

void AttributeDialog::OnCheckBox(wxCommandEvent & event)
{
	int id = event.GetId();
	switch(id)
	{
		case ID_ARCHIVE :
			{
				bool b = m_checkBox5->GetValue();
				newattribute = b ? newattribute | ATTR_ARCHIVE : newattribute ^ ATTR_ARCHIVE;
			}
			break;

		case ID_READONLY:
			{
				bool b = m_checkBox51->GetValue();
				newattribute = b ? newattribute | ATTR_READONLY : newattribute ^ ATTR_READONLY;
			}
			break;

		case ID_HIDDEN :
			{
				bool b = m_checkBox52->GetValue();
				newattribute = b ? newattribute | ATTR_HIDDEN : newattribute ^ ATTR_HIDDEN;
			}
			break;

		case ID_SYSTEM:
			{
				bool b = m_checkBox53->GetValue();
				newattribute = b ? newattribute | ATTR_SYSTEM : newattribute ^ ATTR_SYSTEM;
			}
			break;
	}
}

void AttributeDialog::Update(int old_attr, const wxString& name)
{
	m_staticText1 = new wxStaticText( this, wxID_ANY, name, wxDefaultPosition, wxSize( -1,-1 ), wxST_NO_AUTORESIZE|wxDOUBLE_BORDER );

	m_checkBox1 = new wxCheckBox( this, wxID_ANY, wxT("기록"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox1->SetValue( (old_attr & ATTR_ARCHIVE) ? true : false );
	m_checkBox1->Disable();	

	m_checkBox11 = new wxCheckBox( this, wxID_ANY, wxT("읽기"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox11->SetValue( (old_attr & ATTR_READONLY) ? true : false);
	m_checkBox11->Disable();	

	m_checkBox12 = new wxCheckBox( this, wxID_ANY, wxT("숨김"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox12->SetValue((old_attr & ATTR_HIDDEN) ? true : false);
	m_checkBox12->Disable();

	m_checkBox13 = new wxCheckBox( this, wxID_ANY, wxT("시스템"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox13->SetValue((old_attr & ATTR_SYSTEM) ? true : false);
	m_checkBox13->Disable();

	Refresh();
}


//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////


MoveFileDialog::MoveFileDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer86;
	bSizer86 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap7 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("f2.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer86->Add( m_bitmap7, 1, wxALL, 5 );

	m_staticText42 = new wxStaticText( this, wxID_ANY, wxT("파일을 이동합니다."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText42->Wrap( -1 );
	m_staticText42->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer86->Add( m_staticText42, 10, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer8->Add( bSizer86, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	m_targetdir = new wxStaticText( this, wxID_ANY, wxT("---"), wxDefaultPosition, wxSize( -1,-1 ), wxST_NO_AUTORESIZE|wxDOUBLE_BORDER );
	m_targetdir->Wrap( -1 );
	m_targetdir->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );

	bSizer9->Add( m_targetdir, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxHORIZONTAL );

	m_filename = new wxStaticText( this, wxID_ANY, wxT("test.cpp"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_filename->Wrap( -1 );
	bSizer22->Add( m_filename, 0, wxALL, 5 );

	m_filecount = new wxStaticText( this, wxID_ANY, wxT("(1/1)"), wxDefaultPosition, wxSize( -1,-1 ), wxALIGN_RIGHT );
	m_filecount->Wrap( -1 );
	bSizer22->Add( m_filecount, 0, wxALIGN_RIGHT|wxALL, 5 );


	bSizer9->Add( bSizer22, 0, wxEXPAND, 5 );

	m_gauge3 = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_gauge3->SetValue( 0 ); 
	bSizer9->Add( m_gauge3, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_button7 = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button7, 0, wxALL, 5 );


	bSizer9->Add( bSizer10, 1, wxALIGN_CENTER, 5 );


	bSizer8->Add( bSizer9, 1, wxALL|wxEXPAND, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();
	bSizer8->Fit( this );

	winDisabler = new wxWindowDisabler(this);
	this->Centre( wxBOTH );

	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MoveFileDialog::OnCancel));
	iscancel = false;
	wxYield();
}

MoveFileDialog::~MoveFileDialog()
{
}


bool MoveFileDialog::Show( bool show )
{
	if(!show)
		ReenableOtherWindows();

	return wxDialog::Show(show);
}

void MoveFileDialog::ReenableOtherWindows()
{
	delete winDisabler;
	winDisabler = (wxWindowDisabler *)NULL;
}

void MoveFileDialog::OnCancel(wxCommandEvent & event)
{
	iscancel = true;
}

void MoveFileDialog::Update(String com, String v)
{
	if( com == L"filename" )
	{
		m_filename->SetLabel(v.c_str());
	}
	else if( com == L"target" )
	{
		m_targetdir->SetLabel(v.c_str());
	}
	else if( com == L"filecount" )
	{
		m_filecount->SetLabel(v.c_str());
	}
	else if( com == L"progress" )
	{
		int vv = unicode::atoi( v.c_str() );
		m_gauge3->SetValue(vv);
	}

	wxYield();
}

//////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(Function, wxDialog)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, Function::OnDoubleClick)
wxEND_EVENT_TABLE()

Function::Function( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );

	m_funclist = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxSize( 500,-1 ), wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_VRULES );
	//m_funclist = new MyListCtrl( this, LIST_CTRL, wxDefaultPosition, wxSize( 500,-1 ), wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_VRULES );
	bSizer6->Add( m_funclist, 1, wxALL|wxEXPAND, 5 );

	SetUpList();

	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxHORIZONTAL );

	m_ok = new wxButton( this, wxID_OK, wxT("확인"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer28->Add( m_ok, 0, wxALIGN_RIGHT|wxALL, 5 );

	m_cancel = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer28->Add( m_cancel, 0, wxALIGN_RIGHT|wxALL, 5 );


	bSizer6->Add( bSizer28, 0, wxALIGN_RIGHT, 5 );


	this->SetSizer( bSizer6 );
	this->Layout();

	this->Centre( wxBOTH );

	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Function::OnOk));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Function::OnCancel));

}

Function::~Function()
{
}



void Function::SetUpList()
{
	wxListItem col0;
	col0.SetId(0);
	col0.SetText( L"키" );
	col0.SetWidth(50);
	m_funclist->InsertColumn(0, col0);

	wxListItem col1;
	col1.SetId(1);
	col1.SetText( L"이름" );
	m_funclist->InsertColumn(1, col1);

	wxListItem col2;
	col2.SetId(2);
	col2.SetText( L"명령" );
	m_funclist->InsertColumn(2, col2);
	m_funclist->SetColumnWidth(2, 400);

#if 0
	for (int n=0; n<12; n++)
	{
		wxListItem item;
		item.SetId(n);
		item.SetText( unicode::format(L"%d", n) );

		m_funclist->InsertItem( item );

		_Func *func = FunctionKey::GetInstance()->GetFunction(n);
		String name = unicode::format(L"F%d", n+1);
		m_funclist->SetItem(n, 0, name.c_str());
		m_funclist->SetItem(n, 1, func->name);

		String param = func->exec + L" " + func->param;
		m_funclist->SetItem(n, 2, param);
	}

#else
	for (int n=0; n<12; n++)
	{
		wxListItem item;
		item.SetId(n);
		item.SetText( unicode::format(L"%d", n) );
		m_funclist->InsertItem( item );

		String key = unicode::format(L"functionkey_f%d", n+1);
		String name = ConfigManager::GetInstance()->GetFuntionKey(key, L"name");
		String exec = ConfigManager::GetInstance()->GetFuntionKey(key, L"exec");
		String param = ConfigManager::GetInstance()->GetFuntionKey(key, L"param");

		m_funclist->SetItem(n, 0, unicode::format(L"F%d", n+1));
		m_funclist->SetItem(n, 1, name);
		m_funclist->SetItem(n, 2, exec + L" " + param);
	}

#endif

}

void Function::OnDoubleClick(wxListEvent& event)
{
	FunctionInput *dialog = new FunctionInput( this, wxID_ANY); 
	dialog->Setup(event.m_itemIndex);
	dialog->ShowModal();

	if( dialog->ret == wxID_OK )
	{
		m_funclist->ClearAll();
		SetUpList();
	}
}

void	Function::OnOk(wxCommandEvent & event)
{
	Destroy();
}

void	Function::OnCancel(wxCommandEvent & event)
{
	Destroy();
}

//////////////////////////////////////////////////////////////////////////


wxBEGIN_EVENT_TABLE(FunctionInput, wxDialog)
	EVT_COMBOBOX(wxID_ANY,FunctionInput::OnComboBoxUpdate)
wxEND_EVENT_TABLE()

void FunctionInput::OnComboBoxUpdate( wxCommandEvent& event )
{
	int id = event.GetSelection();
	m_param->SetLabel(functioncommand[id]);
}

FunctionInput::FunctionInput( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	bSizer8->SetMinSize( wxSize( 500,-1 ) ); 
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap1 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("terminal.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer34->Add( m_bitmap1, 0, wxALL|wxEXPAND, 5 );

	m_staticText18 = new wxStaticText( this, wxID_ANY, wxT("기능키를 편집합니다."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	m_staticText18->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer34->Add( m_staticText18, 0, wxALL|wxEXPAND, 10 );


	bSizer9->Add( bSizer34, 1, wxEXPAND, 5 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer9->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );

	wxBoxSizer* bSizer35;
	bSizer35 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("이름 :"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	bSizer35->Add( m_staticText4, 1, wxALL, 5 );

	m_name = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer35->Add( m_name, 9, wxALL, 5 );


	bSizer9->Add( bSizer35, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer351;
	bSizer351 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText41 = new wxStaticText( this, wxID_ANY, wxT("명령 :"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->Wrap( -1 );
	bSizer351->Add( m_staticText41, 1, wxALL|wxEXPAND, 5 );

	m_filePicker1 = new wxFilePickerCtrl( this, wxID_ANY, wxEmptyString, wxT("파일을 선택하세요"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	bSizer351->Add( m_filePicker1, 9, wxALL|wxEXPAND, 5 );


	bSizer9->Add( bSizer351, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer352;
	bSizer352 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText42 = new wxStaticText( this, wxID_ANY, wxT("인자 :"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText42->Wrap( -1 );
	bSizer352->Add( m_staticText42, 1, wxALL|wxEXPAND, 5 );

	m_param = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer352->Add( m_param, 6, wxALL, 5 );

	m_paramcombo = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_READONLY ); 
	bSizer352->Add( m_paramcombo, 3, wxALL, 5 );


	bSizer9->Add( bSizer352, 1, wxEXPAND, 5 );

	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer9->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );

	m_staticText19 = new wxStaticText( this, wxID_ANY, wxT("인자 설명 : $(DRIVE) 드라이브, $(DIR) 경로. $(FULLPATH) 전체경로, $(FILENAME) 파일이름"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText19->Wrap( -1 );
	m_staticText19->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer9->Add( m_staticText19, 0, wxALL, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_ok = new wxButton( this, wxID_OK, wxT("확인"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_ok, 0, wxALL, 5 );

	m_cancel = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_cancel, 0, wxALL, 5 );


	bSizer9->Add( bSizer10, 1, wxALIGN_CENTER, 5 );


	bSizer8->Add( bSizer9, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();

	this->Centre( wxBOTH );

	//////////////////////////////////////////////////////////////////////////

	m_paramcombo->Append(L"%$(DRIVE)");
	m_paramcombo->Append(L"%$(DIR)");
	m_paramcombo->Append(L"%$(FULLPATH)");
	m_paramcombo->Append(L"%$(FILENAME)");

	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(FunctionInput::OnOk));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(FunctionInput::OnCancel));
	selected = -1;
}

FunctionInput::~FunctionInput()
{
}

void	FunctionInput::OnOk(wxCommandEvent & event)
{
	if( selected != -1 )
	{
		String name = m_name->GetValue();
		String exec = m_filePicker1->GetPath();
		String param = m_param->GetValue();
		FunctionKey::GetInstance()->ChangeFunction(selected, name, exec, param);
	}

	ret = wxID_OK;
	Destroy();
}

void	FunctionInput::OnCancel(wxCommandEvent & event)
{
	ret = wxID_CANCEL;
	Destroy();
}

void	FunctionInput::Setup(int sel)
{
	selected = sel;
	String key = unicode::format(L"functionkey_f%d", selected+1);
	String name = ConfigManager::GetInstance()->GetFuntionKey(key, L"name");
	String exec = ConfigManager::GetInstance()->GetFuntionKey(key, L"exec");
	String param = ConfigManager::GetInstance()->GetFuntionKey(key, L"param");
	m_name->SetLabel(name);
	m_filePicker1->SetPath(exec);
	m_param->SetLabel(param);
}

//////////////////////////////////////////////////////////////////////////


DeleteDialog::DeleteDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer45;
	bSizer45 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap2 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("trash.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer45->Add( m_bitmap2, 0, wxALL, 5 );

	m_staticText22 = new wxStaticText( this, wxID_ANY, wxT("파일을 삭제 중 입니다."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText22->Wrap( -1 );
	m_staticText22->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer45->Add( m_staticText22, 0, wxALIGN_CENTER|wxALL, 10 );


	bSizer1->Add( bSizer45, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer37;
	bSizer37 = new wxBoxSizer( wxVERTICAL );

	m_filename = new wxTextCtrl( this, wxID_ANY, wxT("asdasdasd"), wxDefaultPosition, wxSize( 450,-1 ), 0 );
	m_filename->Enable( false );

	bSizer37->Add( m_filename, 0, wxALL|wxEXPAND, 5 );

	m_progressnum = new wxTextCtrl( this, wxID_ANY, wxT("(10550/10500)"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
	m_progressnum->Enable( false );

	bSizer37->Add( m_progressnum, 0, wxALIGN_RIGHT|wxALL, 5 );


	bSizer1->Add( bSizer37, 0, 0, 5 );

	m_progress = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_progress->SetValue( 0 ); 
	bSizer1->Add( m_progress, 1, wxALL|wxEXPAND, 5 );

	m_button25 = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_button25, 0, wxALIGN_CENTER|wxALL, 5 );


	this->SetSizer( bSizer1 );
	this->Layout();

	this->Centre( wxBOTH );

	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DeleteDialog::OnCancel));

	winDisabler = new wxWindowDisabler(this);

	Show(true);
	Enable(true);
	wxYield();

	iscancel = false;
}

DeleteDialog::~DeleteDialog()
{
}

void	DeleteDialog::OnCancel(wxCommandEvent & event)
{
	iscancel = true;
}

bool	DeleteDialog::Show( bool show )
{
	if(!show)
		ReenableOtherWindows();

	return wxDialog::Show(show);
}

void	DeleteDialog::ReenableOtherWindows()
{
	delete winDisabler;
	winDisabler = (wxWindowDisabler *)NULL;
}

void	DeleteDialog::Update(String com, String v)
{
	if(com == L"path")
	{
		m_filename->SetLabel(v);
	}
	else if(com == L"totalnum")
	{
		m_progressnum->SetLabel(v);
	}
	else if(com == L"progress")
	{
		int p = unicode::atoi(v.c_str());
		m_progress->SetRange(100);
		m_progress->SetValue(p);
	}
	wxYield();
}

bool	DeleteDialog::IsCancel()
{
	return iscancel;
}



//////////////////////////////////////////////////////////////////////////


SVNDialog::SVNDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer38;
	bSizer38 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer54;
	bSizer54 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer55;
	bSizer55 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText27 = new wxStaticText( this, wxID_ANY, wxT("SVN 선택"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText27->Wrap( -1 );
	bSizer55->Add( m_staticText27, 2, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString m_choice2Choices;
	m_choice2 = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice2Choices, 0 );
	m_choice2->SetSelection( 0 );
	bSizer55->Add( m_choice2, 8, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer54->Add( bSizer55, 0, wxALL|wxEXPAND, 5 );
	
	
	bSizer38->Add( bSizer54, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxBoxSizer* bSizer40;
	bSizer40 = new wxBoxSizer( wxHORIZONTAL );
	
	m_svn_update = new wxBitmapButton( this, ID_SVN_UPDATE, wxBitmap( wxT("tortoisesvn-update.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 100,100 ), wxBU_AUTODRAW );
	
	m_svn_update->SetBitmapDisabled( wxBitmap( wxT("tortoisesvn-update.bmp"), wxBITMAP_TYPE_ANY ) );
	m_svn_update->SetBitmapSelected( wxBitmap( wxT("tortoisesvn-update.bmp"), wxBITMAP_TYPE_ANY ) );
	m_svn_update->SetBitmapFocus( wxBitmap( wxT("tortoisesvn-update.bmp"), wxBITMAP_TYPE_ANY ) );
	m_svn_update->SetBitmapHover( wxBitmap( wxT("tortoisesvn-update.bmp"), wxBITMAP_TYPE_ANY ) );
	bSizer40->Add( m_svn_update, 0, wxALL, 5 );
	
	m_svn_commit = new wxBitmapButton( this, ID_SVN_COMMIT, wxBitmap( wxT("tortoisesvn-commit.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 100,100 ), wxBU_AUTODRAW );
	
	m_svn_commit->SetBitmapDisabled( wxBitmap( wxT("tortoisesvn-commit.bmp"), wxBITMAP_TYPE_ANY ) );
	m_svn_commit->SetBitmapSelected( wxBitmap( wxT("tortoisesvn-commit.bmp"), wxBITMAP_TYPE_ANY ) );
	m_svn_commit->SetBitmapFocus( wxBitmap( wxT("tortoisesvn-commit.bmp"), wxBITMAP_TYPE_ANY ) );
	m_svn_commit->SetBitmapHover( wxBitmap( wxT("tortoisesvn-commit.bmp"), wxBITMAP_TYPE_ANY ) );
	bSizer40->Add( m_svn_commit, 0, wxALL, 5 );
	
	m_svn_cleanup = new wxBitmapButton( this, ID_SVN_CLEANUP, wxBitmap( wxT("tortoisesvn-cleanup.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 100,100 ), wxBU_AUTODRAW );
	
	m_svn_cleanup->SetBitmapDisabled( wxBitmap( wxT("tortoisesvn-cleanup.bmp"), wxBITMAP_TYPE_ANY ) );
	m_svn_cleanup->SetBitmapSelected( wxBitmap( wxT("tortoisesvn-cleanup.bmp"), wxBITMAP_TYPE_ANY ) );
	m_svn_cleanup->SetBitmapFocus( wxBitmap( wxT("tortoisesvn-cleanup.bmp"), wxBITMAP_TYPE_ANY ) );
	m_svn_cleanup->SetBitmapHover( wxBitmap( wxT("tortoisesvn-cleanup.bmp"), wxBITMAP_TYPE_ANY ) );
	bSizer40->Add( m_svn_cleanup, 0, wxALL, 5 );
	
	
	sbSizer3->Add( bSizer40, 1, wxEXPAND, 5 );
	
	
	bSizer38->Add( sbSizer3, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer( wxHORIZONTAL );
	
	m_cancel = new wxButton( this, wxID_CANCEL, wxT("닫기"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer42->Add( m_cancel, 0, wxALL, 5 );
	
	
	bSizer38->Add( bSizer42, 1, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( bSizer38 );
	this->Layout();
	
	this->Centre( wxBOTH );

	Connect(ID_SVN_UPDATE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SVNDialog::OnCommand));
	Connect(ID_SVN_COMMIT, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SVNDialog::OnCommand));
	Connect(ID_SVN_CLEANUP, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SVNDialog::OnCommand));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SVNDialog::OnClose));

	ConfigManager::GetInstance()->GetSVN(svninfo);
	for (size_t i=0;i<svninfo.size(); i++)
	{
		m_choice2->Append(svninfo[i].name);
	}
	m_choice2->SetSelection( 0 );
}

SVNDialog::~SVNDialog()
{
}

// SVN 실행
void	SVNDialog::OnCommand(wxCommandEvent & event)
{
	int selectedsvn = m_choice2->GetSelection();

	if( selectedsvn >= (int)svninfo.size() )
		return;

	int id = event.GetId();
	if(id == ID_SVN_UPDATE) 
	{
		// update
		String exec = svninfo[selectedsvn].exec;
		String param = svninfo[selectedsvn].path;

		if(exec.size() > 0)
		{
			String result = L"\"" + exec + L"\"" + L" " + L"/command:update /path:" + L"\"" + param + L"\"";
			utility::CreateProcess(result);
		}
	}
	else if(id == ID_SVN_COMMIT) 
	{
		// commit
		String exec = svninfo[selectedsvn].exec;
		String param = svninfo[selectedsvn].path;

		if(exec.size() > 0)
		{
			String result = L"\"" + exec + L"\"" + L" " + L"/command:commit /path:" + L"\"" + param + L"\"";
			utility::CreateProcess(result);
		}
	}
	else if(id == ID_SVN_CLEANUP) 
	{
		// cleanup
		String exec = svninfo[selectedsvn].exec;
		String param = svninfo[selectedsvn].path;

		if(exec.size() > 0)
		{
			String result = L"\"" + exec + L"\"" + L" " + L"/command:cleanup /path:" + L"\"" + param + L"\"";
			utility::CreateProcess(result);
		}
	}

	Destroy();
}

void	SVNDialog::OnClose(wxCommandEvent & event)
{
	Destroy();
}

//////////////////////////////////////////////////////////////////////////

SVNConfig::SVNConfig( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer43;
	bSizer43 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap1 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("f2.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer34->Add( m_bitmap1, 0, wxALL|wxEXPAND, 5 );

	m_staticText18 = new wxStaticText( this, wxID_ANY, wxT("SVN 설정을 편집합니다."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	m_staticText18->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer34->Add( m_staticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 10 );


	bSizer43->Add( bSizer34, 0, wxEXPAND, 5 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer43->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );

	wxBoxSizer* bSizer77;
	bSizer77 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText38 = new wxStaticText( this, wxID_ANY, wxT("SVN 대표이름"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText38->Wrap( -1 );
	bSizer77->Add( m_staticText38, 0, wxALL, 5 );

	m_textCtrl12 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer77->Add( m_textCtrl12, 1, wxALL, 5 );


	bSizer43->Add( bSizer77, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer44;
	bSizer44 = new wxBoxSizer( wxHORIZONTAL );

	bSizer44->SetMinSize( wxSize( 400,-1 ) ); 
	m_staticText27 = new wxStaticText( this, wxID_ANY, wxT("SVN 실행파일"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText27->Wrap( -1 );
	bSizer44->Add( m_staticText27, 0, wxALL, 5 );

	m_filePicker2 = new wxFilePickerCtrl( this, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
	bSizer44->Add( m_filePicker2, 9, wxALL, 5 );


	bSizer43->Add( bSizer44, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer45;
	bSizer45 = new wxBoxSizer( wxHORIZONTAL );

	bSizer45->SetMinSize( wxSize( 400,-1 ) ); 
	m_staticText28 = new wxStaticText( this, wxID_ANY, wxT("SVN 대상경로"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText28->Wrap( -1 );
	bSizer45->Add( m_staticText28, 0, wxALL, 5 );

	m_dirPicker1 = new wxDirPickerCtrl( this, wxID_ANY, wxEmptyString, wxT("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE );
	bSizer45->Add( m_dirPicker1, 9, wxALL, 5 );


	bSizer43->Add( bSizer45, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer46;
	bSizer46 = new wxBoxSizer( wxHORIZONTAL );

	m_ok = new wxButton( this, wxID_OK, wxT("확인"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer46->Add( m_ok, 1, wxALL, 5 );

	m_cancel = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer46->Add( m_cancel, 0, wxALL, 5 );


	bSizer43->Add( bSizer46, 1, wxALIGN_CENTER_HORIZONTAL, 5 );


	this->SetSizer( bSizer43 );
	this->Layout();
	bSizer43->Fit( this );

	this->Centre( wxBOTH );

	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SVNConfig::OnCommand));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SVNConfig::OnCommand));
}

SVNConfig::~SVNConfig()
{
}

void	SVNConfig::SetValue(String name, String exec, String path)
{
	editinfo = name;

	m_textCtrl12->SetValue(name);
	m_filePicker2->SetPath(exec);
	m_dirPicker1->SetPath(path);
}

void	SVNConfig::OnCommand(wxCommandEvent & event)
{
	int id = event.GetId();
	if( id == wxID_OK )
	{
		String name = m_textCtrl12->GetValue();
		String exec = m_filePicker2->GetPath();
		String targetdir = m_dirPicker1->GetPath();

		if( name.empty() || exec.empty() || targetdir.empty() )
		{
			// 비어있다!
			wxMessageDialog dial(NULL, L"내용이 비어있습니다.", L"오류", wxCENTER | wxOK);
			dial.ShowModal();
			return;
		}

		// 수정이면...
		if( !editinfo.empty() )
		{
			ConfigManager::GetInstance()->RemoveSVN(editinfo);
		}

		// 이름 검사
		if( ConfigManager::GetInstance()->ExistSVN(name) )
		{
			wxMessageDialog dial(NULL, L"이미 있는 이름입니다.", L"오류", wxCENTER | wxOK);
			dial.ShowModal();
			return;
		}

		ConfigManager::GetInstance()->SetSVN(name, exec, targetdir);
		ConfigManager::GetInstance()->Save();
	}

	Destroy();
}


//////////////////////////////////////////////////////////////////////////



ExecParam::ExecParam( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	bSizer8->SetMinSize( wxSize( 400,-1 ) ); 
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer48;
	bSizer48 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap4 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("f2.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer48->Add( m_bitmap4, 0, wxALL, 5 );

	//m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("파일을 열기위해 사용할 인자를 넣어주세요."), wxDefaultPosition, wxDefaultSize, 0 );
	//m_staticText4->Wrap( -1 );
	m_staticText4 = new StaticTextDC( this, wxT("파일을 열기위해 사용할 인자를 넣어주세요."), wxDefaultPosition, wxSize(100, 20));

	m_staticText4->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer48->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer9->Add( bSizer48, 0, wxEXPAND, 5 );

	m_param = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer9->Add( m_param, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_button6 = new wxButton( this, wxID_OK, wxT("확인"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button6, 0, wxALL, 5 );

	m_button7 = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button7, 0, wxALL, 5 );


	bSizer9->Add( bSizer10, 1, wxALIGN_CENTER, 5 );


	bSizer8->Add( bSizer9, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();
	bSizer8->Fit( this );

	this->Centre( wxBOTH );

	m_button6->SetDefault();
	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ExecParam::OnOk));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ExecParam::OnCancel));

	param.clear();
	ret = wxID_CANCEL;
}

ExecParam::~ExecParam()
{
}

void	ExecParam::OnOk(wxCommandEvent & event)
{
	param = m_param->GetValue();
	ret = wxID_OK;
	Destroy();
}

void	ExecParam::OnCancel(wxCommandEvent & event)
{
	ret = wxID_CANCEL;
	Destroy();
}

//////////////////////////////////////////////////////////////////////////


Compress::Compress( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) 
: wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	bSizer8->SetMinSize( wxSize( 400,-1 ) ); 
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer63;
	bSizer63 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap5 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("f2.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer63->Add( m_bitmap5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("압축 파일 이름을 넣어 주세요."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	m_staticText4->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer63->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer9->Add( bSizer63, 1, wxEXPAND, 5 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer9->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );

	m_textCtrl1 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer9->Add( m_textCtrl1, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText8 = new wxStaticText( this, wxID_ANY, wxT("압축수준"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	bSizer20->Add( m_staticText8, 0, wxALL, 5 );

	wxArrayString m_choice1Choices;
	m_choice1 = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice1Choices, 0 );
	m_choice1->Append(L"압축 안함");
	m_choice1->Append(L"빠른 압축");
	m_choice1->Append(L"일반 압축");
	m_choice1->Append(L"최대 압축");
	m_choice1->SetSelection( 2 );
	bSizer20->Add( m_choice1, 0, wxALL, 5 );
	compressmethod = 2;

	m_staticText82 = new wxStaticText( this, wxID_ANY, wxT("압축종류"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText82->Wrap( -1 );
	bSizer20->Add( m_staticText82, 0, wxALL, 5 );

	wxArrayString m_archiveChoices;
	m_archive = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_archiveChoices, 0 );
	m_archive->Append(L"ZIP");
	m_archive->Append(L"7-ZIP");
	m_archive->Append(L"LZH");
	m_archive->SetSelection( 0 );
	bSizer20->Add( m_archive, 0, wxALL, 5 );
	compressarchive = 0;

	bSizer9->Add( bSizer20, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer201;
	bSizer201 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText81 = new wxStaticText( this, wxID_ANY, wxT("비밀번호"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	bSizer201->Add( m_staticText81, 0, wxALL, 5 );

	m_textCtrl5 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
	bSizer201->Add( m_textCtrl5, 0, wxALL, 5 );


	bSizer9->Add( bSizer201, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_button6 = new wxButton( this, wxID_OK, wxT("확인"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button6, 0, wxALL, 5 );
	m_button6->SetDefault();

	m_button7 = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button7, 0, wxALL, 5 );


	bSizer9->Add( bSizer10, 1, wxALIGN_CENTER, 5 );
	bSizer8->Add( bSizer9, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();
	bSizer8->Fit( this );

	this->Centre( wxBOTH );

	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Compress::OnOk));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Compress::OnCancel));
}

Compress::~Compress()
{
}

void Compress::OnOk(wxCommandEvent & event)
{
	compressfilename = m_textCtrl1->GetValue();
	compresspassword = m_textCtrl5->GetValue();
	compressmethod = m_choice1->GetSelection();
	compressarchive = m_archive->GetSelection();

	ret = ID_OK;
	EndModal(0);
}

void Compress::OnCancel(wxCommandEvent & event)
{
	ret = ID_CANCEL;
	EndModal(-1);
}


CompressProgress::CompressProgress( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) 
: wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer63;
	bSizer63 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap5 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("zip.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer63->Add( m_bitmap5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("파일을 압축 하고 있습니다."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	m_staticText4->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer63->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer9->Add( bSizer63, 1, wxEXPAND, 5 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer9->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );

	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("압축파일 :"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	m_staticText1->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer24->Add( m_staticText1, 1, wxALL|wxEXPAND, 5 );

	m_staticText16 = new wxStaticText( this, wxID_ANY, wxT("name"), wxDefaultPosition, wxSize( -1,-1 ), wxST_NO_AUTORESIZE|wxDOUBLE_BORDER );
	m_staticText16->Wrap( -1 );
	m_staticText16->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );

	bSizer24->Add( m_staticText16, 6, wxALL|wxEXPAND, 5 );


	bSizer9->Add( bSizer24, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText21 = new wxStaticText( this, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxSize( 380,-1 ), 0 );
	m_staticText21->Wrap( -1 );
	bSizer22->Add( m_staticText21, 8, wxALL, 5 );

	m_staticText22 = new wxStaticText( this, wxID_ANY, wxT("(1/1)"), wxPoint( 0,-1 ), wxSize( 100,-1 ), wxALIGN_RIGHT );
	m_staticText22->Wrap( -1 );
	bSizer22->Add( m_staticText22, 2, wxALIGN_RIGHT|wxALL, 5 );


	bSizer9->Add( bSizer22, 0, 0, 5 );

	m_staticText14 = new wxStaticText( this, wxID_ANY, wxT("(12310293/19192931929)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	bSizer9->Add( m_staticText14, 0, wxALL, 5 );

	m_gauge1 = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_gauge1->SetValue( 0 ); 
	bSizer9->Add( m_gauge1, 1, wxALL|wxEXPAND, 5 );

	m_gauge11 = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_gauge11->SetValue( 0 ); 
	bSizer9->Add( m_gauge11, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_button7 = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button7, 1, wxALL, 5 );


	bSizer9->Add( bSizer10, 0, wxALIGN_CENTER, 5 );


	bSizer8->Add( bSizer9, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();
	bSizer8->Fit( this );

	this->Centre( wxBOTH );

	iscancel = false;

	winDisabler = new wxWindowDisabler(this);

	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CompressProgress::OnCancel));
	wxYield();
}

CompressProgress::~CompressProgress()
{
}

void CompressProgress::OnCancel(wxCommandEvent & event)
{
	iscancel = true;
}


void CompressProgress::Update(String com, String v)
{
	if( com == L"zipfilename" )
	{
		m_staticText16->SetLabel(v.c_str());
	}
	else if( com == L"currentfile" )
	{
		// 현재 작업중 파일
		m_staticText21->SetLabel(v.c_str());
	}
	else if( com == L"filecount" )
	{
		// (1/1)
		m_staticText22->SetLabel(v.c_str());
	}
	else if( com == L"zipprogress" )
	{
		int vv = unicode::atoi( v.c_str() );
		m_gauge1->SetValue(vv);
	}
	else if( com == L"totalprogress" )
	{
		int vv = unicode::atoi( v.c_str() );
		m_gauge11->SetValue(vv);
	}
	else if( com == L"progresssize" )
	{
		m_staticText14->SetLabel(v.c_str());
	}

	wxYield();
}

bool CompressProgress::Show( bool show )
{
	if(!show)
		ReenableOtherWindows();

	return wxDialog::Show(show);
}

void CompressProgress::ReenableOtherWindows()
{
	delete winDisabler;
	winDisabler = (wxWindowDisabler *)NULL;
}

//////////////////////////////////////////////////////////////////////////


UnCompressProgress::UnCompressProgress( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	bSizer8->SetMinSize( wxSize( 400,-1 ) ); 
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer63;
	bSizer63 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap5 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("zip.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer63->Add( m_bitmap5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("압축을 해제 하고 있습니다."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	m_staticText4->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer63->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer9->Add( bSizer63, 1, wxEXPAND, 5 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer9->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );

	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("압축파일 :"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	m_staticText1->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer24->Add( m_staticText1, 1, wxALL|wxEXPAND, 5 );

	m_staticText16 = new wxStaticText( this, wxID_ANY, wxT("name"), wxDefaultPosition, wxSize( -1,-1 ), wxST_NO_AUTORESIZE|wxDOUBLE_BORDER );
	m_staticText16->Wrap( -1 );
	m_staticText16->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );

	bSizer24->Add( m_staticText16, 6, wxALL, 5 );


	bSizer9->Add( bSizer24, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText21 = new wxStaticText( this, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxSize( 380,-1 ), 0 );
	m_staticText21->Wrap( -1 );
	bSizer22->Add( m_staticText21, 0, wxALL, 5 );

	m_staticText22 = new wxStaticText( this, wxID_ANY, wxT("(1/1)"), wxPoint( 0,-1 ), wxSize( 100,-1 ), wxALIGN_RIGHT );
	m_staticText22->Wrap( -1 );
	bSizer22->Add( m_staticText22, 0, wxALIGN_RIGHT|wxALL, 5 );


	bSizer9->Add( bSizer22, 0, 0, 5 );

	m_staticText14 = new wxStaticText( this, wxID_ANY, wxT("(12310293/19192931929)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	bSizer9->Add( m_staticText14, 0, wxALL, 5 );

	m_gauge1 = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_gauge1->SetValue( 0 ); 
	bSizer9->Add( m_gauge1, 1, wxALL|wxEXPAND, 5 );

	m_gauge11 = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_gauge11->SetValue( 0 ); 
	bSizer9->Add( m_gauge11, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_button7 = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button7, 0, wxALL, 5 );


	bSizer9->Add( bSizer10, 0, wxALIGN_CENTER, 5 );


	bSizer8->Add( bSizer9, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();
	bSizer8->Fit( this );

	this->Centre( wxBOTH );
	
	iscancel = false;
	winDisabler = new wxWindowDisabler(this);
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(UnCompressProgress::OnCancel));
	wxYield();
}

UnCompressProgress::~UnCompressProgress()
{
}

void UnCompressProgress::OnCancel(wxCommandEvent & event)
{
	iscancel = true;
}

void UnCompressProgress::Update(String com, String v)
{
	if( com == L"zipfilename" )
	{
		m_staticText16->SetLabel(v.c_str());
	}
	else if( com == L"currentfile" )
	{
		// 현재 작업중 파일
		m_staticText21->SetLabel(v.c_str());
	}
	else if( com == L"filecount" )
	{
		// (1/1)
		m_staticText22->SetLabel(v.c_str());
	}
	else if( com == L"zipprogress" )
	{
		int vv = unicode::atoi( v.c_str() );
		m_gauge1->SetValue(vv);
	}
	else if( com == L"totalprogress" )
	{
		int vv = unicode::atoi( v.c_str() );
		m_gauge11->SetValue(vv);
	}
	else if( com == L"progresssize" )
	{
		m_staticText14->SetLabel(v.c_str());
	}

	wxYield();
}

bool UnCompressProgress::Show( bool show )
{
	if(!show)
		ReenableOtherWindows();

	return wxDialog::Show(show);
}

void UnCompressProgress::ReenableOtherWindows()
{
	delete winDisabler;
	winDisabler = (wxWindowDisabler *)NULL;
}

//////////////////////////////////////////////////////////////////////////


InputPasswd::InputPasswd( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	bSizer8->SetMinSize( wxSize( 400,-1 ) ); 
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer48;
	bSizer48 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap4 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("zip.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer48->Add( m_bitmap4, 0, wxALL, 5 );

	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("압축을 풀기 위한 암호를 넣어주세요."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	m_staticText4->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer48->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer9->Add( bSizer48, 0, wxEXPAND, 5 );

	m_param = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
	bSizer9->Add( m_param, 0, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_button6 = new wxButton( this, wxID_OK, wxT("확인"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button6, 0, wxALL, 5 );

	m_button7 = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_button7, 0, wxALL, 5 );


	bSizer9->Add( bSizer10, 1, wxALIGN_CENTER, 5 );


	bSizer8->Add( bSizer9, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();
	bSizer8->Fit( this );

	this->Centre( wxBOTH );

	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(InputPasswd::OnOk));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(InputPasswd::OnCancel));
}

InputPasswd::~InputPasswd()
{
}

void InputPasswd::OnOk(wxCommandEvent & event)
{
	param = m_param->GetValue();
	ret = wxID_OK;
	Destroy();
}

void InputPasswd::OnCancel(wxCommandEvent & event)
{
	ret = wxID_CANCEL;
	Destroy();
}

//////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(Options, wxDialog)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, Options::OnDoubleClick)
wxEND_EVENT_TABLE()

Options::Options( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{

	m_listBox3 = NULL;

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer46;
	bSizer46 = new wxBoxSizer( wxVERTICAL );

	m_notebook1 = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxSize( 500,400 ), 0 );
	m_color = new wxPanel( m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer471;
	bSizer471 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer481;
	bSizer481 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizer51;
	sbSizer51 = new wxStaticBoxSizer( new wxStaticBox( m_color, wxID_ANY, wxT("일반색상") ), wxVERTICAL );

	m_listBox6 = new ColorCheckListBox(this, m_color, ID_CONFIGCOLOR, SYSTEM_COLOR);

	//////////////////////////////////////////////////////////////////////////

	LoadSystemConfig();

	//////////////////////////////////////////////////////////////////////////

	sbSizer51->Add( m_listBox6, 1, wxALL|wxEXPAND, 5 );


	bSizer481->Add( sbSizer51, 1, wxALL|wxEXPAND, 5 );


	bSizer471->Add( bSizer481, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer491;
	bSizer491 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizer61;
	sbSizer61 = new wxStaticBoxSizer( new wxStaticBox( m_color, wxID_ANY, wxT("확장자 색상") ), wxVERTICAL );

	m_listBox3 = new ColorCheckListBox(this, m_color, ID_COLORBOX, EXT_COLOR);

	//////////////////////////////////////////////////////////////////////////

	LoadExtConfig();

	//////////////////////////////////////////////////////////////////////////

	sbSizer61->Add( m_listBox3, 9, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer56;
	bSizer56 = new wxBoxSizer( wxHORIZONTAL );

	m_button24 = new wxButton( m_color, ID_COLOR_ADD, wxT("추가"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer56->Add( m_button24, 0, wxALL, 5 );

	m_button25 = new wxButton( m_color, ID_COLOR_EDIT, wxT("편집"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer56->Add( m_button25, 0, wxALL, 5 );

	m_button26 = new wxButton( m_color, ID_COLOR_REMOVE, wxT("삭제"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer56->Add( m_button26, 0, wxALL, 5 );


	sbSizer61->Add( bSizer56, 1, wxEXPAND, 5 );


	bSizer491->Add( sbSizer61, 1, wxALL|wxEXPAND, 5 );


	bSizer471->Add( bSizer491, 2, wxEXPAND, 5 );


	m_color->SetSizer( bSizer471 );
	m_color->Layout();
	bSizer471->Fit( m_color );
	m_notebook1->AddPage( m_color, wxT("색상"), false );
	m_svn = new wxPanel( m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer4711;
	bSizer4711 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer4811;
	bSizer4811 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizer511;
	sbSizer511 = new wxStaticBoxSizer( new wxStaticBox( m_svn, wxID_ANY, wxT("SVN 설정") ), wxVERTICAL );

	m_listBox4 = new wxListBox( m_svn, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	sbSizer511->Add( m_listBox4, 1, wxALL|wxEXPAND, 5 );

	wxStaticBoxSizer* sbSizer141;
	sbSizer141 = new wxStaticBoxSizer( new wxStaticBox( m_svn, wxID_ANY, wxEmptyString ), wxVERTICAL );

	wxBoxSizer* bSizer353;
	bSizer353 = new wxBoxSizer( wxHORIZONTAL );

	m_button29 = new wxButton( m_svn, ID_SVN_ADD, wxT("추가"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer353->Add( m_button29, 0, wxALL, 5 );

	m_button30 = new wxButton( m_svn, ID_SVN_EDIT, wxT("수정"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer353->Add( m_button30, 0, wxALL, 5 );

	m_button31 = new wxButton( m_svn, ID_SVN_REMOVE, wxT("삭제"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer353->Add( m_button31, 0, wxALL, 5 );


	sbSizer141->Add( bSizer353, 0, wxALIGN_CENTER|wxALL, 5 );


	sbSizer511->Add( sbSizer141, 0, wxEXPAND, 5 );


	bSizer4811->Add( sbSizer511, 1, wxALL|wxEXPAND, 5 );


	bSizer4711->Add( bSizer4811, 1, wxEXPAND, 5 );


	m_svn->SetSizer( bSizer4711 );
	m_svn->Layout();
	bSizer4711->Fit( m_svn );
	m_notebook1->AddPage( m_svn, wxT("SVN 설정"), false );
	m_function = new wxPanel( m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer47111;
	bSizer47111 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer48111;
	bSizer48111 = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizer5111;
	sbSizer5111 = new wxStaticBoxSizer( new wxStaticBox( m_function, wxID_ANY, wxT("기능키 설정") ), wxVERTICAL );

	m_listCtrl3 = new wxListCtrl( m_function, ID_FUNCTION_EDIT, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_VRULES );
	sbSizer5111->Add( m_listCtrl3, 1, wxALL|wxEXPAND, 5 );

	//////////////////////////////////////////////////////////////////////////

	LoadFunction();

	//////////////////////////////////////////////////////////////////////////

	bSizer48111->Add( sbSizer5111, 1, wxALL|wxEXPAND, 5 );


	bSizer47111->Add( bSizer48111, 1, wxEXPAND, 5 );


	m_function->SetSizer( bSizer47111 );
	m_function->Layout();
	bSizer47111->Fit( m_function );
	m_notebook1->AddPage( m_function, wxT("기능키"), false );
	m_option = new wxPanel( m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer9;
	sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( m_option, wxID_ANY, wxT("일반 설정") ), wxVERTICAL );

	m_showimage = new wxCheckBox( m_option, ID_IMAGEPREVIEW, wxT("이미지 프리뷰 보기"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer9->Add( m_showimage, 0, wxALL, 5 );

	wxBoxSizer* bSizer101;
	bSizer101 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText49 = new wxStaticText( m_option, wxID_ANY, wxT("폰트 설정"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText49->Wrap( -1 );
	bSizer101->Add( m_staticText49, 0, wxALL, 5 );

	String _fontname = ConfigManager::GetInstance()->GetConfigValue(L"font");

	m_selectfont = new wxButton( m_option, ID_SELECTFONT, _fontname, wxDefaultPosition, wxSize(200, wxDefaultSize.y), 0 );
	bSizer101->Add( m_selectfont, 0, wxALL, 5 );


	sbSizer9->Add( bSizer101, 1, wxEXPAND, 5 );

	m_option->SetSizer( sbSizer9 );
	m_option->Layout();
	sbSizer9->Fit( m_option );
	m_notebook1->AddPage( m_option, wxT("일반 설정"), false );

	bSizer46->Add( m_notebook1, 1, wxEXPAND | wxALL, 5 );

	wxBoxSizer* bSizer86;
	bSizer86 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer87;
	bSizer87 = new wxBoxSizer( wxHORIZONTAL );

	m_ok = new wxButton( this, wxID_OK, wxT("확인"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer87->Add( m_ok, 0, wxALIGN_RIGHT|wxALL, 5 );

	m_cancel = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer87->Add( m_cancel, 0, wxALIGN_RIGHT|wxALL, 5 );


	bSizer86->Add( bSizer87, 1, wxALIGN_RIGHT, 5 );


	bSizer46->Add( bSizer86, 0, wxALL|wxEXPAND, 5 );


	this->SetSizer( bSizer46 );
	this->Layout();
	bSizer46->Fit( this );

	this->Centre( wxBOTH );

	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Options::OnOk));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Options::OnCancel));

	Connect(ID_COLOR_ADD, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Options::OnBtnDown));
	Connect(ID_COLOR_EDIT, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Options::OnBtnDown));
	Connect(ID_COLOR_REMOVE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Options::OnBtnDown));

	Connect(ID_SVN_ADD, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Options::OnSvnBtnDown));
	Connect(ID_SVN_EDIT, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Options::OnSvnBtnDown));
	Connect(ID_SVN_REMOVE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Options::OnSvnBtnDown));

	Connect(ID_IMAGEPREVIEW, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(Options::OnImageviewCheckBox));

	Connect(ID_SELECTFONT, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Options::OnSelectFont));
	

	//Connect(ID_FUNCTION_EDIT, wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler(Options::OnDoubleClick));

	LoadSVNConfig();
	LoadCommonConfig();

}

Options::~Options()
{
}

void Options::OnOk(wxCommandEvent & event)
{
	ret = wxID_OK;
	Destroy();
}

void Options::OnCancel(wxCommandEvent & event)
{
	ret = wxID_CANCEL;
	Destroy();
}

void Options::OnBtnDown(wxCommandEvent & event)
{
	int id = event.GetId();
	switch(id)
	{
		case ID_COLOR_ADD :
			{
				String key = L"addext";
				EnterTextDialog dialog( this, L"확장자 추가", L"추가", L"", key.c_str());
				dialog.ShowModal();

				String ret =  Valueman::GetInstance()->getvalue(key.c_str());
				if( !ret.empty() )
				{
					// 이미 있는가 검사
					transform(ret.begin(), ret.end(), ret.begin(), toupper);
					bool exist = ColorManager::GetInstance()->ExistColor(EXT_COLOR, ret);

					if( exist )
					{
						wxMessageDialog dial(NULL, L"이미 있는 확장자 입니다.", L"오류", wxCENTER | wxOK);
						dial.ShowModal();
					}
					else
					{
						wxColourData data;
						data.SetChooseFull(TRUE);
						for (int i = 0; i < 16; i++)
						{
							wxColour colour(i*16, i*16, i*16);
							data.SetCustomColour(i, colour);
						}
						wxColourDialog dialog(this, &data);
						if (dialog.ShowModal() == wxID_OK)
						{
							wxColourData retData = dialog.GetColourData();
							wxColour col = retData.GetColour();

							unsigned long c = (col.Red() << 16) | (col.Green() << 8) | col.Blue();
							ColorManager::GetInstance()->SetColor(EXT_COLOR, ret, c);
							LoadExtConfig();
						}
					}
				}
			}
			break;

		case ID_COLOR_EDIT :
			break;

		case ID_COLOR_REMOVE :
			{
				int i = m_listBox3->GetSelection();
				if( i >= 0 )
				{
					wxMessageDialog dial(NULL, L"삭제하시겠습니까?", L"삭제", wxCENTER | wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
					int ret = dial.ShowModal();
					if( ret == wxID_YES )
					{
						String tag = m_listBox3->GetSelectedItem(i);
						ColorManager::GetInstance()->RemoveColor(EXT_COLOR, tag);
						LoadExtConfig();
					}
				}
			}

			break;
	}
}

void Options::OnSvnBtnDown(wxCommandEvent & event)
{
	int id = event.GetId();
	switch(id)
	{
		case ID_SVN_ADD :
			{
				SVNConfig configdlg(this);
				configdlg.ShowModal();
				// Load SVN config
				LoadSVNConfig();
			}
			break;

		case ID_SVN_EDIT :
			{
				int sel = m_listBox4->GetSelection();
				if( sel >= 0 )
				{
					SVNInfo info;
					String name = m_listBox4->GetString(sel);
					bool ret = ConfigManager::GetInstance()->GetSVN(name, info);
					if( ret )
					{
						SVNConfig configdlg(this);
						configdlg.SetValue(info.name, info.exec, info.path);
						configdlg.ShowModal();

						LoadSVNConfig();
					}
				}
			}
			break;

		case ID_SVN_REMOVE :
			{
				int sel = m_listBox4->GetSelection();
				if( sel >= 0 )
				{
					wxMessageDialog dial(NULL, L"삭제하시겠습니까?", L"삭제", wxCENTER | wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
					int ret = dial.ShowModal();
					if( ret == wxID_YES )
					{
						String name = m_listBox4->GetString(sel);
						ConfigManager::GetInstance()->RemoveSVN(name);
						ConfigManager::GetInstance()->Save();

						LoadSVNConfig();
					}
				}
			}
			break;
	}
}

void Options::OnImageviewCheckBox(wxCommandEvent & event)
{
	bool ch = m_showimage->GetValue();
	ConfigManager::GetInstance()->SetCommonOption(L"imagepreview", ch ? L"true" : L"false");
	ConfigManager::GetInstance()->Save();
}

void Options::OnSelectFont(wxCommandEvent & event)
{
	wxFontDialog *fontDialog = new wxFontDialog(this);

	if (fontDialog->ShowModal() == wxID_OK) {
		//st->SetFont(fontDialog->GetFontData().GetChosenFont());

		wxFont font = fontDialog->GetFontData().GetChosenFont();
		String name = font.GetFaceName();

		m_selectfont->SetLabelText(name);
		CFont::GetInstance()->setFont(name);
	}

}

void Options::LoadSVNConfig()
{
	m_listBox4->Clear();

	std::vector<SVNInfo> info;
	ConfigManager::GetInstance()->GetSVN(info);
	for (size_t i=0; i<info.size(); i++)
	{
		m_listBox4->Append(info[i].name);
	}
}

void Options::LoadSystemConfig()
{
	struct 
	{
		String tag;
		String desc;
	} systemcol[] = { 
		L"MAINBG", L"배경", 
		L"FOLDER", L"폴더",
		L"DRIVE", L"드라이브",
		L"FILESIZE",L"파일크기",
		L"FILEATTR",L"파일속성",
		L"TOPBG",L"상단 배경색",
		L"TOPFONT",L"상단 글꼴색",
		L"TOPFONTBG",L"상단 글꼴 배경색",
		L"TOPFONTOUTLINE",L"상단 글꼴 외각색",
		L"BOTTOMBG",L"하단 배경색",
		L"BOTTOMFONT",L"하단 글꼴색",
		L"BOTTOMFONTBG",L"하단 글꼴 배경색",
		L"BOTTOMFONTOUTLINE",L"하단 글꼴 외각색",
	};

	m_listBox6->Reset();

	for (int i=0; i<_countof(systemcol); i++)
	{
		unsigned long col = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, systemcol[i].tag);
		wxColor bgcolor = wxColor(
			(col&0x00FF0000)>>16, 
			(col&0x0000FF00)>>8, 
			(col&0x000000FF));
		m_listBox6->AppendItem(systemcol[i].desc, systemcol[i].tag, bgcolor, wxColor(255,255,255), wxColor(0,0,0));
	}

	m_listBox6->Refresh();
	if(m_listBox3)
		LoadExtConfig();
}

void Options::LoadExtConfig()
{
	std::map<String, unsigned long> table = ColorManager::GetInstance()->GetAlltable(EXT_COLOR);
	std::map<String, unsigned long>::iterator it = table.begin();
	unsigned long bgcol = ColorManager::GetInstance()->GetColor(SYSTEM_COLOR, L"MAINBG");

	m_listBox3->Reset();

	for (; it != table.end(); it++)
	{
		wxColor color = wxColor(
			(it->second&0x00FF0000)>>16, 
			(it->second&0x0000FF00)>>8, 
			(it->second&0x000000FF));

		wxColor mainbg = wxColor(
			(bgcol&0x00FF0000)>>16,
			(bgcol&0x0000FF00)>>8,
			(bgcol&0x000000FF));

		m_listBox3->AppendItem(it->first, it->first, color, mainbg, color);
	}

	m_listBox3->Refresh();
}

void	Options::OnDoubleClick(wxListEvent& event)
{
	FunctionInput *dialog = new FunctionInput( this, wxID_ANY); 
	dialog->Setup(event.m_itemIndex);
	dialog->ShowModal();

	if( dialog->ret == wxID_OK )
	{
		ConfigManager::GetInstance()->Save();

		m_listCtrl3->ClearAll();
		LoadFunction();
	}
}

void Options::LoadFunction()
{
	wxListItem col0;
	col0.SetId(0);
	col0.SetText( L"키" );
	col0.SetWidth(50);
	m_listCtrl3->InsertColumn(0, col0);

	wxListItem col1;
	col1.SetId(1);
	col1.SetText( L"이름" );
	m_listCtrl3->InsertColumn(1, col1);

	wxListItem col2;
	col2.SetId(2);
	col2.SetText( L"명령" );
	m_listCtrl3->InsertColumn(2, col2);
	m_listCtrl3->SetColumnWidth(2, 400);

	for (int n=0; n<12; n++)
	{
		wxListItem item;
		item.SetId(n);
		item.SetText( unicode::format(L"%d", n) );
		m_listCtrl3->InsertItem( item );

		String key = unicode::format(L"functionkey_f%d", n+1);
		String name = ConfigManager::GetInstance()->GetFuntionKey(key, L"name");
		String exec = ConfigManager::GetInstance()->GetFuntionKey(key, L"exec");
		String param = ConfigManager::GetInstance()->GetFuntionKey(key, L"param");

		m_listCtrl3->SetItem(n, 0, unicode::format(L"F%d", n+1));
		m_listCtrl3->SetItem(n, 1, name);
		m_listCtrl3->SetItem(n, 2, exec + L" " + param);
	}
}

void Options::LoadCommonConfig()
{
	String imagepreview = ConfigManager::GetInstance()->GetCommonOption(L"imagepreview");
	m_showimage->SetValue(imagepreview == L"true" ? true : false);
}

//////////////////////////////////////////////////////////////////////////

AboutDialog::AboutDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer40;
	bSizer40 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer41;
	bSizer41 = new wxBoxSizer( wxHORIZONTAL );

	bSizer41->SetMinSize( wxSize( 400,-1 ) ); 
	m_bitmap3 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("terminal.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer41->Add( m_bitmap3, 1, wxALL, 5 );

	m_staticText23 = new wxStaticText( this, wxID_ANY, wxT("Mojo Shell"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText23->Wrap( -1 );
	m_staticText23->SetFont( wxFont( 20, 70, 90, 92, false, wxEmptyString ) );

	bSizer41->Add( m_staticText23, 2, wxALL, 5 );


	bSizer40->Add( bSizer41, 0, wxEXPAND, 5 );

#ifdef _DEBUG
	String build = L"(Debug)";
#else
	String build = L"(Release)";
#endif

	m_staticText24 = new wxStaticText( this, wxID_ANY, wxT("Version 0.3 (Unicode) ") + build, wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText24->Wrap( -1 );
	bSizer40->Add( m_staticText24, 0, wxALIGN_CENTER|wxALL, 5 );

	m_staticText241 = new wxStaticText( this, wxID_ANY, wxT("이 어플리케이션은 무료로 제공되며\n상업적인 용도로 재배포 불가합니다."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText241->Wrap( -1 );
	bSizer40->Add( m_staticText241, 0, wxALIGN_CENTER|wxALL, 5 );

	m_staticText2411 = new wxStaticText( this, wxID_ANY, wxT("문의 / 제안은 홈페이지, 이메일을 이용해주세요."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2411->Wrap( -1 );
	bSizer40->Add( m_staticText2411, 0, wxALIGN_CENTER|wxALL, 5 );

	m_hyperlink1 = new wxHyperlinkCtrl( this, wxID_ANY, wxT("Mojo Shell Web Site"), wxT("http://www.google.com"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	bSizer40->Add( m_hyperlink1, 0, wxALIGN_CENTER|wxALL, 5 );


	this->SetSizer( bSizer40 );
	this->Layout();
	bSizer40->Fit( this );

	this->Centre( wxBOTH );
}

AboutDialog::~AboutDialog()
{
}

//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(StaticTextDC, wxPanel)

	EVT_PAINT(StaticTextDC::OnPaint)
	EVT_SIZE(StaticTextDC::OnSize)

END_EVENT_TABLE()

StaticTextDC::StaticTextDC(wxWindow *parent, const wxString& label, const wxPoint& pos, const wxSize& size) 
: wxPanel(parent, wxID_ANY, pos, size)
{
	m_backBuffer = NULL;
	max_width = size.GetWidth();
	SetLabel(label);
}

StaticTextDC::~StaticTextDC() 
{
	if (m_backBuffer != NULL)
		delete m_backBuffer;
}

void StaticTextDC::SetLabel(const wxString& label)
{
#if 1
	wxSize size = GetSize();
	if (m_backBuffer == NULL)
		m_backBuffer = new wxBitmap(size.x, size.y);

	wxMemoryDC memdc;
	memdc.SelectObject(*m_backBuffer);
	memdc.SetBackground(*wxBLACK_BRUSH);
	memdc.Clear();

	Canvas c(&memdc, size.x, size.y);

	String text = label;
	String resulttext;
	//this->SetLabel(text);

	wxSize ws = c.GetTextSize(text.c_str(), SHELL_FONTSIZE);
	if( ws.GetWidth() > size.x )
	{
		int i= 0;
		while(1)
		{
			wxSize s = c.GetTextSize(resulttext.c_str(), SHELL_FONTSIZE);
			if( s.GetWidth() + 140 >= size.x )
			{
				break;
			}
			int half = text.size()/2;
			resulttext = String(text,0,half-i) + L"..." + String(text,half+i, text.size()-half+i);
			//this->SetLabel(t);
			i++;
		}
	}
	else
		resulttext = text;

#endif
	m_text = resulttext;
	Refresh(false);
}

void StaticTextDC::OnSize(wxSizeEvent& event)
{
	wxSize canvassize = GetClientSize();
	wxSize windowsize = GetSize();

	if (m_backBuffer != NULL)
	{
		delete m_backBuffer;
		m_backBuffer = new wxBitmap(canvassize.x, canvassize.y);
	}
	else
	{
		m_backBuffer = new wxBitmap(canvassize.x, canvassize.y);
	}

	Refresh(true);
}

void StaticTextDC::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc( this );
	wxSize size = GetSize();

	if (m_backBuffer == NULL)
		m_backBuffer= new wxBitmap(size.x, size.y);

	wxMemoryDC memdc;
	memdc.SelectObject(*m_backBuffer);
	memdc.SetBackground(*wxBLACK_BRUSH);
	memdc.Clear();

	Canvas c(&memdc, size.x, size.y);
	c.Clear(0xFFF0F0F0);
	//c.Clear(0xFFF00000);

	//c.DrawRect(0, 0, size.x, size.y, 0xFFF0F0F0, false);
	c.Print(0, 0, m_text, 0xff101010, false, SHELL_FONTSIZE);	
	dc.Blit(0, 0, size.x, size.y, &memdc, 0, 0);
}

//////////////////////////////////////////////////////////////////////////


DragDropDialog::DragDropDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	bSizer8->SetMinSize( wxSize( 400,-1 ) ); 
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer48;
	bSizer48 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap4 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("f2.bmp"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer48->Add( m_bitmap4, 0, wxALL, 5 );

	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("작업을 선택하세요."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	m_staticText4->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer48->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	bSizer9->Add( bSizer48, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_copy = new wxButton( this, ID_DRAGDROP_COPY, wxT("복사"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_copy, 0, wxALL, 5 );
	m_copy->SetDefault();

	m_move = new wxButton( this, ID_DRAGDROP_MOVE, wxT("이동"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_move, 0, wxALL, 5 );

	m_cancel = new wxButton( this, wxID_CANCEL, wxT("취소"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_cancel, 0, wxALL, 5 );


	bSizer9->Add( bSizer10, 1, wxALIGN_CENTER, 5 );


	bSizer8->Add( bSizer9, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();
	bSizer8->Fit( this );

	this->Centre( wxBOTH );

	Connect(ID_DRAGDROP_COPY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DragDropDialog::OnCopy));
	Connect(ID_DRAGDROP_MOVE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DragDropDialog::OnMove));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DragDropDialog::OnCancel));
}

DragDropDialog::~DragDropDialog()
{
}

void DragDropDialog::OnCopy(wxCommandEvent & event)
{
	EndModal(ID_DRAGDROP_COPY);
	//Destroy();
}

void DragDropDialog::OnMove(wxCommandEvent & event)
{
	EndModal(ID_DRAGDROP_MOVE);
	//Destroy();
}

void DragDropDialog::OnCancel(wxCommandEvent & event)
{
	EndModal(wxID_CANCEL);
	//Destroy();
}
