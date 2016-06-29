#ifndef _IMAGELOADER_
#define _IMAGELOADER_

#include "predef.h"
#include "lock.h"
#include "canvas.h"
#include "formatparser.h"

class ImageLoader
{
public:
	static ImageLoader *GetInstance()
	{
		if (instance == NULL)
			instance = new ImageLoader;
		return instance;
	}

	void	Init();
	void	Close();
	void	Reset();
	void	LoadImage(String file);
	void	ImageLoading();

	int		getImageWidth();
	int		getImageHeight();
	void	renderimage(wxPoint p, wxSize size, Canvas *c);

	HANDLE	handle[2];

private :

	HANDLE	threadhandle;

	static ImageLoader*	instance;

	ImageLoader();
	~ImageLoader();

	String		loadedfile;

	_FFINFO			lastinfo;
	wxBitmap		*bitmap;
	CCriticalSection	lock;

	bool	bigImage;
	wxSize	imageSize;
};



#endif