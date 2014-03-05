#pragma once

/******************************************************************************
CPatcherDlg Includes
******************************************************************************/
#include "Config.h"
#include "PatcherApp.h"
#include "TransStatic.h"
#include "SimpleBrowser.h"
#include "resource.h"

/******************************************************************************
CPatcherDlg
******************************************************************************/
class CPatcherDlg : public CDialog
{
public:
	CPatcherDlg(CString windowTitle);	// standard constructor

	//enum { IDD = IDD_PATCHER_DIALOG };

	void SetBrowser(CString path); //Navigate browser to page
	void SetAction(const CString fmt, ...); //Set Action Label
	void SetStatus(const CString fmt, ...); //Set Status Label
	void SetProgress1(int fDown, int fSize); //Set Minor Progress Bar
	void SetProgress2(int fCmds, int fDone); //Set Major Progress Bar
	void SetProgress3(int fPercent); //Set Patch Progress Bar
	void SetVersion(int remoteVersion, int localVersion); //Set Version Difference Label
	CString SizeToString(int size);

protected:
	virtual void DoDataExchange(CDataExchange* pDX); //Handle Data Exchange

	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog(); //Dialog Initialization
	afx_msg void OnPaint(); //Draws Minimize Button
	afx_msg HCURSOR OnQueryDragIcon(); //Cursor on Window Drag
	DECLARE_MESSAGE_MAP()

	//virtual void OnCancel(); //Handles "Esc" Key

public:
	afx_msg void OnClickedButtonLaunch(); //Handles Launch Button Click
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor); //Label Transparency and Text Color
	afx_msg void OnClose(); //Handles Dialog Close

private:
	CString m_WindowTitle;

	SimpleBrowser m_Browser; //HTML Browser control
	CProgressCtrl m_Progress1; //Minor Progress Bar
	CProgressCtrl m_Progress2; //Major Progress Bar
	CTransStatic m_StaticAction; //Current Action Label
	CTransStatic m_StaticStatus; //Current Status Label
	CTransStatic m_StaticVersions; //Version Difference Label
	CTransStatic m_StaticDownloading; //Download progress
	CTransStatic m_StaticPatching; //Patch progress
};