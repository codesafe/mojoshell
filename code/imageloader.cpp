
#include "predef.h"
#include "imageloader.h"
#include "stringutil.h"
#include "utility.h"

#include <process.h>

ImageLoader*	ImageLoader::instance = NULL;

ImageLoader::ImageLoader()
{
	bitmap = NULL;
	bigImage = false;
}

ImageLoader::~ImageLoader()
{

}

unsigned int  WINAPI threadfunction(void* data)
{
	DWORD dwWaitStatus;

	while (true)
	{
		// Stop Thread!
		dwWaitStatus = WaitForMultipleObjects(2, ImageLoader::GetInstance()->handle, FALSE, INFINITE); 

		switch (dwWaitStatus) 
		{ 
		case WAIT_OBJECT_0: 
			{
				ImageLoader::GetInstance()->ImageLoading();
				ResetEvent(ImageLoader::GetInstance()->handle[0]);
			}
			break; 

		case WAIT_OBJECT_0 + 1:
			{
				// exit
				return 1;
			}
			break;
		}
	}

	return 0;
}

void	ImageLoader::ImageLoading()
{
	lock.Enter();
	if( bitmap )
	{
		delete bitmap;
		bitmap = NULL;
	}
	bigImage = false;
	lastinfo = FFPasrer::getinstance()->parseFile(loadedfile.c_str());

	if( lastinfo.width < IMAGEVIEW_MAX_X && lastinfo.width < IMAGEVIEW_MAX_Y )
	{
		// start load
		wxString fname = loadedfile;
		wxImage	image(fname, wxBITMAP_TYPE_ANY);
		//imageSize.x = image.GetWidth();
		//imageSize.y = image.GetHeight();
		bitmap = new wxBitmap(image);
		image.Destroy();
	}
	else
	{
		wxImage	image(IMAGEVIEW_X, IMAGEVIEW_Y, true);
		bitmap = new wxBitmap(image);
		bigImage = true;
		image.Destroy();
	}

	lock.Leave();
}


void	ImageLoader::renderimage(wxPoint p, wxSize size, Canvas *c)
{
	lock.Enter();

	const int yfix = 40;
	int xpos = (size.x-p.x) - (IMAGEVIEW_X + 10);

	if(bitmap)
	{
		int w = bitmap->GetWidth();

		int cl = 0;
		for (int y = 0; y < GRID_DIVIDE; y++)
			for (int x = 0; x < GRID_DIVIDE; x++)
			{
				int nw = IMAGEVIEW_X / GRID_DIVIDE;
				int nh = IMAGEVIEW_Y / GRID_DIVIDE;

				unsigned long color = (cl%2 == 0) ? 0xffa0a0a0 : 0xff808080;
				c->DrawRect(xpos + (x*nw), yfix + (y*nh), nw, nh, color, false);
				cl++;
			}

			c->DrawRect(xpos - 1, yfix - 1, IMAGEVIEW_X+1, IMAGEVIEW_Y+1, 0xff00ff00, true);
			c->DrawImage(xpos, yfix, *bitmap);
			c->DrawRect(xpos - 1, yfix + IMAGEVIEW_Y, IMAGEVIEW_X, 20, 0xff202020, false);

			if( bigImage )
			{
				String stext = unicode::Format(L"Too Big %d X %d", lastinfo.width, lastinfo.height);
				wxSize ss =	c->GetTextSize(stext.c_str(), SHELL_FONTSIZE);
				int xx = (IMAGEVIEW_X - ss.x)/2;
				c->Print(xpos + xx, yfix + IMAGEVIEW_Y + 4, stext.c_str(), 0xff00ff00, false, SHELL_FONTSIZE);
			}
			else
			{
				String stext = unicode::Format(L"%d X %d %d bit", bitmap->GetWidth(), bitmap->GetHeight(), bitmap->GetDepth());
				wxSize ss =	c->GetTextSize(stext.c_str(), SHELL_FONTSIZE);
				int xx = (IMAGEVIEW_X - ss.x)/2;
				c->Print(xpos + xx, yfix + IMAGEVIEW_Y + 4, stext.c_str(), 0xff00ff00, false, SHELL_FONTSIZE);
			}
	}	

	lock.Leave();
}

void	ImageLoader::Init()
{
	handle[0] = CreateEvent( NULL, TRUE, FALSE, NULL );
	handle[1] = CreateEvent( NULL, TRUE, FALSE, NULL );

	DWORD threadid = 0;
	threadhandle = (HANDLE)_beginthreadex(NULL, 0, threadfunction, (void*)this, 0, 0);

}

void	ImageLoader::Close()
{
	SetEvent(handle[1]);
}

void	ImageLoader::Reset()
{
	// Image off
	if (loadedfile.empty()) return;

	loadedfile.clear();
	lock.Enter();
	if( bitmap )
	{
		delete bitmap;
		bitmap = NULL;
		bigImage = false;
	}
	lock.Leave();
}

bool	ImageLoader::enableshowext(String ext)
{
	for (int i = 0; i < _countof(imageext); i++)
	{
		if (imageext[i] == ext) return true;
	}

	return false;
}

void	ImageLoader::LoadImage(String file)
{
	String ext = utility::GetExtention(file.c_str());
	unicode::Upper(ext);
	//if( ext == L".BMP" || ext == L".PNG" || ext == L".TGA" || ext == L".JPG" || ext == L".JPEG" || ext == L".PCX" || ext == L".GIF" || ext == L".XPM" || ext == L".ICO")
	if(enableshowext(ext))
	{
		loadedfile = file;
		SetEvent(handle[0]);	// LOAD
	}
	else
	{
		Reset();
	}
}


int		ImageLoader::getImageWidth()
{
	if( !loadedfile.empty() && bitmap != NULL )
	{
		return bitmap->GetWidth();
	}

	return -1;
}

int		ImageLoader::getImageHeight()
{
	if( !loadedfile.empty() && bitmap != NULL )
	{
		return bitmap->GetHeight();
	}

	return -1;
}
