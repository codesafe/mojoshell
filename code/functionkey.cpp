
#include "functionkey.h"
#include "ini.h"
#include "utility.h"
#include "configmanager.h"

FunctionKey*	FunctionKey::instance = NULL;

FunctionKey::FunctionKey()
{

}

FunctionKey::~FunctionKey()
{

}

void	FunctionKey::ChangeFunction(int key, String name, String exec, String param)
{
	String funkey = unicode::format(L"functionkey_f%d", key+1);
	ConfigManager::GetInstance()->SetFuntionKey(funkey, L"name", name);
	ConfigManager::GetInstance()->SetFuntionKey(funkey, L"exec", exec);
	ConfigManager::GetInstance()->SetFuntionKey(funkey, L"param", param);
}

void	FunctionKey::ExecFunction(int key, FILEINFO &info)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	
	String funkey = unicode::format(L"functionkey_f%d", key+1);
	String funcexec = ConfigManager::GetInstance()->GetFuntionKey(funkey, L"exec");
	String funcparam = ConfigManager::GetInstance()->GetFuntionKey(funkey, L"param");

	if( funcexec.size() > 0 )
	{
		String path = info.m_type == TYPE_DIRECTORY ? info.m_fullpath : info.m_excutepath;
		String param = ParseParameter(funcparam, path);

		String exe = funcexec + L" " + L"\"" + param + L"\"";
		utility::CreateProcess(exe);
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}
}

String  FunctionKey::ParseParameter(String param, String path)
{
#if 1
	String resultparam;
	resultparam = unicode::replaceAll(param, functioncommand[0], utility::GetDrive(path.c_str()));
	resultparam = unicode::replaceAll(resultparam, functioncommand[1], utility::GetDrive(path.c_str()) + utility::GetPath(path.c_str()));
	resultparam = unicode::replaceAll(resultparam, functioncommand[2], path);
	resultparam = unicode::replaceAll(resultparam, functioncommand[3], utility::GetFileName(path.c_str()));

	return resultparam;
#else
	String resultparam;
	std::vector<String> tokens;
	Tokenize(param, tokens, String(L"%"));

	for (size_t i=0; i<tokens.size(); i++)
	{
		//resultparam += i > 0 ? L" " : L"";
		if( functioncommand[0] == tokens[i] )
		{
			// drive
			resultparam = resultparam + utility::GetDrive(path.c_str());
		}
		else if( functioncommand[1] == tokens[i] )
		{
			// dir
			resultparam = resultparam + utility::GetDrive(path.c_str()) + utility::GetPath(path.c_str());
		}
		else if( functioncommand[2] == tokens[i] )
		{
			// fullpath
			resultparam = resultparam + path;
		}
		else if( functioncommand[3] == tokens[i] )
		{
			// filename
			resultparam = resultparam + utility::GetFileName(path.c_str());
		}
		else
		{
			resultparam = resultparam + tokens[i];
		}
	}
	return resultparam;
#endif	
}

