/******************************************************************************
PatcherDlg.cpp : implementation file
******************************************************************************/
#include "stdafx.h"
#include "PatcherDlg.h"

/******************************************************************************
CPatcherDlg dialog
******************************************************************************/
CPatcherDlg::CPatcherDlg(CString windowTitle) : CDialog()
{
	m_WindowTitle = windowTitle;

	//Load icon
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON);
}

/******************************************************************************
Handle Data Exchange
******************************************************************************/
void CPatcherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_BROWSER, m_Browser);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress1);
	DDX_Control(pDX, IDC_PROGRESS2, m_Progress2);
	DDX_Control(pDX, IDC_STATIC_ACTION, m_StaticAction);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_StaticStatus);
	DDX_Control(pDX, IDC_STATIC_VERSIONS, m_StaticVersions);
	DDX_Control(pDX, IDC_STATIC_DOWNLOADING, m_StaticDownloading);
	DDX_Control(pDX, IDC_STATIC_PATCHING, m_StaticPatching);
}

/******************************************************************************
CPatcherDlg Message Map
******************************************************************************/
BEGIN_MESSAGE_MAP(CPatcherDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_LAUNCH, OnClickedButtonLaunch)
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


/******************************************************************************
Initializes Dialog
******************************************************************************/
BOOL CPatcherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//Set the icon for this dialog.
	SetIcon(m_hIcon, TRUE); // Set big icon
	SetIcon(m_hIcon, FALSE); // Set small icon

	//Set dialog title bar
	//SetWindowText(CConfig::APP_TITLE);
	SetWindowText(m_WindowTitle);

	//Create browser and load default page
	m_Browser.CreateFromControl(this, IDC_BROWSER);
	m_Browser.PutSilent(true);
	SetBrowser(L"");

	//Setup Progress Bars
	m_Progress1.SetBkColor(RGB(0, 0, 0));
	m_Progress2.SetBkColor(RGB(0, 0, 0));
	m_Progress1.SetRange(0, 100);
	m_Progress2.SetRange(0, 100);
	m_Progress1.SetPos(0);
	m_Progress2.SetPos(0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/******************************************************************************
Handle Paint Message

If you add a minimize button to your dialog, you will need the code below to
draw the icon.  For MFC applications using the document/view model, this is
automatically done for you by the framework.
******************************************************************************/
void CPatcherDlg::OnPaint() 
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
The system calls this function to obtain the cursor to display while the user
drags the minimized window.
******************************************************************************/
HCURSOR CPatcherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/******************************************************************************
Handle Launch Button Clicked
******************************************************************************/
void CPatcherDlg::OnClickedButtonLaunch()
{
	theApp.LaunchApp();
	DestroyWindow();
}

/******************************************************************************
Handle CtlColor (Set static control backgrounds transparent)
******************************************************************************/
HBRUSH CPatcherDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(nCtlColor == CTLCOLOR_STATIC)
	{
		switch(pWnd->GetDlgCtrlID())
		{
		case IDC_STATIC_ACTION:
		case IDC_STATIC_STATUS:
		case IDC_STATIC_VERSIONS:
		case IDC_STATIC_DOWNLOADING:
		case IDC_STATIC_PATCHING:
			pDC->SetTextColor(RGB(255, 255, 255));
			break;
		default:
			break;
		}
	}

	return hbr;
}

/******************************************************************************
Handle dialog closing (close application)
******************************************************************************/
void CPatcherDlg::OnClose()
{
	theApp.ClosePatcher();

	CDialog::OnClose();
}

/******************************************************************************
Handle dialog cancel - caused by pressing ESC
******************************************************************************/
//void CPatcherDlg::OnCancel()
//{
//	theApp.ClosePatcher();
//
//	CDialog::OnCancel();
//}
/******************************************************************************
Set browser navigation URL
******************************************************************************/
void CPatcherDlg::SetBrowser(CString path)
{
	m_Browser.Stop();
	m_Browser.Clear();

	if(!path.IsEmpty() && CFileHelper::Exists(path))
		m_Browser.Navigate(path);
	else
		m_Browser.Write(L"<!DOCTYPE html><html><body bgcolor=#000000><font color=#00ff00 face=\"Bauhaus 93\" size=5><center>Loading...</center></font></body></html>");
}
/******************************************************************************
Set action label
******************************************************************************/
void CPatcherDlg::SetAction(const CString fmt, ...)
{
	CString temp;
	va_list va_alist;
	temp.Empty();
	va_start(va_alist, fmt);
	temp.FormatV(fmt, va_alist);
	va_end(va_alist);

	SetDlgItemText(IDC_STATIC_ACTION, temp);
}

/******************************************************************************
Set status label
******************************************************************************/
void CPatcherDlg::SetStatus(const CString fmt, ...)
{
	CString temp;
	va_list va_alist;	
	temp.Empty();
	va_start(va_alist, fmt);
	temp.FormatV(fmt, va_alist);
	va_end(va_alist);

	SetDlgItemText(IDC_STATIC_STATUS, temp);
}

/******************************************************************************
Set Minor Progress Bar
******************************************************************************/
void CPatcherDlg::SetProgress1(int fDown, int fSize)
{
	CString dlStr;

	if(fSize)
	{
		int iPercent = (int)((((float)fDown / fSize) * 100));
		m_Progress1.SetPos(iPercent);

		dlStr.Format(L"Downloading: %s of %s", SizeToString(fDown), SizeToString(fSize));

		//if((fSize > 0) && (fSize < 1024))
		//	dlStr.AppendFormat(L"%d B of %d B", fDown, fSize);
		//else if((fSize >= 1024) && (fSize < (1024 * 1024)))
		//	dlStr.AppendFormat(L"%d kB of %d kB", fDown / 1024, fSize / 1024);
		//else if(fSize >= (1024 * 1024))
		//{
		//	if(fDown < (1024 * 1024))
		//		dlStr.AppendFormat(L"%d kB of %.2f MB", fDown / 1024, (float)fSize / (1024 * 1024));
		//	else
		//		dlStr.AppendFormat(L"%.2f MB of %.2f MB", (float)fDown / (1024 * 1024), (float)fSize / (1024 * 1024));
		//}
	}
	else if(fSize <= 0)
	{
		m_Progress1.SetPos(0);
		dlStr.Empty();
	}

	SetDlgItemText(IDC_STATIC_DOWNLOADING, dlStr);
}

CString CPatcherDlg::SizeToString(int size)
{
	CString ret;

	if(size >= 1048576) //Greater than a Million
		ret.Format(L"%.2f MB", (float)size / 1048576);
	else if(size >= 1024) //Greater than a Thousand
		ret.Format(L"%d kB", size / 1024);
	else if(size >= 0) //Zero or Positive
		ret.Format(L"%d B", size);

	return ret;
}

/******************************************************************************
Set Major Progress Bar
******************************************************************************/
void CPatcherDlg::SetProgress2(int fCmds, int fDone)
{
	if(fCmds)
	{
		int iPercent = (int)(((float)fDone / fCmds) * 100);
		m_Progress2.SetPos(iPercent);
	}
}

void CPatcherDlg::SetProgress3(int fPercent)
{
	CString temp;
	if(fPercent)
		temp.Format(L"Patching: %d%%", fPercent);

	SetDlgItemText(IDC_STATIC_PATCHING, temp);
}

/******************************************************************************
Set Version Difference label
******************************************************************************/
void CPatcherDlg::SetVersion(int remoteVersion, int localVersion)
{
	int vDiff = remoteVersion - localVersion;
	CString verDiff;
	if(vDiff > 0)
		verDiff.Format(L"%d Versions Out-of-Date", vDiff);
	else if(vDiff == 0)
		verDiff.Format(L"Versions Match");
	else if(vDiff < 0)
		verDiff.Format(L"Invalid Versions");

	SetDlgItemText(IDC_STATIC_VERSIONS, verDiff);
}