#pragma once

/******************************************************************************
Includes
******************************************************************************/
#include <afxtempl.h>

#include "Config.h"
#include "PatcherDlg.h"
#include "HTTPClient.h"
#include "Downloader.h"
#include "FileHelper.h"
#include "Settings.h"
#include "PatcherCommand.h"
#include "Command.h"

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

class PatcherCommand : public Command
{
public:
	PatcherCommand(CPatcher *obj, bool(CPatcher:: *meth)(CPatcher::ArgList *args), CPatcher::ArgList *args)
	{
		_object = obj;
		_method = meth;
		_args = args;
	}
	~PatcherCommand()
	{
		delete _args;
	}
	bool Execute()
	{
		return (_object->*_method)(_args);
	}

	static Command* MakeCommand(CPatcher *patcher, CString cmd, CPatcher::ArgList *args)
	{
		if (cmd == "patchfile")
			return new PatcherCommand(patcher, &CPatcher::cmdPatchFile, args);
		else if (cmd == "setverif")
			return new PatcherCommand(patcher, &CPatcher::cmdSetVerIf, args);
		else if (cmd == "addfile")
			return new PatcherCommand(patcher, &CPatcher::cmdAddFile, args);
		else if (cmd == "delfile")
			return new PatcherCommand(patcher, &CPatcher::cmdDelFile, args);
		else if (cmd == "regedit")
			return new PatcherCommand(patcher, &CPatcher::cmdRegEdit, args);
		else if (cmd == "setversion")
			return new PatcherCommand(patcher, &CPatcher::cmdSetVersion, args);
		else if (cmd == "remoteversion")
			return new PatcherCommand(patcher, &CPatcher::cmdRemoteVersion, args);
		else if (cmd == "uddfile")
			return new PatcherCommand(patcher, &CPatcher::cmdUDDFile, args);
		else if (cmd == "msgbox")
			return new PatcherCommand(patcher, &CPatcher::cmdMsgBox, args);
		else if (cmd == "AddLog")
			return new PatcherCommand(patcher, &CPatcher::cmdAddLog, args);
		else if (cmd == "cmdfile")
			return new PatcherCommand(patcher, &CPatcher::cmdCmdFile, args);
		else if (cmd == "checkupdatedir")
			return new PatcherCommand(patcher, &CPatcher::cmdCheckUpdateDir, args);
		else if (cmd == "launchapp")
			return new PatcherCommand(patcher, &CPatcher::cmdLaunchApp, args);
		else
		{
			args->SetAt(L"cmd", cmd);
			return new PatcherCommand(patcher, &CPatcher::cmdUnknown, args);
		}
	}

private:
	CPatcher *_object;
	bool(CPatcher:: *_method)(CPatcher::ArgList *args);
	CPatcher::ArgList *_args;
};