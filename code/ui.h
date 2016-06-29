#ifndef _UI_H_
#define _UI_H_

#include "predef.h"
#include "stringutil.h"
#include "canvas.h"

#include <list>

class CuiBase
{
public :
	CuiBase() {} ;
	virtual ~CuiBase() {};
	virtual void Render(Canvas *canvas) = 0;
};

//////////////////////////////////////////////////////////////////////////

class Box : public CuiBase
{
public :
	Box(int w, int h);
	~Box();

	void Render(Canvas *canvas);
	void SetText(const wchar *text);

protected :

	int		x,y;
	int		width;
	int		heght;

	String	str;
};



//////////////////////////////////////////////////////////////////////////


class UI
{
public:
	UI();
	~UI();

	void Render(Canvas *canvas);
	void AddUI(CuiBase *ui);

private:
	std::list<CuiBase *>	uilist;
};

#endif