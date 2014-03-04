#pragma once

#include <afxinet.h>
#include "PatcherDlg.h"
#include "FileHelper.h"
#include "Log.h"

class CPatcherDlg;

class CDownloader
{
public:
	CDownloader(CPatcherDlg* wnd, CString url, CString root = "");
	~CDownloader();

	bool Connect();
	bool Connect(CString name, INTERNET_PORT port = 80);
	void Disconnect();
	bool IsConnected();
	bool Download(CString path, CFile &dest);
	bool DownloadToFile(CString path, CString file);
	bool DownloadToMem(CString path, CString &str);

protected:
private:
	//Window
	CPatcherDlg *m_wnd;

	//Connection
	CInternetSession m_Session;
	CHttpConnection *m_Connection;

	CString m_url;
	CString m_root;

	bool m_Connected;
};