#ifndef _FTP_
#define _FTP_

#include "predef.h"

class Ftp
{
private :
	static Ftp*	instance;

	Ftp();
	~Ftp();

	wxFTP	ftp;

public:
	static Ftp *GetInstance()
	{
		if (instance == NULL)
			instance = new Ftp;
		return instance;
	}


	bool	Open(String address, String id, String pw);
	void	Close();


	void	ScanFtpFileinfo(std::vector<FILEINFO> &directoryinfolist, std::vector<FILEINFO> &fileinfolist);
};




#endif

