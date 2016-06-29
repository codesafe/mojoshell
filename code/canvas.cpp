/*
		Custom Canvas
*/

#include "canvas.h"
#include "font.h"

Canvas::Canvas(wxDC * dc, int w, int h)
{
	m_DC = dc;
	m_Size[0]	= w;
	m_Size[1]	= h;
}


Canvas::~Canvas()
{
}

void	Canvas::Clear(unsigned long color)
{
	m_DC->SetBackground(wxBrush(wxColor((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF)));
	m_DC->Clear();

}

void	Canvas::Print(int x, int y, const wchar_t * text, unsigned long color, bool rightalign, int fontsize,int clipwidth, int clipheight)
{
	if (x > (rightalign == true ? m_Size[0] + 100 : m_Size[0]) || y > m_Size[1] || y + fontsize*2 < 0)
		return ;

//	y -=2;
	wxFont * f = CFont::GetInstance()->getFont(fontsize);

	m_DC->SetFont(*f);
	m_DC->SetTextForeground(wxColor((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF));

	if (rightalign == false)
	{
		if( clipwidth != -1 && clipheight != -1 )
		{
			m_DC->SetClippingRegion(x,y,clipwidth, clipheight);
		}

		m_DC->DrawText(text, x, y+2);

		m_DC->DestroyClippingRegion();
	}
	else
	{
		UINT mode = ::GetTextAlign((HDC)m_DC->GetHDC());
		::SetTextAlign((HDC)m_DC->GetHDC(), TA_RIGHT);
		m_DC->DrawText(text, x, y+2);
		::SetTextAlign((HDC)m_DC->GetHDC(), mode);
	}

}

wxSize	Canvas::GetTextSize(const wchar_t * text, int fontsize)
{
	wxFont * f = CFont::GetInstance()->getFont(fontsize);
	m_DC->SetFont(*f);
	wxSize size = m_DC->GetTextExtent(text);
	return size;

}

void	Canvas::DrawRect(int x, int y, int w, int h, unsigned long color, bool outline)
{
	if (x + w < 0 || x > m_Size[0] || y + h < 0 || y > m_Size[1])
		return ;

	if (outline == true)
	{
		m_DC->SetPen(wxPen(wxColor((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF)));
		m_DC->SetBrush(wxBrush(*wxRED, wxTRANSPARENT));
		m_DC->DrawRectangle(x, y, w, h);
	}
	else
	{
		m_DC->SetPen(wxPen(wxColor((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF, (color>>24)&0xFF)));
		m_DC->SetBrush(wxBrush(wxColor((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF, (color>>24)&0xFF)));
		m_DC->DrawRectangle(x, y, w, h);
	}
}

void	Canvas::DrawRectClip(int x, int y, int w, int h, unsigned long color, bool outline,int clipwidth, int clipheight)
{
	m_DC->SetClippingRegion(x,y,clipwidth, clipheight);

	DrawRect(x, y, w, h, color, outline);

	m_DC->DestroyClippingRegion();
}


void	Canvas::DrawLine(int x1, int y1, int x2, int y2, unsigned long color, int width)
{
	if ((x1 < 0 && x2 < 0) || (x1 > m_Size[0] && x2 > m_Size[0]) ||
		(y1 < 0 && y2 < 0) || (y1 > m_Size[1] && y2 > m_Size[1]))
		return ;

	wxPen pen(wxColor((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF));
	pen.SetWidth(width);
	m_DC->SetPen(pen);
	m_DC->DrawLine(x1, y1, x2, y2);
}


void	Canvas::DrawBitmap(int x, int y, wxBitmap &bitmap, bool usealpha)
{
	m_DC->DrawBitmap(bitmap, x, y, usealpha);
}

void	Canvas::DrawImage(int x, int y, wxBitmap &bitmap, bool balance)
{
	wxMemoryDC memDC;
	memDC.SelectObject( bitmap );

	int imagew = bitmap.GetWidth();
	int imageh = bitmap.GetHeight();

	float neww = IMAGEVIEW_X;
	float newh = IMAGEVIEW_Y;

	float newx = x;
	float newy = y;

	if( imagew > imageh )
	{
		neww = IMAGEVIEW_X - 1;
		newh = (float)imageh/(float)imagew * IMAGEVIEW_Y;
		newy = y + ((float)IMAGEVIEW_Y - newh) / 2.f;
	}
	else if( imagew < imageh )
	{
		neww = (float)imagew/(float)imageh * IMAGEVIEW_X;
		newh = IMAGEVIEW_Y - 1;
		newx = x + ((float)IMAGEVIEW_X - neww) / 2.f;
	}

	m_DC->StretchBlit(newx , newy, neww, newh, &memDC, 0, 0, imagew, imageh, wxCOPY, true);

	DrawRect(newx , newy, neww, newh, 0xff00ffff, true);

}

void	Canvas::SetClipping(int x, int y, int w, int h)
{
	m_DC->SetClippingRegion(wxPoint(x,y), wxSize(w,h));
}

void	Canvas::ClearClipping()
{
	m_DC->DestroyClippingRegion();
}

