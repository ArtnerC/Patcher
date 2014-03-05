#include "stdafx.h"
#include "PBDlg.h"

/******************************************************************************
Creates PBDlg
******************************************************************************/
PBDlg::PBDlg(CWnd* pParent /*=NULL*/):CDialog(PBDlg::IDD, pParent)
{
}

/******************************************************************************
Handles Data Exchange
******************************************************************************/
void PBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/******************************************************************************
Message Map for PBDlg
******************************************************************************/
BEGIN_MESSAGE_MAP(PBDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_BUILD, OnBnClickedButtonBuild)
	ON_BN_CLICKED(IDC_BUTTON_BUILD_UPDATER, OnBnClickedButtonBuildUpdater)
	ON_BN_CLICKED(IDC_OUTPUT_BROWSE, OnOutputBrowse)
	ON_BN_CLICKED(IDC_UPDATER_DIRECTORY_BROWSE, OnUpdaterDirectoryBrowse)
	ON_BN_CLICKED(IDC_OLD_DIRECTORY_BROWSE, OnOldDirectoryBrowse)
	ON_BN_CLICKED(IDC_CURRENT_DIRECTORY_BROWSE, OnCurrentDirectoryBrowse)
	ON_BN_CLICKED(IDC_DEFAULT_VERSION, OnClickedDefaultVersion)
	ON_BN_CLICKED(IDC_DEFAULT_PATH, OnClickedDefaultPath)
	ON_EN_CHANGE(IDC_CURRENT_VERSION, OnChangeCurrentVersion)
	ON_EN_CHANGE(IDC_OLD_VERSION, OnChangeOldVersion)
	ON_BN_CLICKED(IDC_INSTALLER_DIRECTORY_BROWSE, OnInstallerDirectoryBrowse)
	ON_BN_CLICKED(IDC_BUTTON_BUILD_JUMP, OnBnClickedButtonBuildJump)
END_MESSAGE_MAP()

/******************************************************************************
Handles InitDialog
******************************************************************************/
BOOL PBDlg::OnInitDialog()
{
	//Initializes the Dialog
	CDialog::OnInitDialog();

	//Loads the Icon
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//Sets the Icons
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	//Sets up dialog with registry entries
	CString temp;
	temp = GetRegValue(L"OLDVERSION", L"0");
	PBDlg::SetDlgItemText(IDC_OLD_VERSION, temp);
	temp = GetRegValue(L"CURRENTVERSION", L"1");
	PBDlg::SetDlgItemText(IDC_CURRENT_VERSION, temp);
	temp = GetRegValue(L"INSTALLERVERSION", L"0");
	PBDlg::SetDlgItemText(IDC_INSTALLER_VERSION, temp);

	temp = GetRegValue(L"OLDDIRECTORY", L"");
	PBDlg::SetDlgItemText(IDC_OLD_DIRECTORY, temp);
	temp = GetRegValue(L"CURRENTDIRECTORY", L"");
	PBDlg::SetDlgItemText(IDC_CURRENT_DIRECTORY, temp);
	temp = GetRegValue(L"UPDATERDIRECTORY", L"");
	PBDlg::SetDlgItemText(IDC_UPDATER_DIRECTORY, temp);
	temp = GetRegValue(L"OUTPUTDIRECTORY", L"");
	PBDlg::SetDlgItemText(IDC_OUTPUT_FOLDER, temp);
	temp = GetRegValue(L"INSTALLERDIRECTORY", L"");
	PBDlg::SetDlgItemText(IDC_INSTALLER_DIRECTORY, temp);

	//Sets checked states of appropriate check boxes
	PBDlg::CheckDlgButton(IDC_DEFAULT_PATH, TRUE);
	PBDlg::OnClickedDefaultPath();
	PBDlg::CheckDlgButton(IDC_DEFAULT_VERSION, TRUE);
	PBDlg::OnClickedDefaultVersion();

	PBDlg::CheckDlgButton(IDC_IGNORE_ATTRIBUTES, TRUE);
	PBDlg::CheckDlgButton(IDC_IGNORE_FILE_TIME, TRUE);
	PBDlg::CheckDlgButton(IDC_IGNORE_VERSION, TRUE);
	PBDlg::CheckDlgButton(IDC_IGNORE_READ_ONLY, TRUE);
	
	//Returns Success
	return TRUE;
}

/******************************************************************************
Handles a Paint Message
******************************************************************************/
void PBDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

/******************************************************************************
Runs when dialog is closed
******************************************************************************/
void PBDlg::OnClose()
{
	//Sets registry values if valid
	CString temp;
	PBDlg::GetDlgItemText(IDC_OLD_VERSION, temp);
	SetRegValue(L"OLDVERSION", temp);
	PBDlg::GetDlgItemText(IDC_CURRENT_VERSION, temp);
	SetRegValue(L"CURRENTVERSION", temp);
	PBDlg::GetDlgItemText(IDC_INSTALLER_VERSION, temp);
	SetRegValue(L"INSTALLERVERSION", temp);

	PBDlg::GetDlgItemText(IDC_OLD_DIRECTORY, temp);
	if(PathIsDirectory(temp.GetString()))
		SetRegValue(L"OLDDIRECTORY", temp);
	PBDlg::GetDlgItemText(IDC_CURRENT_DIRECTORY, temp);
	if(PathIsDirectory(temp.GetString()))
		SetRegValue(L"CURRENTDIRECTORY", temp);
	PBDlg::GetDlgItemText(IDC_UPDATER_DIRECTORY, temp);
	if(PathIsDirectory(temp.GetString()))
		SetRegValue(L"UPDATERDIRECTORY", temp);
	PBDlg::GetDlgItemText(IDC_OUTPUT_FOLDER, temp);
	if(PathIsDirectory(temp.GetString()))
		SetRegValue(L"OUTPUTDIRECTORY", temp);
	PBDlg::GetDlgItemText(IDC_INSTALLER_DIRECTORY, temp);
	if(PathIsDirectory(temp.GetString()))
		SetRegValue(L"INSTALLERDIRECTORY", temp);

	return PostQuitMessage(WM_QUIT);
}

/******************************************************************************
Runs Build Patch function
******************************************************************************/
void PBDlg::OnBnClickedButtonBuild()
{
	patchbuilder.BuildPatch();
}

/******************************************************************************
Runs Build file function
******************************************************************************/
void PBDlg::OnBnClickedButtonBuildUpdater()
{
	patchbuilder.BuildPatchFile();
}

/******************************************************************************
Runs Build Jump Patch function
******************************************************************************/
void PBDlg::OnBnClickedButtonBuildJump()
{
	patchbuilder.BuildJump();
}

/******************************************************************************
Handles browse button click
******************************************************************************/
void PBDlg::OnOutputBrowse()
{
	CString folder, title;
	title = "Select Output Path";
	folder = BrowseFolder(title);
	if(!folder.IsEmpty())
		PBDlg::SetDlgItemText(IDC_OUTPUT_FOLDER, folder);
}

/******************************************************************************
Opens browse for folder dialog
******************************************************************************/
CString PBDlg::BrowseFolder(CString title)
{
	BROWSEINFO lpbi;
	LPITEMIDLIST idl;
	wchar_t buff[MAX_PATH];
	CString folder;
	LPMALLOC pMalloc;

	folder.Empty();
	SHGetMalloc(&pMalloc);
	ZeroMemory(&lpbi, sizeof(lpbi));

	lpbi.hwndOwner = GetSafeHwnd();
	lpbi.lParam = 0;
	lpbi.lpfn = NULL;
	lpbi.pidlRoot = NULL;
	lpbi.lpszTitle = title.GetString();
	lpbi.ulFlags = lpbi.ulFlags | BIF_USENEWUI;
	idl = SHBrowseForFolder(&lpbi);

	if(idl)
	{
		if(SHGetPathFromIDList(idl, buff))
		{
			folder.Format(L"%s", buff);
		}
	}

	pMalloc->Free(idl);
	pMalloc->Release();

	if(!PathIsDirectory(folder.GetString()))
		folder.Empty();

	return folder;
}

/******************************************************************************
Handles browse button click
******************************************************************************/
void PBDlg::OnUpdaterDirectoryBrowse()
{
	CString folder, title;
	title = "Select Updater Directory Path";
	folder = BrowseFolder(title);
	if(!folder.IsEmpty())
		PBDlg::SetDlgItemText(IDC_UPDATER_DIRECTORY, folder);
}

/******************************************************************************
Handles browse button click
******************************************************************************/
void PBDlg::OnOldDirectoryBrowse()
{
	CString folder, title;
	title = "Select Old Directory Path";
	folder = BrowseFolder(title);
	if(!folder.IsEmpty())
		PBDlg::SetDlgItemText(IDC_OLD_DIRECTORY, folder);
}

/******************************************************************************
Handles browse button click
******************************************************************************/
void PBDlg::OnCurrentDirectoryBrowse()
{
	CString folder, title;
	title = "Select Current Directory Path";
	folder = BrowseFolder(title);
	if(!folder.IsEmpty())
		PBDlg::SetDlgItemText(IDC_CURRENT_DIRECTORY, folder);
}

/******************************************************************************
Handles browse button click
******************************************************************************/
void PBDlg::OnInstallerDirectoryBrowse()
{
	CString folder, title;
	title = "Select Installer Directory Path";
	folder = BrowseFolder(title);
	if(!folder.IsEmpty())
		PBDlg::SetDlgItemText(IDC_INSTALLER_DIRECTORY, folder);
}


/******************************************************************************
Handles checkbox click
******************************************************************************/
void PBDlg::OnClickedDefaultVersion()
{
	if(PBDlg::IsDlgButtonChecked(IDC_DEFAULT_VERSION))
	{
		CString temp;
		PBDlg::GetDlgItemText(IDC_CURRENT_VERSION, temp);
		PBDlg::SetDlgItemText(IDC_APPLIED_VERSION, temp);
		PBDlg::GetDlgItem(IDC_APPLIED_VERSION)->EnableWindow(FALSE);
	}
	else
		PBDlg::GetDlgItem(IDC_APPLIED_VERSION)->EnableWindow();
}

/******************************************************************************
Handles checkbox click
******************************************************************************/
void PBDlg::OnClickedDefaultPath()
{
	if(PBDlg::IsDlgButtonChecked(IDC_DEFAULT_PATH))
	{
		CString temp;
		PBDlg::GetDlgItemText(IDC_OLD_VERSION, temp);
		PBDlg::SetDlgItemText(IDC_FILE_PATCH_DIRECTORY, temp);
		PBDlg::GetDlgItem(IDC_FILE_PATCH_DIRECTORY)->EnableWindow(FALSE);
	}
	else
		PBDlg::GetDlgItem(IDC_FILE_PATCH_DIRECTORY)->EnableWindow();
}

/******************************************************************************
Sets correct dialog controls
******************************************************************************/
void PBDlg::OnChangeCurrentVersion()
{
	CString temp;
	PBDlg::GetDlgItemText(IDC_CURRENT_VERSION, temp);
	
	if(PBDlg::IsDlgButtonChecked(IDC_DEFAULT_VERSION))
		PBDlg::SetDlgItemText(IDC_APPLIED_VERSION, temp);
}

/******************************************************************************
Sets correct dialog controls
******************************************************************************/
void PBDlg::OnChangeOldVersion()
{
	CString temp;
	PBDlg::GetDlgItemText(IDC_OLD_VERSION, temp);

	if(PBDlg::IsDlgButtonChecked(IDC_DEFAULT_PATH))
		PBDlg::SetDlgItemText(IDC_FILE_PATCH_DIRECTORY, temp);
}

/******************************************************************************
Gets the values from the registry
******************************************************************************/
CString PBDlg::GetRegValue(CString key, CString def)
{
  //Initializes variables
  ULONG vLength=255;
  CHAR value[255];
  HKEY regKey;
  CString val;

  //Loads the stored value from the registry
  if(RegOpenKey(HKEY_LOCAL_MACHINE,L"Software\\Patch Builder",&regKey)==ERROR_SUCCESS)
  {
	vLength=255;
	if(RegQueryValueEx(regKey,key.GetString(),0,0,(UCHAR*)value,&vLength)==ERROR_SUCCESS)
		val.Format(L"%s", value);
	else
		val = def;

    RegCloseKey(regKey);
  }

  //Sets val to default if empty
  if(val.IsEmpty())
	  val = def;

  //Returns value
  return val;
}

/******************************************************************************
Sets a registry value
******************************************************************************/
BOOL PBDlg::SetRegValue(CString key, CString value)
{
  //Initializes variables
  DWORD vLength=50;
  HKEY regKey;

  //Opens the key
  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"Software\\Patch Builder",0,KEY_ALL_ACCESS,&regKey)!=ERROR_SUCCESS)
	  if(RegCreateKey(HKEY_LOCAL_MACHINE, L"Software\\Patch Builder",&regKey))
		  return PostMessage(WM_QUIT);

  //Sets the current version
  if(RegSetValueEx(regKey,key,0,REG_SZ,(UCHAR*)value.GetString(),(DWORD)value.GetLength()+1)!=ERROR_SUCCESS)
    return PostMessage(WM_QUIT);

  //Closes the key
  RegCloseKey(regKey);

  //Returns success
  return TRUE;
}
