// UpdateCheck.cpp : Defines the entry point for the console application.
#include "stdafx.h"
#include "UpdateCheck.h"

// The one and only application object
CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		nRetCode = 1;
	}
	else
	{
		//Wait for ~10 seconds or until patcher is closed
		for(int i = 0; i < 1000; i++)
		{
			if(FindWindow(NULL, CConfig::APP_TITLE) == NULL)
			{
				//Run UpdateCheck code
				Process();
				break;
			}
			Sleep(10);
		}
	}

	return nRetCode;
}

void Process()
{
	//Gets exe location and sets variables
	wchar_t exeFileName[16385];
	GetModuleFileName(NULL, exeFileName, sizeof(exeFileName));
	CString exePath, updaterDir;

	exePath.Format(L"%s", exeFileName);
	updaterDir = CFileHelper::GetParentDir(exePath);
	
	SetCurrentDirectory(updaterDir);

    //CLog cLog(updaterDir);
	
	CArray<CFileHelper::CFileObj> fileList;
	CFileHelper::DirList(fileList, updaterDir);

	//Process files with .temp or .tmp extensions
	for(int i = 0; i < fileList.GetSize(); i++)
	{
		CFileHelper::CFileObj temp = fileList.GetAt(i);

		if(!temp.FileExt.CompareNoCase(L"temp") || !temp.FileExt.CompareNoCase(L"tmp"))
		{
			CString fTemp = temp.RootDir;
			CFileHelper::AddSlash(fTemp);
			fTemp.Append(temp.FileTitle);
			
			CFileHelper::Delete(fTemp);
			CFileHelper::Move(fTemp, temp.FilePath);
		}
	}

	//Get command line
	CCommandLineInfo cli;
	theApp.ParseCommandLine(cli);

	CString cmd;
	cmd.Format(L"%s /p \"%s\"", CConfig::PRIMARY_UPDATER, cli.m_strFileName);

	int nLen = cmd.GetLength();
	LPTSTR lpszBuf = cmd.GetBuffer(nLen);

	//Restart Updater
	STARTUPINFO sinfo = {0};
	PROCESS_INFORMATION pinfo = {0};
	sinfo.cb = sizeof(sinfo);
	CreateProcess(CConfig::PRIMARY_UPDATER, lpszBuf, 0, 0, 0, 0, 0, 0, &sinfo, &pinfo);

	cmd.ReleaseBuffer();

	return;
}