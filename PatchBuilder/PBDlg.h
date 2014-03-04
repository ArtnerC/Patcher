#pragma once

/******************************************************************************
Includes for PBDlg
******************************************************************************/
#include "PatchBuilder.h"
#include "resource.h"
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi")

/******************************************************************************
Class for PBDlg
******************************************************************************/
class PBDlg : public CDialog
{
private:
	HICON m_hIcon;

public:
	PBDlg(CWnd* pParent = NULL);
	enum { IDD = IDD_PATCHBUILDER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

public:
	CString BrowseFolder(CString title);
	CString GetRegValue(CString key, CString def);
	BOOL SetRegValue(CString key, CString value);
	afx_msg void OnBnClickedButtonBuild();
	afx_msg void OnBnClickedButtonBuildUpdater();
	afx_msg void OnOutputBrowse();
	afx_msg void OnUpdaterDirectoryBrowse();
	afx_msg void OnOldDirectoryBrowse();
	afx_msg void OnCurrentDirectoryBrowse();
	afx_msg void OnClickedDefaultVersion();
	afx_msg void OnClickedDefaultPath();
	afx_msg void OnChangeCurrentVersion();
	afx_msg void OnClose();
	afx_msg void OnChangeOldVersion();
	afx_msg void OnInstallerDirectoryBrowse();
	afx_msg void OnBnClickedButtonBuildJump();
};