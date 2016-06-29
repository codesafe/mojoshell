#include "font.h"
#include "configmanager.h"

CFont*	CFont::instance = NULL;

CFont::CFont()
{
	fontname = L"";
	String _fontname = ConfigManager::GetInstance()->GetConfigValue(L"font");
	fontname = _fontname;
}

CFont::~CFont()
{
	clearFont();
}

void CFont::setFont(String _fontname)
{
	if( fontname != L"" )
		clearFont();

	ConfigManager::GetInstance()->SetConfig(L"font", _fontname);
	ConfigManager::GetInstance()->Save();
	fontname = _fontname;
}


wxFont *CFont::getFont(int fontsize)
{
	wxFont * f = fonts[fontsize];
	if (f == NULL)
	{
		if( fontname == L"" )
			fonts[fontsize] = f = new wxFont(fontsize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false);
		else
			fonts[fontsize] = f = new wxFont(fontsize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, fontname.c_str());
	}
	return f;
}


void	CFont::clearFont()
{
	std::map <int, wxFont*>::iterator it;
	for(it=fonts.begin(); it!=fonts.end(); it++)
		delete (*it).second;

	fonts.clear();
}