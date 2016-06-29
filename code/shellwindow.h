#ifndef _SHELL_WINDOW_
#define _SHELL_WINDOW_

#include "predef.h"
#include "canvas.h"

class Shellwindow : public wxPanel
{
public :
	enum
	{
		_LEFT_WINDOW,
		_RIGHT_WINDOW,
	};

public :
	Shellwindow(wxFrame* parent);
	~Shellwindow();

	void init(int type);
	void paintEvent(wxPaintEvent & evt);
	void render(wxDC& dc);

	void	OnMove(wxMoveEvent& event);
	void	OnSize(wxSizeEvent& event);

private :

	DECLARE_EVENT_TABLE()

	int		windowtype;
	bool	show;
};


#endif