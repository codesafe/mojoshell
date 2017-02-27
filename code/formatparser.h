#ifndef _FORMAT_PARSER_
#define _FORMAT_PARSER_

#include "predef.h"
#include "stringutil.h"
#include "canvas.h"

// FAST Format Format parser FILEID
#define FF_ERROR		0x0000
#define FF_UNKNOWN		0x0001

#define FF_JPG			0x1000
#define FF_PNG			0x1001
#define FF_BMP			0x1002
#define FF_GIF			0x1003
#define FF_PCX			0x1004
#define FF_TGA			0x1005

struct _FFINFO 
{
	int fileid;
	int width;
	int height;
	_FFINFO()
	{
		width = 0;
		height = 0;
	}
};

class FFPasrer
{
public :
	static FFPasrer * getinstance() 
	{
		if( instance == NULL )
			instance = new FFPasrer();
		return instance;
	}

	_FFINFO parseFile(const wchar *finename);

private:

	int analizeformat(unsigned char *buf, int &width, int &height);

	FFPasrer(){};
	~FFPasrer(){};

	static FFPasrer *	instance;
};



#endif