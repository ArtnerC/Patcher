/******************************************************************************
Includes
******************************************************************************/
#include "stdafx.h"
#include "Patcher.h"

/******************************************************************************
CPatcher Constructor
******************************************************************************/
CPatcher::CPatcher(CString root, CString updater)
{
	//Initializes Variables
	rootDir = root;
	updaterDir = updater;

	fDone = fCmds = 0;

	//Settings
	m_Settings = new CSettings(CConfig::REGISTRY_SUBKEY);

	//Window
	//m_wnd = CPatcherDlg::GetPatcherDlg();

	//Initialize ActivePatch Library
	InitActivePatch(APATCH_LICENSE_KEY, 0);
}

/******************************************************************************
CPatcher Destructor
******************************************************************************/
CPatcher::~CPatcher()
{
}

/******************************************************************************
Begins patching process
******************************************************************************/
bool CPatcher::BeginProcess()
{
	m_wnd->SetAction("Initializing...");

	//Check for a setup file and process (before connect)
	CFile sFile;
	if(sFile.Open("setup.txt", CFile::modeRead))
	{
		Sleep(1000);
		m_wnd->SetAction("Running Setup...");
		cmdFile.Empty();
		cmdFile.Preallocate((long int)sFile.GetLength());
		char sBuffer[1024];
		int rlen = 0;
		while((rlen = sFile.Read(sBuffer, 1024)) > 0)
			cmdFile.Append(sBuffer, rlen);

		sFile.Close();
		CFileHelper::Delete("setup.txt");

		ProcessFile(cmdFile);
	}

	//Starts getting patches and processing
	m_wnd->SetAction("Patching Files...");
	bool bReturn = true;
	while((m_Settings->GetInt("LocalVersion", true) < m_Settings->GetInt("RemoteVersion")) && m_Settings->GetInt("LocalVersion", true) && m_Settings->GetInt("RemoteVersion"))
	{
		//Return false to keep patcher open
		bReturn = false;

		//Did everything succeed
		bool bSuccess = true;

		//Setup our files location
		CString patchLoc;
		patchLoc.Format("commands/%d.txt", m_Settings->GetInt("LocalVersion", true));

		//Download and run command file
		if(!m_Downloader->DownloadToMem(patchLoc, cmdFile))
			//Download default patch command file
			if(!m_Downloader->DownloadToMem("commands/default.txt", cmdFile))
				bSuccess = false;

		if(bSuccess)
			bSuccess = ProcessFile(cmdFile);			

		//Checks patch success
		if(!bSuccess)
		{
			m_wnd->SetStatus("Patching Failed!");
			CLog::Instance()->AddLog("(Process) Error with patch command file. LocalVersion: %d  RemoteVersion: %d", m_Settings->GetInt("LocalVersion", true), m_Settings->GetInt("RemoteVersion"));
			return false;
		}
	}

	//Display that patching is complete
	if(!bReturn)
		m_wnd->SetAction("Patching Finished");
	else
		m_wnd->SetAction("Ready to Launch!");
	m_wnd->SetStatus("");

	return bReturn;
}
/******************************************************************************
Callback Function for PatchApply() status
******************************************************************************/
BOOL CALLBACK PatchStatusProc(HPACKAGE hPackage, LPPATCH_STATUS_INFORMATION lpInfo, LPVOID lParam)
{
	//Passed in child window
	CPatcher* cPatcher = reinterpret_cast<CPatcher*>(lParam);
	//CPatcher* cPatcher = (CPatcher*)lParam;

	//Copy percent done to window
	cPatcher->SetPatchPercentage(lpInfo->dwFileProgress);

	return true;
}

/******************************************************************************
Public function to set percent of patch complete
******************************************************************************/
void CPatcher::SetPatchPercentage(DWORD percent)
{	
	m_wnd->SetProgress3(percent);
}

/******************************************************************************
Parse command and params from line
******************************************************************************/
void CPatcher::ParseCommand(CString lineCommand, CmdList &cmds)
{	
	//Remove Comment
	int pos = lineCommand.Find("//");
	if(pos != -1)
		lineCommand.Delete(pos, lineCommand.GetLength() - pos);

	//Get command
	cmds.command.Format("%s", lineCommand);
	pos = lineCommand.FindOneOf(" \t\r\n");
	if(pos != -1)
	{
		cmds.command.Format("%s", lineCommand.Left(pos));
		lineCommand.Delete(0, pos + 1);
	}

	//Get Param 1
	cmds.pA.Format("%s", lineCommand);
	pos = lineCommand.FindOneOf("|\t\r\n");
	if(pos != -1)
	{
		cmds.pA.Format("%s", lineCommand.Left(pos));
		lineCommand.Delete(0, pos + 1);
	}

	//Get Param 2
	cmds.pB.Format("%s", lineCommand);

	//Strip trailing and leading spaces
	cmds.command.Trim();
	cmds.pA.Trim();
	cmds.pB.Trim();

	return;
}

/******************************************************************************
Runs the command ie:from patch command file
******************************************************************************/
void CPatcher::ProcessCommand(CString lineCommand, bool &patchSuccess)
{
	//Split the command and parameters into separate vars
	CmdList cmds;
	ParseCommand(lineCommand, cmds);

	int fDown = 0;
	int fSize = 0;
	m_wnd->SetProgress1(0, 0);
	m_wnd->SetProgress3(0);

	//Processes single file patching
	//patchfile [Path of Patch on Server]|[Local Computer Path]
	if(!strcmp(cmds.command, "patchfile"))
	{
		//CString tempPatch, tempBackup;
		CString fPatch, fBackup;
		fPatch.Format("%s", cmds.pA);
		fBackup.Format("%s", cmds.pB);

		//Replace \ with /
		fBackup.Replace('\\', '/');

		//Get file name from path
		int cPos = cmds.pA.ReverseFind('/');
		if(cPos >= 0)
			fPatch.Format("%s", cmds.pA.Right(cmds.pA.GetLength() - cPos - 1));

		CString PatchDest, FilePath, BackupURL, PatchURL;
		PatchDest.Format(	"%s%s",			updaterDir,	fPatch);
		FilePath.Format(	"%s%s",			rootDir,	cmds.pB);
		BackupURL.Format(	"install/%s",	fBackup);
		PatchURL.Format(	"patches/%s",	cmds.pA);

		m_wnd->SetStatus("Patching: %s", fPatch);

		//Checks to see if the entire file has already been downloaded
		bool fileClean = false;
		for(int g = 0; g < goodFiles.GetSize(); g++)
		{
			CString tempDL;
			tempDL = goodFiles.GetAt(g);
			if(!tempDL.CompareNoCase(FilePath))
				fileClean = true;
		}

		//Download patch file to disk
		if(!fileClean && m_Downloader->DownloadToFile(PatchURL, PatchDest))
		{
			//Remove bad attributes from file(read-only, hidden, etc)
			CFileHelper::SetFileNormal(FilePath);

			//Get patch file information
			PATCH_FILE_INFORMATION patchInfo;
			patchInfo.cbStructure = sizeof(PATCH_FILE_INFORMATION);
			GetPatchFileInformation(PatchDest, NULL, PATCH_INFO_REFERENCE_FILE, &patchInfo);

			//Processes patch file
			if(!ApplyPatchFile(PatchDest, patchInfo.dwPatchOptions, 0, 0, FilePath, 0, (LPPATCHSTATUSPROC)PatchStatusProc, (LPARAM)this))
			{
				m_wnd->SetProgress3(0);

				//Add log entry with error info
				CLog::Instance()->AddLog("(ProcessCommand) ApplyPatchFile Error: 0x%x File: %s LocalVersion: %d", GetLastError(), cmds.pB, m_Settings->GetInt("LocalVersion", true));

				m_wnd->SetStatus("Repairing: %s", cmds.pB);

				//Download full file to disk if patching fails
				if(m_Downloader->DownloadToFile(BackupURL, FilePath))
				{										
					//Add File to list of full file downloads
					CString tempDL;
					tempDL.Format(FilePath);
					goodFiles.Add(tempDL);
				}
				else
					patchSuccess = false;
			}

			m_wnd->SetProgress3(0);

			//Remove patch file
			CFileHelper::Delete(PatchDest);
		}
		else if(!fileClean)
			patchSuccess = false;

		return;
	}

	//Sets version if all patching was successful
	//setverif [Version Number]
	if((!strcmp(cmds.command, "setverif")) && patchSuccess)
	{
		m_Settings->SetStr("LocalVersion", cmds.pA, true);
		m_wnd->SetVersion(m_Settings->GetInt("RemoteVersion"), m_Settings->GetInt("LocalVersion", true));
		return;
	}

	//Downloads and adds a whole new file
	//addfile [Path on Server]|[Path on Computer]
	if(!strcmp(cmds.command, "addfile"))
	{
		CString FileURL;
		FileURL.Format("patches/%s", cmds.pA);
		CString FileDest;
		FileDest.Format("%s%s", rootDir, cmds.pB);

		//Checks to see if the entire file has already been downloaded
		bool fileClean = false;
		for(int g = 0; g < goodFiles.GetSize(); g++)
		{
			CString tempDL;
			tempDL = goodFiles.GetAt(g);
			if(!tempDL.CompareNoCase(FileDest))
				fileClean = true;
		}

		//Download file to disk
		if(!fileClean && m_Downloader->DownloadToFile(FileURL, FileDest))
		{
			//Add File to list of full file downloads
			CString tempDL;
			tempDL.Format(FileDest);
			goodFiles.Add(tempDL);
		}
		else if(!fileClean)
			patchSuccess = false;

		return;
	}

	//Deletes a specified file
	//delfile [Local Path to File]
	if(!strcmp(cmds.command, "delfile"))
	{
		//Create full path
		CString FileDest;
		FileDest.Format("%s%s", rootDir, cmds.pA);

		//Delete file
		CFileHelper::Delete(FileDest);

		//Removes file from clean files list if it's had a full download
		for(int g = 0; g < goodFiles.GetSize(); g++)
		{
			CString tempDL;
			tempDL = goodFiles.GetAt(g);
			if(!tempDL.CompareNoCase(FileDest))
				goodFiles.RemoveAt(g);
		}

		return;
	}

	//Sets a registry value
	//regedit [Key]|[Value]
	if(!strcmp(cmds.command, "regedit"))
	{
		m_Settings->SetStr(cmds.pA, cmds.pB, true);
		return;
	}

	//Sets version no matter what
	//setversion [Version Number]
	if(!strcmp(cmds.command, "setversion"))
	{
		m_Settings->SetStr("LocalVersion", cmds.pA, true);
		m_wnd->SetVersion(m_Settings->GetInt("RemoteVersion"), m_Settings->GetInt("LocalVersion", true));
		return;
	}

	//Sets remote version
	//remoteversion [Version Number]
	if(!strcmp(cmds.command, "remoteversion"))
	{		
		m_Settings->SetStr("RemoteVersion", cmds.pA);
		m_wnd->SetVersion(m_Settings->GetInt("RemoteVersion"), m_Settings->GetInt("LocalVersion", true));
		return;
	}

	//Adds file/checksum to udFiles
	//uddfile [File Name]|[MD5 Checksum of File]
	if(!strcmp(cmds.command, "uddfile"))
	{
		CFileHelper::CFileObj temp;
		temp.FileName.Format("%s", cmds.pA);
		temp.Checksum.Format("%s", cmds.pB);
		temp.FilePath.Format("%s%s", updaterDir, temp.FileName);
		temp.FileExt = CFileHelper::GetExt(temp.FileName);
		temp.RootDir = updaterDir;
		udFiles.Add(temp);
		return;
	}

	//Displays a message to the user
	//msgbox [Message]
	//msgbox [Title]|[Message]
	if(!strcmp(cmds.command, "msgbox"))
	{
		if((cmds.pB.GetLength() <= 0) || (!cmds.pB.Compare(cmds.pA)))
			theApp.m_pMainWnd->MessageBox(cmds.pA, "Update Information!");
		else
			theApp.m_pMainWnd->MessageBox(cmds.pB, cmds.pA);
		return;
	}

	//Adds line to log file
	//CLog::Instance()->AddLog [Log Entry]
	if(!strcmp(cmds.command, "AddLog"))
	{
		CLog::Instance()->AddLog("%s", cmds.pA);
		return;
	}

	//Downloads and displays patch info
	//patchinfo PatchInfo.rtf
	if(!strcmp(cmds.command, "patchinfo"))
	{
		//m_wnd->SetBrowser(updaterDir + "\Notes.htm");
		return;
	}

	//Download and run patch command file
	//cmdfile commands/[file].txt
	if(!strcmp(cmds.command, "cmdfile"))
	{
		CString cmdFileB;
		if(m_Downloader->DownloadToMem(cmds.pA, cmdFileB))
			if(!ProcessFile(cmdFileB))
				patchSuccess = false;
		return;
	}

	//Check updater directory for consistency
	//checkupdatedir
	if(!strcmp(cmds.command, "checkupdatedir"))
	{
		if(m_Settings->GetStr("runState", true).CompareNoCase("NOCHECK") && (udFiles.GetSize() > 1))
			if(!CheckUpdateDir())
				patchSuccess = false;
		return;
	}

	if(!strcmp(cmds.command, "launchapp"))
	{
		theApp.LaunchApp();
	}

	CLog::Instance()->AddLog("(ProcessCommand) Un-Supported Command: \"%s\"", lineCommand);

	return;
}
bool CPatcher::ProcessCommand(CString command)
{
	bool patchSuccess = true;
	ProcessCommand(command, patchSuccess);
	return patchSuccess;
}

/******************************************************************************
Processes a command file
******************************************************************************/
bool CPatcher::ProcessFile(CString cmdFile)
{
	int numCmds = CmdFileLines(cmdFile);
	int numDone = 0;
	fCmds = numCmds;
	fDone = numDone;	
	m_wnd->SetProgress2(fCmds, fDone);

	//Runs commands line by line
	int pos = 0;
	bool patchSuccess = true;
	while((pos < cmdFile.GetLength()) && patchSuccess)
	{
		//Process the line
		ProcessCommand(cmdFile.Tokenize("\n", pos), patchSuccess);

		//Set Lines
		fCmds = numCmds;
		fDone = ++numDone;
		m_wnd->SetProgress2(fCmds, fDone);
	}

	return patchSuccess;
}

bool CPatcher::ProcessFile(CFile& commandFile)
{
	int numCommands = 0; //TODO: Determine number of commands in file
	int numCompleted = 0;
	m_wnd->SetProgress2(numCommands, numCompleted);

	CList<Command*> CommandList;

	CString line;
	while(GetLine(commandFile, line))
	{
		CString command;
		ArgList* args = new ArgList;
		ParseLine(line, command, args);

		CommandList.AddTail(PatcherCommand::MakeCommand(this, command, args));
		
		line.Empty();
	}

	POSITION pos = CommandList.GetHeadPosition();
	while(pos)
	{
		CommandList.GetNext(pos)->Execute();
	}
	
	//Runs commands line by line
	int position = 0;
	bool patchSuccess = true;
	

}

bool CPatcher::GetLine(CFile& commandFile, CString& line)
{
	char c;
	if(!commandFile.Read(&c, 1))
		return false;
	
	line.AppendChar(c);
	while(commandFile.Read(&c, 1))
		if(c == '\n')
			return true;
		else
			line.AppendChar(c);
	
	return true;
}

void CPatcher::ParseLine(CString& line, CString& command, ArgList* args)
{
	int pos = 0;
	command = line.Tokenize(" \t\r\n", pos);
	
	CString arg = line.Tokenize("|\t\r\n", pos);
	for(int i = 1; pos != -1; i++)
	{
		CString name;
		name.Format("arg%d", i);
		args->SetAt(name, arg);
		
		arg = line.Tokenize("|\t\r\n", pos);
	}
}

bool CPatcher::ProcessFromServer(CDownloader &downloader, CString fileName)
{
	CMemFile commandFile;

	//Download file to memory
	if(!downloader.Download(fileName, commandFile))
		return false;

	//Process commands in file
	if(!ProcessFile(commandFile))
		return false;

	return true;
}

/******************************************************************************
Returns number of lines in command file
******************************************************************************/
int CPatcher::CmdFileLines(CString cmd)
{
	int pos = 0;
	int count = 0;
	while(pos < cmd.GetLength())
	{
		CmdList cmds;
		ParseCommand(cmd.Tokenize("\n", pos), cmds);
		if(!cmds.command.IsEmpty())
			count++;
	}

	return count;
}

//Processes single file patching
//patchfile [Path of Patch on Server]|[Local Computer Path]
bool CPatcher::cmdPatchFile(ArgList *args)
{
	CString arg1, arg2;
	if(!args->Lookup("arg1", arg1) || !args->Lookup("arg2", arg2))
		return false;

	//CString tempPatch, tempBackup;
	CString fPatch, fBackup;
	fPatch.Format("%s", arg1);
	fBackup.Format("%s", arg2);

	//Replace \ with /
	fBackup.Replace('\\', '/');

	//Get file name from path
	int cPos = arg1.ReverseFind('/');
	if(cPos >= 0)
		fPatch.Format("%s", arg1.Right(arg1.GetLength() - cPos - 1));

	CString PatchDest, FilePath, BackupURL, PatchURL;
	PatchDest.Format(	"%s%s",			updaterDir,	fPatch);
	FilePath.Format(	"%s%s",			rootDir,	arg2);
	BackupURL.Format(	"install/%s",	fBackup);
	PatchURL.Format(	"patches/%s",	arg1);

	m_wnd->SetStatus("Patching: %s", fPatch);

	//Checks to see if the entire file has already been downloaded
	bool fileClean = false;
	for(int g = 0; g < goodFiles.GetSize(); g++)
	{
		CString tempDL;
		tempDL = goodFiles.GetAt(g);
		if(!tempDL.CompareNoCase(FilePath))
			fileClean = true;
	}

	//Download patch file to disk
	if(!fileClean && m_Downloader->DownloadToFile(PatchURL, PatchDest))
	{
		//Remove bad attributes from file(read-only, hidden, etc)
		CFileHelper::SetFileNormal(FilePath);

		//Get patch file information
		PATCH_FILE_INFORMATION patchInfo;
		patchInfo.cbStructure = sizeof(PATCH_FILE_INFORMATION);
		GetPatchFileInformation(PatchDest, NULL, PATCH_INFO_REFERENCE_FILE, &patchInfo);

		//Processes patch file
		if(!ApplyPatchFile(PatchDest, patchInfo.dwPatchOptions, 0, 0, FilePath, 0, (LPPATCHSTATUSPROC)PatchStatusProc, (LPARAM)this))
		{
			m_wnd->SetProgress3(0);

			//Add log entry with error info
			CLog::Instance()->AddLog("(ProcessCommand) ApplyPatchFile Error: 0x%x File: %s LocalVersion: %d", GetLastError(), arg2, m_Settings->GetInt("LocalVersion", true));

			m_wnd->SetStatus("Repairing: %s", arg2);

			//Download full file to disk if patching fails
			if(m_Downloader->DownloadToFile(BackupURL, FilePath))
			{										
				//Add File to list of full file downloads
				CString tempDL;
				tempDL.Format(FilePath);
				goodFiles.Add(tempDL);
			}
			else
				return (m_PatchSucceeded = false);
				//patchSuccess = false;
		}

		m_wnd->SetProgress3(0);

		//Remove patch file
		CFileHelper::Delete(PatchDest);
	}
	else if(!fileClean)
		return (m_PatchSucceeded = false);
		//patchSuccess = false;

	return true;
}

//Sets version if all patching was successful
//setverif [Version Number]
bool CPatcher::cmdSetVerIf(ArgList *args)
{
	CString arg1;
	if(!args->Lookup("arg1", arg1))
		return false;

	if(m_PatchSucceeded)
	{
		m_Settings->SetStr("LocalVersion", arg1, true);
		m_wnd->SetVersion(m_Settings->GetInt("RemoteVersion"), m_Settings->GetInt("LocalVersion", true));
		return true;
	}
	
	return false;
}

//Downloads and adds a whole new file
//addfile [Path on Server]|[Path on Computer]
bool CPatcher::cmdAddFile(ArgList *args)
{
	CString arg1, arg2;
	if(!args->Lookup("arg1", arg1) || !args->Lookup("arg2", arg2))
		return false;

	CString FileURL;
	FileURL.Format("patches/%s", arg1);
	CString FileDest;
	FileDest.Format("%s%s", rootDir, arg2);

	//Checks to see if the entire file has already been downloaded
	bool fileClean = false;
	for(int g = 0; g < goodFiles.GetSize(); g++)
	{
		CString tempDL;
		tempDL = goodFiles.GetAt(g);
		if(!tempDL.CompareNoCase(FileDest))
			fileClean = true;
	}

	//Download file to disk
	if(!fileClean && m_Downloader->DownloadToFile(FileURL, FileDest))
	{
		//Add File to list of full file downloads
		CString tempDL;
		tempDL.Format(FileDest);
		goodFiles.Add(tempDL);
	}
	else if(!fileClean)
		return (m_PatchSucceeded = false);

	return true;
}

//Deletes a specified file
//delfile [Local Path to File]
bool CPatcher::cmdDelFile(ArgList *args)
{
	CString arg1;
	if(!args->Lookup("arg1", arg1))
		return false;

	//Create full path
	CString FileDest;
	FileDest.Format("%s%s", rootDir, arg1);

	//Delete file
	CFileHelper::Delete(FileDest);

	//Removes file from clean files list if it's had a full download
	for(int g = 0; g < goodFiles.GetSize(); g++)
	{
		CString tempDL;
		tempDL = goodFiles.GetAt(g);
		if(!tempDL.CompareNoCase(FileDest))
			goodFiles.RemoveAt(g);
	}

	return true;
}

//Sets a registry value
//regedit [Key]|[Value]
bool CPatcher::cmdRegEdit(ArgList *args)
{
	CString arg1, arg2;
	if(!args->Lookup("arg1", arg1) || !args->Lookup("arg2", arg2))
		return false;

	m_Settings->SetStr(arg1, arg2, true);

	return true;
}

//Sets version no matter what
//setversion [Version Number]
bool CPatcher::cmdSetVersion(ArgList *args)
{
	CString arg1;
	if(!args->Lookup("arg1", arg1))
		return false;

	m_Settings->SetStr("LocalVersion", arg1, true);
	m_wnd->SetVersion(m_Settings->GetInt("RemoteVersion"), m_Settings->GetInt("LocalVersion", true));

	return true;
}

//Sets remote version
//remoteversion [Version Number]
bool CPatcher::cmdRemoteVersion(ArgList *args)
{
	CString arg1;
	if(!args->Lookup("arg1", arg1))
		return false;

	m_Settings->SetStr("RemoteVersion", arg1);
	m_wnd->SetVersion(m_Settings->GetInt("RemoteVersion"), m_Settings->GetInt("LocalVersion", true));

	return true;
}

//Adds file/checksum to udFiles
//uddfile [File Name]|[MD5 Checksum of File]
bool CPatcher::cmdUDDFile(ArgList *args)
{
	CString arg1, arg2;
	if(!args->Lookup("arg1", arg1) || !args->Lookup("arg2", arg2))
		return false;

	CFileHelper::CFileObj temp;
	temp.FileName.Format("%s", arg1);
	temp.Checksum.Format("%s", arg2);
	temp.FilePath.Format("%s%s", updaterDir, temp.FileName);
	temp.FileExt = CFileHelper::GetExt(temp.FileName);
	temp.RootDir = updaterDir;
	udFiles.Add(temp);

	return true;
}

//Displays a message to the user
//msgbox [Message]
//msgbox [Title]|[Message]
bool CPatcher::cmdMsgBox(ArgList *args)
{
	CString arg1, arg2;

	if(args->Lookup("arg1", arg1))
	{
		if(args->Lookup("arg2", arg2))
			theApp.m_pMainWnd->MessageBox(arg2, arg1);
		else
			theApp.m_pMainWnd->MessageBox(arg1, "Update Information!");
	}
	
	return true;
}

//Adds line to log file
//CLog::Instance()->AddLog [Log Entry]
bool CPatcher::cmdAddLog(ArgList *args)
{
	CString arg1;
	if(!args->Lookup("arg1", arg1))
		return false;

	CLog::Instance()->AddLog("%s", arg1);

	return true;
}

//Download and run patch command file
//cmdfile commands/[file].txt
bool CPatcher::cmdCmdFile(ArgList *args)
{
	CString arg1;
	if(!args->Lookup("arg1", arg1))
		return false;

	CString cmdFileB;
	if(m_Downloader->DownloadToMem(arg1, cmdFileB))
		if(!ProcessFile(cmdFileB))
			m_PatchSucceeded = false;

	return true;
}

//Check updater directory for consistency
//checkupdatedir
bool CPatcher::cmdCheckUpdateDir(ArgList *args)
{	
	if(m_Settings->GetStr("runState", true).CompareNoCase("NOCHECK") && (udFiles.GetSize() > 1))
		if(!CheckUpdateDir())
			return (m_PatchSucceeded = false);

	return true;
}

bool CPatcher::cmdLaunchApp(ArgList *args)
{
	theApp.LaunchApp();

	return true;
}

bool CPatcher::cmdUnknown(ArgList *args)
{
	CString command, arg1, arg2;

	args->Lookup("cmd", command);
	args->Lookup("arg1", arg1);
	args->Lookup("arg2", arg2);

	CLog::Instance()->AddLog("(ProcessCommand) Un-Supported Command: \"%s\" Arg1: %s  Arg2: %s", command, arg1, arg2);

	return true;
}

/******************************************************************************
Checks updater directory and updates if neccesary
******************************************************************************/
bool CPatcher::CheckUpdateDir()
{
	//Checks every file in udFiles list
	CArray<CFileHelper::CFileObj> UpdaterFileList;
	CArray<CFileHelper::CCmpObj> ResultList;
	bool bLaunch = false; //Return value

	//Makes a list of files in updater directory
	CFileHelper::DirList(UpdaterFileList, updaterDir);

	//Compare the two lists of files
	CFileHelper::CompareFileLists(ResultList, UpdaterFileList, udFiles);

	//Loop through result list
	for(int i = 0; i < ResultList.GetSize(); i++)
	{
		CFileHelper::CCmpObj temp = ResultList.GetAt(i);
		switch (temp.FileState)
		{
		case CFileHelper::CCmpObj::EXTRA : //Delete File
			CFileHelper::Delete(temp.FileObj.FilePath);
			break;

		case CFileHelper::CCmpObj::MODIFIED :
			if(temp.FileObj.FileName != CString(CConfig::SECONDARY_UPDATER))
			{
				temp.FileObj.FilePath.Append(".temp");
				bLaunch = true;
			}
			m_Downloader->DownloadToFile(temp.FileObj.FileName, temp.FileObj.FilePath);
			break;
		
		case CFileHelper::CCmpObj::MISSING :
			bLaunch = true;
			m_Downloader->DownloadToFile(temp.FileObj.FileName, temp.FileObj.FilePath);
			break;
		}
	}

	//Launch UpdateCheck.exe if needed
	if(bLaunch)
	{
		//Get command line
		CCommandLineInfo cli;
		theApp.ParseCommandLine(cli);	

		CString cmd;
		cmd.Format("%s \"%s\"", CConfig::SECONDARY_UPDATER, cli.m_strFileName);

		int nLen = cmd.GetLength();
		LPTSTR lpszBuf = cmd.GetBuffer(nLen);

		//Launches UpdateCheck.exe
		STARTUPINFO sinfo = {0};
		PROCESS_INFORMATION pinfo = {0};
		sinfo.cb = sizeof(sinfo);
		CreateProcess(CConfig::SECONDARY_UPDATER, lpszBuf, 0, 0, 0, CREATE_NO_WINDOW, 0, 0, &sinfo, &pinfo);

		cmd.ReleaseBuffer();

		theApp.ClosePatcher();
	}

	return !bLaunch;
}