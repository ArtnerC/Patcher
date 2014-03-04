#pragma once

/******************************************************************************
Includes
******************************************************************************/
#include <afxtempl.h>

#include "Config.h"
#include "PatcherDlg.h"
#include "Downloader.h"
#include "FileHelper.h"
#include "Settings.h"
#include "PatcherCommand.h"

#include "apatch.h"

class CPatcherDlg;
class CLog;
class CDownloader;

/******************************************************************************
Libraries
******************************************************************************/
#pragma comment(lib, "apatch.lib")

/******************************************************************************
Patcher Class
******************************************************************************/
class CPatcher
{
	struct StrAB
	{
		CString czName;
		CString czA;
		CString czB;
		StrAB() { czName.Empty(); czA.Empty(); czB.Empty(); }
	};
	struct CmdList
	{
		CString command;
		CString pA;
		CString pB;
		CmdList() { command.Empty(); pA.Empty(); pB.Empty(); }
	};
public:
	CPatcher(CString root, CString updater);
	~CPatcher();

	typedef CMapStringToString ArgList;

	//Main patching function
	bool BeginProcess();
	
	void SetPatchPercentage(DWORD percent);

	//Patching
	void ParseCommand(CString line, CmdList &cmds); //Parses command line
	void ProcessCommand(CString lineCommand, bool &patchSuccess); //Process single command
	bool ProcessCommand(CString command); //Process single command without returning success
	bool ProcessFile(CString cmdFile); //Run an entire file through ProcessCommand
	
	bool ProcessFile(CFile& commandFile);
	bool GetLine(CFile& commandFile, CString& line);
	void ParseLine(CString& line, CString& command, ArgList* args);
	bool ProcessFromServer(CDownloader& downloader, CString fileName);
	
	int CmdFileLines(CString cmd); //Returns number of lines in patch command file

	//COMMANDS
	bool cmdPatchFile(ArgList *args);
	bool cmdSetVerIf(ArgList *args);
	bool cmdAddFile(ArgList *args);
	bool cmdDelFile(ArgList *args);
	bool cmdRegEdit(ArgList *args);
	bool cmdSetVersion(ArgList *args);
	bool cmdRemoteVersion(ArgList *args);
	bool cmdUDDFile(ArgList *args);
	bool cmdMsgBox(ArgList *args);
	bool cmdAddLog(ArgList *args);
	bool cmdCmdFile(ArgList *args);
	bool cmdCheckUpdateDir(ArgList *args);
	bool cmdLaunchApp(ArgList *args);
	bool cmdUnknown(ArgList *args);

	//Directory checking
	bool CheckUpdateDir(); //Patches patcher, cleans update directory

private:
	//Dialog
	CPatcherDlg *m_wnd;

	//Settings
	CSettings *m_Settings;

	//Downloader
	CDownloader *m_Downloader;

	//Patch Success
	bool m_PatchSucceeded;

	//Status vars
	unsigned long int fDone, fCmds; //Bottom status bar

	CString runState; //Check debugging state

	//File(downloads)
	CString cmdFile;

	//Directory paths
	CString rootDir; //Path to root application directory
	CString updaterDir; //Path to updater directory

	//File Lists
	CArray<CString> goodFiles; //List of up-to-date files
	CArray<CFileHelper::CFileObj> udFiles; //List of files in updater directory
};