#include "shellwindow.h"


BEGIN_EVENT_TABLE(Shellwindow, wxPanel)
/*
	 EVT_MOTION(BasicDrawPane::mouseMoved)
	 EVT_LEFT_DOWN(BasicDrawPane::mouseDown)
	 EVT_LEFT_UP(BasicDrawPane::mouseReleased)
	 EVT_RIGHT_DOWN(BasicDrawPane::rightClick)
	 EVT_LEAVE_WINDOW(BasicDrawPane::mouseLeftWindow)
	 EVT_KEY_DOWN(BasicDrawPane::keyPressed)
	 EVT_KEY_UP(BasicDrawPane::keyReleased)
	 EVT_MOUSEWHEEL(BasicDrawPane::mouseWheelMoved)
*/


// 	EVT_SIZE(Shellwindow::OnSize)
// 	EVT_MOVE(Shellwindow::OnMove)
 	EVT_PAINT(Shellwindow::paintEvent) 

END_EVENT_TABLE()

Shellwindow::Shellwindow(wxFrame* parent)
: wxPanel(parent, -1, wxPoint(-1, -1), wxSize(-1, -1), wxBORDER_SIMPLE)
{

}

Shellwindow::~Shellwindow()
{

}

void Shellwindow::init(int type)
{
	windowtype = type;
	show = true;
}

void	Shellwindow::OnMove(wxMoveEvent& event)
{
	_ASSERT(0);
}

void	Shellwindow::OnSize(wxSizeEvent& event)
{
	_ASSERT(0);
}

void Shellwindow::paintEvent(wxPaintEvent & evt)
{
 	wxPaintDC dc(this);
 	render(dc);
}

void Shellwindow::render(wxDC& dc)
{
// 	dc.SetBrush(*wxGREEN_BRUSH); // green filling
// 	dc.SetPen( wxPen( wxColor(255,0,0), 5 ) ); // 5-pixels-thick red outline
// 	dc.DrawCircle( wxPoint(200,100), 25 /* radius */ );


/*
	wxPaintDC dcc(this);
	wxSize size = GetClientSize();
	Canvas c(&dcc, size.x, size.y);
	c.Clear(0xff000000);

	c.DrawRect(100, 100, 100, 100, 0xff00ff0f, false);
	c.Print(10, 10, L"HAHAHA", 0xffffffff, false, 20);
*/


	wxSize size = GetClientSize();
	Canvas c(&dc, size.x, size.y);
	c.Clear(0xff000000);

	c.DrawRect(100, 100, 100, 100, 0xff00ff0f, false);
	c.Print(10, 10, L"HAHAHA", 0xffffffff, false, 20);

}
