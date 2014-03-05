#include "stdafx.h"
#include "PatchBuilder.h"

/******************************************************************************
Creates PatchBuilder
******************************************************************************/
PatchBuilder::PatchBuilder()
{
	//Initialize ActivePatch Library
	InitActivePatch(APATCH_LICENSE_KEY, 0);
}

/******************************************************************************
PatchBuilder Object
******************************************************************************/
PatchBuilder patchbuilder;

/******************************************************************************
Initializes PatchBuilder
******************************************************************************/
BOOL PatchBuilder::InitInstance()
{
	//Initializes WinXP control manifest
	InitCommonControls();

	//Initializes the Application
	CWinApp::InitInstance();

	//Creates the Dialog
	PBDlg pbDlg(patchbuilder.m_pMainWnd);
	m_pMainWnd = &pbDlg;
	pbDlg.DoModal();

	//Exits the Application
	return FALSE;
}

/******************************************************************************
Starts thread to create Patches
******************************************************************************/
void PatchBuilder::BuildPatch()
{
	Settings.Type = NORMAL;
	if(!GetSettings())
	{
		m_pMainWnd->SetDlgItemText(IDC_STATUS, L"Patch Creation FAILED! One or more specified paths were invalid...");
		return;
	}

	bThread = AfxBeginThread((AFX_THREADPROC)PatchBuilderBuild,this,0,0,0,0);
}

/******************************************************************************
Starts thread to create Jump Patch
******************************************************************************/
void PatchBuilder::BuildJump()
{
	Settings.Type = JUMP;
	if(!GetSettings())
	{
		m_pMainWnd->SetDlgItemText(IDC_STATUS, L"Patch Creation FAILED! One or more specified paths were invalid...");
		return;
	}

	Settings.OldVer = Settings.InstallerVer;
	Settings.FilePatchDir = Settings.InstallerVer;
	Settings.OldVerDir = Settings.InstallerDir;

	bThread = AfxBeginThread((AFX_THREADPROC)PatchBuilderBuildJump,this,0,0,0,0);
}

/******************************************************************************
Starts thread to create Updater file
******************************************************************************/
void PatchBuilder::BuildPatchFile()
{
	Settings.Type = NORMAL;
	if(!GetSettings())
	{
		m_pMainWnd->SetDlgItemText(IDC_STATUS, L"Updater File Creation FAILED! One or more specified paths were invalid...");
		return;
	}

	bThread = AfxBeginThread((AFX_THREADPROC)PatchBuilderFileBuild,this,0,0,0,0);
}

/******************************************************************************
Patch build process
******************************************************************************/
void PatchBuilder::BuildProc()
{
	//Sets status and locks buttons
	m_pMainWnd->GetDlgItem(IDC_BUTTON_BUILD)->EnableWindow(FALSE);
	m_pMainWnd->GetDlgItem(IDC_BUTTON_BUILD_UPDATER)->EnableWindow(FALSE);
	m_pMainWnd->GetDlgItem(IDC_BUTTON_BUILD_JUMP)->EnableWindow(FALSE);
	m_pMainWnd->SetDlgItemText(IDC_STATUS, L"Processing Files. Please Wait...");

	//Initiates file lists
	CArray<FileNode> oList, cList;

	//Recursively add files to list
	DirSearch(Settings.OldVerDir, Settings.OldVerDir, &oList);
	DirSearch(Settings.CurrentVerDir, Settings.CurrentVerDir, &cList);

	//Creates patch info file
	CFile file;
	CString filePath, fileDir;

	fileDir.Format(L"%scommands\\", Settings.OutputFolder);
	CreateDirectory(fileDir, NULL);

	filePath.Format(L"%s%s.txt", fileDir, Settings.OldVer);
	file.Open(filePath, CFile::modeCreate | CFile::modeWrite);

	//Loops through all files and compares
	bool fMissing;
	int nAdd(0), nRem(0), nChange(0), nOFiles(0), nCFiles(0);
	nOFiles = (int)oList.GetUpperBound() + 1;
	nCFiles = (int)cList.GetUpperBound() + 1;
	for(int i = 0; i <= oList.GetUpperBound(); i++)
	{
		FileNode oldFile = oList.GetAt(i);
		fMissing = true;

		CString status;
		status.Format(L"Processing File: [%s] %d/%d Stage 1 of 2", oldFile.fileName, i, oList.GetUpperBound());
		m_pMainWnd->SetDlgItemText(IDC_STATUS, status);

		for(int j = 0; j <= cList.GetUpperBound(); j++)
		{
			FileNode curFile = cList.GetAt(j);

			//Compares file names
			if(oldFile.fileName.CompareNoCase(curFile.fileName) == 0)
				if(oldFile.filePath.CompareNoCase(curFile.filePath) == 0)
				{
					CString oldPath, curPath;
					oldPath.Format(L"%s%s", Settings.OldVerDir, oldFile.filePath);
					curPath.Format(L"%s%s", Settings.CurrentVerDir, curFile.filePath);
					fMissing = false;

					if(getMD5(oldPath).CompareNoCase(getMD5(curPath)) != 0)
					{
						//Increments number of changed files
						nChange++;

						CString patchFileName, patchLoc;
						patchFileName.Format(L"%s.%s", curFile.fileName, CConfig::PATCH_EXT);
						patchLoc.Format(L"%s%s", Settings.PatchPath, patchFileName);

						int fNum = 2;
						while(FExists(patchLoc))
						{
							patchFileName.Format(L"%s%d.%s", curFile.fileName, fNum, CConfig::PATCH_EXT);
							patchLoc.Format(L"%s%s", Settings.PatchPath, patchFileName);
							fNum++;
						}

						//Line to write to file
						CString line;
						line.Format(L"patchfile %s/%s|%s\n", Settings.FilePatchDir, patchFileName, curFile.filePath);
						file.Write(line.GetString(), line.GetLength());

						//Creates folder for patches if non existent
						if(!PathIsDirectory(Settings.PatchesDir))
							CreateDirectory(Settings.PatchesDir, NULL);
						if(!PathIsDirectory(Settings.PatchPath))
							CreateDirectory(Settings.PatchPath, NULL);

						//Generates patch						
						MakePatchFile(oldPath, curPath, patchLoc);
					}
				}
		}

		//Delete file and add line
		if(fMissing)
		{
			//Checks if minor patch
			if(!Settings.MinorPatch)
			{
				//Increments number
				nRem++;

				//Writes delete line to file
				CString line;
				line.Format(L"delfile %s\n", oldFile.filePath);
				file.Write(line.GetString(), line.GetLength());
			}
		}
	}

	//Searches for new files to add
	for(int i = 0; i <= cList.GetUpperBound(); i++)
	{
		FileNode curFile = cList.GetAt(i);
		fMissing = true;

		CString status;
		status.Format(L"Processing File: [%s] %d/%d Stage 2 of 2", curFile.fileName, i, cList.GetUpperBound());
		m_pMainWnd->SetDlgItemText(IDC_STATUS, status);

		for(int j = 0; j <= oList.GetUpperBound(); j++)
		{
			FileNode oldFile = oList.GetAt(j);
			if(curFile.fileName.CompareNoCase(oldFile.fileName) == 0)
				if(curFile.filePath.CompareNoCase(oldFile.filePath) == 0)
					fMissing = false;
		}

		//Add file to directory and add line
		if(fMissing)
		{
			//Increments number of new files
			nAdd++;

			CString patchFileName, patchLoc;
			patchFileName.Format(L"%s.temp", curFile.fileName);
			patchLoc.Format(L"%s%s", Settings.PatchPath, patchFileName);

			int fNum = 2;
			while(FExists(patchLoc))
			{
				patchFileName.Format(L"%s%d.temp", curFile.fileName, fNum);
				patchLoc.Format(L"%s%s", Settings.PatchPath, patchFileName);
				fNum++;
			}

			//Write addfile line to file
			CString line;
			line.Format(L"addfile %s/%s|%s\n", Settings.FilePatchDir, patchFileName, curFile.filePath);
			file.Write(line.GetString(), line.GetLength());

			//Creates folder for patches if non existent
			if(!PathIsDirectory(Settings.PatchesDir))
				CreateDirectory(Settings.PatchesDir, NULL);
			if(!PathIsDirectory(Settings.PatchPath))
				CreateDirectory(Settings.PatchPath, NULL);

			//Copies file to patch folder
			CString curPath;
			curPath.Format(L"%s%s", Settings.CurrentVerDir, curFile.filePath);
			CopyFile(curPath, patchLoc, false);
		}
	}

	//Adds new version line
	CString ver;
	ver.Format(L"setverif %s", Settings.CurrentVer);
	file.Write(ver.GetString(), ver.GetLength());

	//Closes the text file
	file.Close();

	//Locks dialog and Increments version #s
	if(Settings.Type == NORMAL)
	{
		m_pMainWnd->EnableWindow(FALSE);
		m_pMainWnd->SetDlgItemText(IDC_OLD_VERSION, Settings.CurrentVer);
		int newVer = _wtoi(Settings.CurrentVer.GetString());
		newVer++;
		Settings.CurrentVer.Format(L"%d", newVer);
		m_pMainWnd->SetDlgItemText(IDC_CURRENT_VERSION, Settings.CurrentVer);
		m_pMainWnd->EnableWindow();
	}

	//Updates status and unlocks buttons
	CString status;
	status.Format(L"Done! Changed: %d, Deleted: %d, Added: %d, Old Files: %d, New Files: %d", nChange, nRem, nAdd, nOFiles, nCFiles);
	m_pMainWnd->SetDlgItemText(IDC_STATUS, status);	
	m_pMainWnd->GetDlgItem(IDC_BUTTON_BUILD)->EnableWindow();
	m_pMainWnd->GetDlgItem(IDC_BUTTON_BUILD_UPDATER)->EnableWindow();
	m_pMainWnd->GetDlgItem(IDC_BUTTON_BUILD_JUMP)->EnableWindow();
}

/******************************************************************************
Does a recursive search of directory and adds files to the CArray
******************************************************************************/
void PatchBuilder::DirSearch(CString path, CString basePath, CArray<FileNode> *list)
{
	CFileFind find;	
	CString searchdir, append;

	append.Empty();
	if(path.Mid(path.GetLength(), 1) != "\\")
		append.Format(L"\\");

	searchdir.Format(L"%s%s*.*", path, append);

	find.FindFile(searchdir);
	find.FindNextFile();
	while(find.FindNextFile())
		if(!find.IsDots())
		{
			if(find.IsDirectory())
				DirSearch(find.GetFilePath(), basePath, list);
			else
			{
				FileNode temp;
				temp.filePath.Format(L"%s", find.GetFilePath());
				temp.filePath.Replace(basePath, L"");
				temp.fileName.Format(L"%s", find.GetFileName());
				list->Add(temp);
			}
		}
	if(!find.IsDots())
	{
		if(find.IsDirectory())
			DirSearch(find.GetFilePath(), basePath, list);
		else
		{
			FileNode temp;
			temp.filePath.Format(L"%s", find.GetFilePath());
			temp.filePath.Replace(basePath, L"");
			temp.fileName.Format(L"%s", find.GetFileName());
			list->Add(temp);
		}
	}

	return;
}

/******************************************************************************
Gets MD5 hash of given file
******************************************************************************/
CString PatchBuilder::getMD5(CString file)
{
	CFile md5File;
	CString md5Str;
	//Gets checksum
	if(md5File.Open(file, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
	{
		md5Str = CMD5Checksum::GetMD5(md5File);
		md5File.Close();
	}
	return md5Str;
}

/******************************************************************************
Creates a Patch file using ActivePatch
******************************************************************************/
void PatchBuilder::MakePatchFile(CString OldPath, CString NewPath, CString PatchPath)
{
	//Creates ActivePatch object and creates patch
	CreatePatchFile(PatchPath.GetString(), PATCH_LEVEL_DEFAULT, Settings.Options, NULL, NULL, NULL, OldPath.GetString(), NewPath.GetString(), NULL, NULL);
}

/******************************************************************************
Gets Settings from Dialog
******************************************************************************/
bool PatchBuilder::GetSettings()
{	
	//Locks and updates dialog
	m_pMainWnd->EnableWindow(FALSE);
	m_pMainWnd->SetDlgItemText(IDC_STATUS, L"Accessing Settings...");
	
	m_pMainWnd->GetDlgItemText(IDC_OUTPUT_FOLDER, Settings.OutputPath);
	if(Settings.OutputPath.Right(1).CompareNoCase(L"\\"))
		Settings.OutputPath.Append(L"\\");
	
	m_pMainWnd->GetDlgItemText(IDC_CURRENT_VERSION, Settings.CurrentVer);
	
	m_pMainWnd->GetDlgItemText(IDC_OLD_VERSION, Settings.OldVer);

	m_pMainWnd->GetDlgItemText(IDC_INSTALLER_VERSION, Settings.InstallerVer);

	m_pMainWnd->GetDlgItemText(IDC_CURRENT_DIRECTORY, Settings.CurrentVerDir);
	if(Settings.CurrentVerDir.Right(1).CompareNoCase(L"\\"))
		Settings.CurrentVerDir.Append(L"\\");
	
	m_pMainWnd->GetDlgItemText(IDC_OLD_DIRECTORY, Settings.OldVerDir);
	if(Settings.OldVerDir.Right(1).CompareNoCase(L"\\"))
		Settings.OldVerDir.Append(L"\\");

	m_pMainWnd->GetDlgItemText(IDC_INSTALLER_DIRECTORY, Settings.InstallerDir);
	if(Settings.InstallerDir.Right(1).CompareNoCase(L"\\"))
		Settings.InstallerDir.Append(L"\\");
	
	m_pMainWnd->GetDlgItemText(IDC_FILE_PATCH_DIRECTORY, Settings.FilePatchDir);

	//Gets path to updater files
	m_pMainWnd->GetDlgItemText(IDC_UPDATER_DIRECTORY, Settings.UpdaterDir);
	if(Settings.UpdaterDir.Right(1).CompareNoCase(L"\\"))
		Settings.UpdaterDir.Append(L"\\");

	m_pMainWnd->GetDlgItemText(IDC_APPLIED_VERSION, Settings.UpdaterAppliedVer);

	//Grabs patch options from dialog
	Settings.Options = 0;

	if(m_pMainWnd->IsDlgButtonChecked(IDC_COMPARE_FILE_TIME))
		Settings.Options |= PATCH_OPTION_COMPARE_FILETIME;

	if(m_pMainWnd->IsDlgButtonChecked(IDC_COMPARE_VERSION))
		Settings.Options |= PATCH_OPTION_COMPARE_VERSION;
	
	if(m_pMainWnd->IsDlgButtonChecked(IDC_IGNORE_ATTRIBUTES))
		Settings.Options |= PATCH_OPTION_IGNORE_ATTRIBUTES;

	if(m_pMainWnd->IsDlgButtonChecked(IDC_FIND_FILE))
		Settings.Options |= PATCH_OPTION_FIND_FILE;

	if(m_pMainWnd->IsDlgButtonChecked(IDC_IGNORE_FILE_TIME))
		Settings.Options |= PATCH_OPTION_IGNORE_FILETIME;

	if(m_pMainWnd->IsDlgButtonChecked(IDC_IGNORE_VERSION))
		Settings.Options |= PATCH_OPTION_IGNORE_VERSION;

	if(m_pMainWnd->IsDlgButtonChecked(IDC_IGNORE_READ_ONLY))
		Settings.Options |= PATCH_OPTION_IGNORE_READONLY;

	if(m_pMainWnd->IsDlgButtonChecked(IDC_BACKUP_FILE))
		Settings.Options |= PATCH_OPTION_BACKUP_FILE;

	//Gets minor patch setting
	Settings.MinorPatch = m_pMainWnd->IsDlgButtonChecked(IDC_MINOR_PATCH);
	
	//Enables window and updates status
	m_pMainWnd->SetDlgItemText(IDC_STATUS, L"Settings Acquired! Validating...");
	m_pMainWnd->EnableWindow();

	if(!PathIsDirectory(Settings.OutputPath.GetString()))
		return false;
	if(!PathIsDirectory(Settings.CurrentVerDir.GetString()))
		return false;
	if(!PathIsDirectory(Settings.OldVerDir.GetString()))
		return false;
	if(!PathIsDirectory(Settings.UpdaterDir.GetString()))
		return false;
	if(!PathIsDirectory(Settings.InstallerDir.GetString()))
		return false;

	//Finds non-existent directory
	Settings.OutputFolder.Format(L"%s", Settings.OutputPath);
	if(Settings.Type == NORMAL)
		Settings.OutputFolder.AppendFormat(L"%s", Settings.CurrentVer);
	else
		Settings.OutputFolder.AppendFormat(L"Jump Patch");
	CString temp, suffix;
	int num = 1;
	suffix.Format(L"\\");
	temp.Format(L"%s%s", Settings.OutputFolder, suffix);
	while(PathIsDirectory(temp))
	{
		num++;
		suffix.Format(L" [%d]\\", num);
		temp.Format(L"%s%s", Settings.OutputFolder, suffix);
	}
	Settings.OutputFolder.AppendFormat(L"%s", suffix);
	CreateDirectory(Settings.OutputFolder, NULL);

	//Sets path to place patches
	Settings.PatchesDir.Format(L"%spatches\\", Settings.OutputFolder);
	if(Settings.Type == NORMAL)
		Settings.PatchPath.Format(L"%s%s\\", Settings.PatchesDir, Settings.FilePatchDir);
	else
		Settings.PatchPath.Format(L"%s%s\\", Settings.PatchesDir, Settings.InstallerVer);

	//Sets status
	m_pMainWnd->SetDlgItemText(IDC_STATUS, L"Settings Valid!");

	return true;
}


/******************************************************************************
Builds Updater files list
******************************************************************************/
void PatchBuilder::BuildFile()
{
	//Sets status and locks buttons
	m_pMainWnd->GetDlgItem(IDC_BUTTON_BUILD)->EnableWindow(FALSE);
	m_pMainWnd->GetDlgItem(IDC_BUTTON_BUILD_UPDATER)->EnableWindow(FALSE);
	m_pMainWnd->GetDlgItem(IDC_BUTTON_BUILD_JUMP)->EnableWindow(FALSE);
	m_pMainWnd->SetDlgItemText(IDC_STATUS, L"Building Updater File...");

	//Removes udata.txt if present
	CString uFilePath;
	uFilePath.Format(L"%spdata.txt", Settings.OutputFolder);
	if(PathFileExists(uFilePath))
	{
		SetFileAttributes(uFilePath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(uFilePath);
	}

	int i = 0;
	CFile file;
	
	//Checks file status
	if(file.Open(uFilePath, CFile::modeCreate | CFile::modeWrite))
	{
		//Writes version to file
		CString ver;
		ver.Format(L"remoteversion %s\n", Settings.UpdaterAppliedVer);
		file.Write(ver.GetString(), ver.GetLength());

		//Cycles through files
		CFileFind find;
		
		CString path, searchdir;
		searchdir.Format(L"%s*.*", Settings.UpdaterDir);
		find.FindFile(searchdir);
		find.FindNextFile();
		while(find.FindNextFile())
			if(!find.IsDirectory() && !find.IsDots())
			{
				//Writes line to udata.txt
				path.Format(L"uddfile %s|%s\n", find.GetFileName(), getMD5(find.GetFilePath()));
				file.Write(path.GetString(), path.GetLength());
				i++;
			}
		if(!find.IsDirectory() && !find.IsDots())
		{
			//Writes line to udata.txt
			path.Format(L"uddfile %s|%s\n", find.GetFileName(), getMD5(find.GetFilePath()));
			file.Write(path.GetString(), path.GetLength());
			i++;
		}

		//Write patchinfo line
		CString patchInfo;
		patchInfo.Format(L"uddfile Updater Log.txt|NULL\n");
		patchInfo.AppendFormat(L"uddfile setup.txt|NULL\n");
		file.Write(patchInfo.GetString(), patchInfo.GetLength());

		//Write checkupdatedir line
		CString chkupdate;
		chkupdate.Format(L"checkupdatedir");
		file.Write(chkupdate.GetString(), chkupdate.GetLength());
		
		//Closes the file
		file.Close();
	}

	//Sets status and unlocks buttons
	CString status;
	status.Format(L"File Created at: %s   Files: %d", uFilePath, i);
	m_pMainWnd->SetDlgItemText(IDC_STATUS, status);
	m_pMainWnd->GetDlgItem(IDC_BUTTON_BUILD)->EnableWindow();
	m_pMainWnd->GetDlgItem(IDC_BUTTON_BUILD_UPDATER)->EnableWindow();
	m_pMainWnd->GetDlgItem(IDC_BUTTON_BUILD_JUMP)->EnableWindow();

	return;
}

/******************************************************************************
Checks if a file exists
******************************************************************************/
bool PatchBuilder::FExists(CString path)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(path.GetString(), &wfd);

	if(hFind == INVALID_HANDLE_VALUE)
		return false;
	else
	{
		FindClose(hFind);
		return true;
	}
}

/******************************************************************************
Patch Builder Proc Functions
******************************************************************************/
UINT PatchBuilderBuild(PatchBuilder *cpa)
{
	cpa->BuildFile();
	cpa->BuildProc();
	return TRUE;
}

UINT PatchBuilderFileBuild(PatchBuilder *cpa)
{
	cpa->BuildFile();
	return TRUE;
}

UINT PatchBuilderBuildJump(PatchBuilder *cpa)
{
	cpa->BuildProc();
	return TRUE;
}