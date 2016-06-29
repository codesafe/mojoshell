#ifndef _CANVAS_H_
#define _CANVAS_H_

#include "predef.h"


class Canvas
{
public :
	Canvas(wxDC * dc, int w, int h);
	~Canvas();

	void	Clear(unsigned long color);
	wxSize	GetTextSize(const wchar_t * text, int fontsize);
	void	Print(int x, int y, const wchar_t * text, unsigned long color, bool rightalign, int fontsize, int clipwidth = -1, int clipheight = -1);

	void	DrawRect(int x, int y, int w, int h, unsigned long color, bool outline);
	void	DrawRectClip(int x, int y, int w, int h, unsigned long color, bool outline,int clipwidth, int clipheight);
	void	DrawLine(int x1, int y1, int x2, int y2, unsigned long color, int width);
	void	DrawBitmap(int x, int y, wxBitmap &bitmap, bool usealpha = false);
	void	DrawImage(int x, int y, wxBitmap &bitmap, bool balance = false);
	void	SetClipping(int x, int y, int w, int h);
	void	ClearClipping();

	int		GetWidth() { return m_Size[0]; }
	int		GetHeight() { return m_Size[1]; }

protected :
	int		m_Size[2];
	wxDC*	m_DC;
};


#endif