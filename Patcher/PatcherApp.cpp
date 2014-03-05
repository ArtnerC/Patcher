/******************************************************************************
PatcherApp.cpp : Defines the class behaviors for the application.
******************************************************************************/
#include "stdafx.h"
#include "PatcherApp.h"

/******************************************************************************
CPatcherApp Message map
******************************************************************************/
BEGIN_MESSAGE_MAP(CPatcherApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/******************************************************************************
CPatcherApp construction
******************************************************************************/
CPatcherApp::CPatcherApp()
{
}

/******************************************************************************
CPatcherApp deletion
******************************************************************************/
CPatcherApp::~CPatcherApp()
{
}

/******************************************************************************
The one and only CPatcherApp object
******************************************************************************/
CPatcherApp theApp;

/******************************************************************************
CPatcherApp initialization
******************************************************************************/
BOOL CPatcherApp::InitInstance()
{
	//Needed for Browser control to function
	AfxEnableControlContainer();

	//Initialize the application
	CWinApp::InitInstance();

	//Setup app and updater paths
	ParsePaths();

	//Create the dialog
	m_wnd = new CPatcherDlg(CConfig::APP_TITLE);
	m_wnd->Create(IDD_PATCHER_DIALOG);
	m_wnd->ShowWindow(true);
	m_pMainWnd = m_wnd;

	m_wnd->SetBrowser(updaterDir + CConfig::HTML_NAME);

	m_Server = new CDownloader(m_wnd, CConfig::MAIN_URL, CConfig::PATCH_DIR);

	//Create CPatcher objects
	m_Patcher = new CPatcher(rootDir, updaterDir);

	//Create patching thread
	pThread = AfxBeginThread((AFX_THREADPROC)PatcherProc, this);

	//FALSE = Application Ends
	//TRUE = Doesn't end...
	return TRUE;
}

/******************************************************************************
Begin Patching process
******************************************************************************/
bool CPatcherApp::Process()
{
	//Auto launches only if nothing was changed.

	if(ServerConnect())
	{
		CCommandLineInfo cli;
		ParseCommandLine(cli);

		//If patching didn't occur
		if(m_Patcher->BeginProcess() && (cli.m_nShellCommand != CCommandLineInfo::FilePrint))
			LaunchApp();
	}

	//Enable launch button
	m_pMainWnd->GetDlgItem(IDC_BUTTON_LAUNCH)->EnableWindow(TRUE);

	return true;
}

void CPatcherApp::ParsePaths()
{
	//Gets exe location and sets paths
	wchar_t exeFileName[16385];
	GetModuleFileName(NULL, exeFileName, sizeof(exeFileName));
	CString exePath;
	exePath.Format(L"%s", exeFileName);

	//Parse updater directory (/Company/App Folder/updater/)
	updaterDir = CFileHelper::GetParentDir(exePath);

	//Parse root directory (/Company/App Folder/)
	rootDir = CFileHelper::GetParentDir(updaterDir);

	//Sets current directory
	SetCurrentDirectory(updaterDir);
}

bool CPatcherApp::ServerConnect()
{
	CString cmdFile;

	//Connects and downloads/processes udata
	m_wnd->SetAction(L"Connecting to Server...");
	if(!m_Server->Connect())
	{
		//Main server connect failed, trying backup server
		m_wnd->SetAction(L"Connection failed! Contacting Backup...");
		CDownloader backupDownloader(m_wnd, CConfig::BACKUP_URL, CConfig::PATCH_DIR);

		//Attempts connecting to backup server
		if(!backupDownloader.Connect())
		{
			m_wnd->SetAction(L"Connect Failed! Check internet/firewall Settings.");
			return false;
		}

		//Downloads and runs backup server command file
		if(!m_Patcher->ProcessFromServer(backupDownloader, L"pdata.txt"))
		{
			m_wnd->SetAction(L"Download Failed! " + CConfig::HELP_STRING);
			return false;
		}
		//if(backupDownloader.DownloadToMem(L"pdata.txt", cmdFile) || !m_Patcher->ProcessFile(cmdFile))
		//{
		//	m_wnd->SetAction(L"Download Failed! " + CConfig::HELP_STRING);
		//	return false;
		//}

		//Disconnect from backup server
		backupDownloader.Disconnect();

		//Attempts re-connect to main server
		if(!m_Server->Connect())
		{
			m_wnd->SetAction(L"Re-connect Failed! " + CConfig::HELP_STRING);
			return false;
		}
	}

	//Downloads version info file and runs it
	m_wnd->SetAction(L"Downloading version info..");
	if(!m_Patcher->ProcessFromServer(*m_Server, L"pdata.txt"))
	//if(!m_Server->DownloadToMem(L"pdata.txt", cmdFile) || !m_Patcher->ProcessFile(cmdFile))
	{
		m_wnd->SetAction(L"Connect Failed! Check internet/firewall Settings.");
		return false;
	}

	return true;
}

/******************************************************************************
Launch patched application
******************************************************************************/
void CPatcherApp::LaunchApp()
{
	//Get command line
	CCommandLineInfo cli;
	ParseCommandLine(cli);

	CString launchFilePath;
	launchFilePath.Format(L"%s", cli.m_strFileName);

	//Exits the updater and launches application
	if(CFileHelper::Exists(launchFilePath))
	{
		CString fileTarget, appPath;

        //Parse complete lanuch file target path (/Company/LAUNCH_FILE)
		fileTarget.Format(L"%s%s", CFileHelper::GetParentDir(rootDir), CConfig::LAUNCH_FILE);

		//Setup target application path (/Company/App Folder/Application.exe)
		appPath.Format(L"%s%s", rootDir, CConfig::APP_NAME);
		
		//Delete old launch file if it exists
		CFileHelper::Delete(fileTarget);

		//Move launch file to correct location
		CFileHelper::SetFileNormal(launchFilePath);
		CFileHelper::Move(fileTarget, launchFilePath);

		STARTUPINFO sinfo;
		PROCESS_INFORMATION pinfo;
		memset(&sinfo, 0, sizeof(sinfo));
		CreateProcess(appPath, 0, 0, 0, 0, 0, 0, rootDir, &sinfo, &pinfo);
	}
	else
	{
		CString dispErr;
		dispErr.Format(L"Browser generated file not found! You must launch from the %s website OR you may need to clear your browsers cache and try again.", CConfig::PRODUCT_NAME);
		m_pMainWnd->MessageBox(dispErr, L"Launch Error!");
	}

	//Exit the patcher
	ClosePatcher();
}

/******************************************************************************
Close Patcher
******************************************************************************/
void CPatcherApp::ClosePatcher()
{
	//Exit the patcher
	PostMessage(AfxGetMainWnd()->m_hWnd, WM_QUIT, 0, 0);
}

/******************************************************************************
Launches the patching thread
******************************************************************************/
UINT PatcherProc(CPatcherApp *cpatcherapp)
{
	cpatcherapp->Process();
	return true;
}

/******************************************************************************
Exit Patcher Instance
******************************************************************************/
int CPatcherApp::ExitInstance()
{
	delete m_wnd;
	delete m_Patcher;
	delete m_Server;
	CLog::Dispose();

	return CWinApp::ExitInstance();
}
