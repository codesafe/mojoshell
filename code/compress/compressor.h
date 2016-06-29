#ifndef COMPRESS_H
#define COMPRESS_H

#include "./ark5/ArkLib.h"
#include "../customdialog.h"


class ComperssEvent : public IArkEvent
{
public :
	ComperssEvent();

	ARKMETHOD(void)			OnOpening(const SArkFileItem* pFileItem, float progress, BOOL& bStop);
	ARKMETHOD(void)			OnStartFile(const SArkFileItem* pFileItem, BOOL& bStopCurrent, BOOL& bStopAll, int index);	
	ARKMETHOD(void)			OnProgressFile(const SArkProgressInfo* pProgressInfo, BOOL& bStopCurrent, BOOL& bStopAll);
	ARKMETHOD(void)			OnCompleteFile(const SArkProgressInfo* pProgressInfo, ARKERR nErr);

	ARKMETHOD(void)			OnError(ARKERR nErr, const SArkFileItem* pFileItem, BOOL bIsWarning, BOOL& bStopAll){}
	ARKMETHOD(void)			OnMultiVolumeFileChanged(LPCWSTR szPathFileName){}
	ARKMETHOD(void)			OnAskOverwrite(const SArkFileItem* pFileItem, LPCWSTR szLocalPathName, ARK_OVERWRITE_MODE& overwrite, WCHAR pathName2Rename[ARK_MAX_PATH]){}
	ARKMETHOD(void)			OnAskPassword(const SArkFileItem* pFileItem, ARK_PASSWORD_ASKTYPE askType, ARK_PASSWORD_RET& ret, WCHAR passwordW[ARK_MAX_PASS]) {}

	void AddInfo(String name, unsigned long len)
	{
		filelengthinfo.insert(std::make_pair(name, len));
	}

	CompressProgress *progressdlg;

private :
	unsigned long currentlength;
	std::map<String, unsigned long>	filelengthinfo;
};

class DeComperssEvent : public IArkEvent
{
public :
	DeComperssEvent();

	ARKMETHOD(void)			OnOpening(const SArkFileItem* pFileItem, float progress, BOOL& bStop);
	ARKMETHOD(void)			OnStartFile(const SArkFileItem* pFileItem, BOOL& bStopCurrent, BOOL& bStopAll, int index);	
	ARKMETHOD(void)			OnProgressFile(const SArkProgressInfo* pProgressInfo, BOOL& bStopCurrent, BOOL& bStopAll);
	ARKMETHOD(void)			OnCompleteFile(const SArkProgressInfo* pProgressInfo, ARKERR nErr);

	ARKMETHOD(void)			OnError(ARKERR nErr, const SArkFileItem* pFileItem, BOOL bIsWarning, BOOL& bStopAll){}
	ARKMETHOD(void)			OnMultiVolumeFileChanged(LPCWSTR szPathFileName){}
	ARKMETHOD(void)			OnAskOverwrite(const SArkFileItem* pFileItem, LPCWSTR szLocalPathName, ARK_OVERWRITE_MODE& overwrite, WCHAR pathName2Rename[ARK_MAX_PATH]){}
	ARKMETHOD(void)			OnAskPassword(const SArkFileItem* pFileItem, ARK_PASSWORD_ASKTYPE askType, ARK_PASSWORD_RET& ret, WCHAR passwordW[ARK_MAX_PASS]) {}

	UnCompressProgress *progressdlg;
	int					totalcount;

private :
	unsigned long currentlength;
	int				currentcount;
};


class Compressor
{
public:
	Compressor();
	~Compressor();

	bool Init();
	void UnInit();

	void BegineCompressFile(String outpath, String passwd, int method, int archivetype);
	void EndCompressFile();

	void AddCompressFile(String scrpath, String outpath);
	void DoCompress(CompressProgress *progressdialog);

	void BegineDeCompressFile(String path);
	bool IsEncrypt();
	void DoDeCompress(UnCompressProgress *progressdialog, String path, String password);

private:
	CArkLib				arklib;
	IArkCompressor*		arkcomp;
	ComperssEvent		compressevt;
	DeComperssEvent		decomperessvt;
	SArkCompressorOpt	opt;

	String	outputpath;
};



#endif