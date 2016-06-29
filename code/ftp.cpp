#include "ftp.h"
#include "ftpparse.h"
#include "stringutil.h"
#include "utility.h"


// ftp result parse
// http://cr.yp.to/ftpparse/ftpparse.c

Ftp* Ftp::instance = NULL;


Ftp::Ftp()
{
	
}

Ftp::~Ftp()
{

}

bool Ftp::Open(String address, String id, String pw)
{
	ftp.SetUser(id.c_str());
	ftp.SetPassword(pw.c_str());
	ftp.SetPassive(false);


	if ( !ftp.Connect(address.c_str()) )
	{
		wxLogError("Couldn't connect");
		return false;
	}

	wxArrayString files;
	bool ret = ftp.GetDirList(files);

	for(int i=0; i<files.Count(); i++)
	{
		String s;
		s = files[i];

		char info[4096];
		unicode::convstr_s(info, 4096, s.c_str());
		struct ftpparse ftpinfo;
		int r = ftpparse(&ftpinfo, info, strlen(info));
	}
/*
	wxArrayString files2;
	 ret = ftp.GetFilesList(files2);

	 for(int i=0; i<files2.Count(); i++)
	 {
		 String s;
		 s = files[i];
	 }
*/

	return true;
}

void	Ftp::Close()
{
	ftp.Close();
}

void Ftp::ScanFtpFileinfo(std::vector<FILEINFO> &directoryinfolist, std::vector<FILEINFO> &fileinfolist)
{
	wxArrayString files;
	bool ret = ftp.GetDirList(files);

	for(int i=0; i<files.Count(); i++)
	{
		String s;
		s = files[i];

		char info[4096];
		unicode::convstr_s(info, 4096, s.c_str());
		struct ftpparse ftpinfo;
		int r = ftpparse(&ftpinfo, info, strlen(info));
		wchar temp[4096];

		if (ftpinfo.flagtrycwd == 1) 
		{
			FILEINFO info;
			info.m_type = TYPE_DIRECTORY;

			unicode::convstr_s(temp, 4096, ftpinfo.name);
			info.m_name = temp;
			info.m_ext = _T("[폴더]");

			info.m_attribute = 0;
			info.m_attribute |= 0;
			info.m_attribute |= 0;
			info.m_attribute |= 0;

/*
			int len = unicode::strlen(path);
			if(path[len-1] == '\\')
				info.m_fullpath = String(path) + wfd.cFileName;
			else
				info.m_fullpath = String(path) + String(_T("\\")) + wfd.cFileName;*/

			utility::TimetToFileTime(ftpinfo.mtime, &info.m_creationtime);
			utility::TimetToFileTime(ftpinfo.mtime, &info.m_LastAccessTime);
			utility::TimetToFileTime(ftpinfo.mtime, &info.m_LastWriteTime);

			directoryinfolist.push_back(info);
		} 
		else 
		{
			String name, ext;

			FILEINFO info;
			info.m_type = TYPE_FILE;
			info.m_attribute = 0;
			info.m_attribute |= 0;
			info.m_attribute |= 0;
			info.m_attribute |= 0;

			unicode::convstr_s(temp, 4096, ftpinfo.name);
			info.m_name = temp;

			name = utility::GetName(temp);
			ext = utility::GetExtention(temp);

			ext.erase(ext.begin());
//			info.m_fullpath = String(path) + String(_T("\\")) + info.m_name;
//			info.m_excutepath = String(path) + String(_T("\\")) + wfd.cFileName;

			info.m_name = name;
			info.m_ext = ext;

			utility::TimetToFileTime(ftpinfo.mtime, &info.m_creationtime);
			utility::TimetToFileTime(ftpinfo.mtime, &info.m_LastAccessTime);
			utility::TimetToFileTime(ftpinfo.mtime, &info.m_LastWriteTime);

			info.m_filesize = ftpinfo.size;

			fileinfolist.push_back(info);
		}

	}




}
