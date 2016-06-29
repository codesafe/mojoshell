#ifndef FONT_H
#define FONT_H

#include <map>
#include "predef.h"


class CFont
{
private :
	static CFont*	instance;

	CFont();
	~CFont();

public:
	static CFont *GetInstance()
	{
		if (instance == NULL)
		{
			instance = new CFont;
		}
		return instance;
	}

	wxFont * getFont(int fontsize);
	void	setFont(String _fontname);
	void	clearFont();

private:

	String	fontname;				// 폰트이름
	std::map <int, wxFont*>	fonts;

};




#endif
