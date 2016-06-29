
#include <windows.h>	

#include "../stringutil.h"
#include "../utility.h"
#include "compressor.h"

ComperssEvent::ComperssEvent()
{
	progressdlg = NULL;
}

void ComperssEvent::OnOpening(const SArkFileItem* pFileItem, float progress, BOOL& bStop)
{
}

void ComperssEvent::OnStartFile(const SArkFileItem* pFileItem, BOOL& bStopCurrent, BOOL& bStopAll, int index)
{
	progressdlg->Update(L"currentfile", pFileItem->fileNameW);
	currentlength = 0;

	std::map<String, unsigned long>::iterator it = filelengthinfo.find(String(pFileItem->fileNameW));
	if( it != filelengthinfo.end() )
	{
		currentlength = it->second;
	}
}

void ComperssEvent::OnProgressFile(const SArkProgressInfo* pProgressInfo, BOOL& bStopCurrent, BOOL& bStopAll)
{
	//progressdialog->Update(L"filecount", unicode::format(L"(%d/%d)", cfilenum+1, totalfilenum+1));

	int progress = (int)pProgressInfo->fCurPercent;
	progressdlg->Update(L"zipprogress", unicode::format(L"%d", progress));

	int fileprogress = (int)pProgressInfo->fTotPercent;
	progressdlg->Update(L"totalprogress", unicode::format(L"%d", fileprogress));

	int clen = (int)((float)currentlength * (pProgressInfo->fCurPercent/100.f));
 	progressdlg->Update(L"progresssize", unicode::format(L"(%d/%d)", clen, currentlength));

	if(progressdlg->iscancel)
	{
		wxMessageDialog dial(NULL, L"압축을 취소 합니까?", L"압축 취소", wxCENTER | wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
		int ret = dial.ShowModal();
		if( ret == wxID_YES )
		{
			bStopAll = TRUE;
			return;
		}
		else
			progressdlg->iscancel = false;
	}
}

void ComperssEvent::OnCompleteFile(const SArkProgressInfo* pProgressInfo, ARKERR nErr)
{
}

//////////////////////////////////////////////////////////////////////////


DeComperssEvent::DeComperssEvent()
{
	progressdlg = NULL;
	currentcount = 0;
}

void DeComperssEvent::OnOpening(const SArkFileItem* pFileItem, float progress, BOOL& bStop)
{
}

void DeComperssEvent::OnStartFile(const SArkFileItem* pFileItem, BOOL& bStopCurrent, BOOL& bStopAll, int index)
{
	progressdlg->Update(L"currentfile", pFileItem->fileNameW);
	currentlength = pFileItem->uncompressedSize;
	currentcount++;

	progressdlg->Update(L"filecount", unicode::format(L"(%d/%d)", currentcount, totalcount));
}

void DeComperssEvent::OnProgressFile(const SArkProgressInfo* pProgressInfo, BOOL& bStopCurrent, BOOL& bStopAll)
{
	int progress = (int)pProgressInfo->fCurPercent;
	progressdlg->Update(L"zipprogress", unicode::format(L"%d", progress));

	int fileprogress = (int)pProgressInfo->fTotPercent;
	progressdlg->Update(L"totalprogress", unicode::format(L"%d", fileprogress));

	int clen = (int)((float)currentlength * (pProgressInfo->fCurPercent/100.f));
	progressdlg->Update(L"progresssize", unicode::format(L"(%d/%d)", clen, currentlength));

	if(progressdlg->iscancel)
	{
		wxMessageDialog dial(NULL, L"압축 풀기를 취소 합니까?", L"압축 풀기 취소", wxCENTER | wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
		int ret = dial.ShowModal();
		if( ret == wxID_YES )
		{
			bStopAll = TRUE;
			return;
		}
		else
			progressdlg->iscancel = false;
	}

}

void DeComperssEvent::OnCompleteFile(const SArkProgressInfo* pProgressInfo, ARKERR nErr)
{

}



//////////////////////////////////////////////////////////////////////////

Compressor::Compressor()
{
}

Compressor::~Compressor()
{
}

bool Compressor::Init()
{
	if(arklib.Create(ARK_DLL_RELEASE_FILE_NAME)!=ARKERR_NOERR)
		return false;
	
	return true;
}


void Compressor::UnInit()
{
	arklib.Close();
}

void Compressor::BegineCompressFile(String outpath, String passwd, int method, int archivetype)
{
	outputpath = outpath;
	int passlen = 0;
	char password[128] = { 0, };

	if (!passwd.empty())
	{
		unicode::convstr_s(password, 128, passwd.c_str());
		passlen = strlen(password);
	}

	opt.deleteArchiveWhenFailed = TRUE;
	opt.compressionLevel = -1;
	opt.lzmaEncodeThreadCount = 1;

	arkcomp = arklib.CreateCompressor();
	arkcomp->Init();
	arkcomp->SetEvent(&compressevt);

	switch (archivetype)
	{
		case ARCHIVE_ZIP :
			{
				opt.ff = ARK_FF_ZIP;
				opt.compressionMethod = ARK_COMPRESSION_METHOD_DEFLATE;

				if(method == 0)
					opt.compressionMethod = ARK_COMPRESSION_METHOD_STORE;
				else if(method == 1)
					opt.compressionLevel = 1;
				else if(method == 2)
					opt.compressionLevel = -1;
				else if(method == 3)
					opt.compressionLevel = 9;

				opt.encryptionMethod = ARK_ENCRYPTION_METHOD_ZIP;
				arkcomp->SetOption(opt, (const ARKBYTE*)password, passlen);
			}
			break;

		case ARCHIVE_7Z :
			{
				opt.ff = ARK_FF_7Z;
				opt.compressionMethod = ARK_COMPRESSION_METHOD_LZMA;

				if(method == 0)
					opt.compressionMethod = ARK_COMPRESSION_METHOD_STORE;
				else if(method == 1)
					opt.compressionLevel = 1;
				else if(method == 2)
					opt.compressionLevel = -1;
				else if(method == 3)
					opt.compressionLevel = 9;

				if( passwd.size() > 0 )
				{
					opt.encryptionMethod = ARK_ENCRYPTION_METHOD_AES256;
					arkcomp->SetOption(opt, (const ARKBYTE*)passwd.c_str(), passwd.size() * sizeof(wchar));
				}
				else
					arkcomp->SetOption(opt, NULL, 0);
			}
			break;

		case ARCHIVE_LZH :
				opt.ff = ARK_FF_LZH;
				opt.compressionMethod = ARK_COMPRESSION_METHOD_LH7;
			break;
	}


}

void Compressor::DoCompress(CompressProgress *progressdialog)
{
	// 실제 압축 수행
	compressevt.progressdlg = progressdialog;
	arkcomp->CreateArchive(outputpath.c_str(), NULL);
}

void Compressor::EndCompressFile()
{
	arkcomp->Release();
	outputpath.clear();
}

void Compressor::AddCompressFile(String scrpath, String outpath)
{
	// 압축할 파일 추가
	unsigned long len = utility::GetFileLength(scrpath.c_str());
	String temppath = outpath;
	std::replace(temppath.begin(), temppath.end(), '\\', '/');
	compressevt.AddInfo(temppath, len);

	int ret = arkcomp->AddFileItem(scrpath.c_str(), outpath.c_str(), FALSE);
}


void Compressor::BegineDeCompressFile(String path)
{
	if(arklib.Open(path.c_str(), NULL) == FALSE)
	{
		arklib.Close();
		return;
	}
}

bool Compressor::IsEncrypt()
{
	if(arklib.IsEncryptedArchive())
		return true;

	return false;
}

void Compressor::DoDeCompress(UnCompressProgress *progressdialog, String path, String password)
{
	arklib.SetEvent(&decomperessvt);
	decomperessvt.progressdlg = progressdialog;

	if( !password.empty() )
		arklib.SetPassword(password.c_str());

	decomperessvt.totalcount = arklib.GetFileItemCount();

	if(arklib.ExtractAllTo(path.c_str())==TRUE)
	{
		// Error!!
	}
}