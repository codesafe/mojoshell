#include "ui.h"


Box::Box(int w, int h)
{

}

Box::~Box()
{

}

void Box::Render(Canvas *canvas)
{
	int width = 400;
	int height = 100;

	int x = (canvas->GetWidth()/2) - (width/2);
	int y = (canvas->GetHeight()/2) - (height/2);

	canvas->DrawRect(x, y, width, height, 0xffffffff, false);
	canvas->Print(x + 10, y + 10, str.c_str(), 0xffffff00, false, 10);
}

void Box::SetText(const wchar *text)
{
	str = text;
}

//////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////


UI::UI()
{

}

UI::~UI()
{

}

void UI::Render(Canvas *canvas)
{
/*
	int width = 400;
	int height = 100;

	int x = (clientsize.x/2) - (width/2);
	int y = (clientsize.y/2) - (height/2);

	canvas->DrawRect(x, y, width, height, 0xffffffff, false);
*/

	std::list<CuiBase *>::iterator it = uilist.begin();
	for( ; it != uilist.end(); it++ )
	{
		(*it)->Render(canvas);
	}
}

void UI::AddUI(CuiBase *ui)
{
	uilist.push_back(ui);
}