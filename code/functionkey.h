#ifndef _FUNCTIONKEY_
#define _FUNCTIONKEY_

#include "predef.h"
#include "stringutil.h"
#include "xml/xml.h"


#define PARAM_DRIVE			1	// C:				"$(DRIVE)"
#define PARAM_DIRECTORY		2	// C:\aaa			"$(DIR)"
#define PARAM_FULLPATH		3	// C:\aaa\test.dat	"$(FULLPATH)"
#define PARAM_FILENAME		4	// test.dat			"$(FILENAME)"

#define FUNCTION_NUM		12	// (F1 ~ F12)

class FunctionKey
{
private :
	static FunctionKey*	instance;

	FunctionKey();
	~FunctionKey();

public:
	static FunctionKey *GetInstance()
	{
		if (instance == NULL)
		{
			instance = new FunctionKey;
		}
		return instance;
	}

	void	ChangeFunction(int key, String name, String exec, String param);

	void	ExecFunction(int key, FILEINFO &info);
	String  ParseParameter(String param, String path);

private:

};


#endif