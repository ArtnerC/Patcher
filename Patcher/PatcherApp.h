#pragma once

/******************************************************************************
CPatcherApp Includes
******************************************************************************/
#include "Config.h"
#include "Patcher.h"
#include "PatcherDlg.h"
#include "Downloader.h"
#include "FileHelper.h"
#include "Log.h"

class CPatcher;
class CPatcherDlg;

/******************************************************************************
CPatcherApp
******************************************************************************/
class CPatcherApp : public CWinApp
{
public:
	CPatcherApp();
	~CPatcherApp();

	void ParsePaths();
	bool ServerConnect();
	void LaunchApp();
	void ClosePatcher();

	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()

protected:
	bool Process();
	friend UINT PatcherProc(CPatcherApp *cpatcherapp);

private:
	//Patcher Thread
	CWinThread *pThread;
	
	//Patcher Class
	CPatcher *m_Patcher;

	//Dialog
	CPatcherDlg *m_wnd;

	//HTTP
	CDownloader *m_Server;

	//Log Class
	CLog *m_Log;

	//Directory paths
	CString rootDir; //Path to root application directory
	CString updaterDir; //Path to updater directory

public:
	virtual int ExitInstance();
};

/******************************************************************************
Globals
******************************************************************************/
extern CPatcherApp theApp;