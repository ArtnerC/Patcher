#include "stdafx.h"
#include "Downloader.h"

CDownloader::CDownloader(CPatcherDlg* wnd, CString url, CString root) : m_Session(CConfig::NET_SESSION), m_Connected(false)
{
	m_wnd = wnd;
	m_url = url;
	m_root = root;
}

CDownloader::~CDownloader()
{
	Disconnect();
}

bool CDownloader::Connect()
{
	return Connect(m_url);
}

/******************************************************************************
Connects to the patching server
******************************************************************************/
bool CDownloader::Connect(CString name, INTERNET_PORT port)
{
	//Updates the status
	m_wnd->SetStatus(L"Connecting to %s", name);

	//Creates the connection
	try
	{
		m_Connection = m_Session.GetHttpConnection(name, port);
		m_Connected = true;
	}
	catch(CInternetException *exc)
	{
		wchar_t error[128];
		exc->GetErrorMessage(error, 128);
		//error[strlen(error) - 2] = 0;
		CLog::Instance()->AddLog(L"(Connect) Exception: <%d> %s - Server: %s:%d", exc->m_dwError, error, name, port);
		m_Connected = false;
	}

	//Returns connection status
	return m_Connected;
}

bool CDownloader::Download(CString path, CFile &dest)
{
	if(!m_Connected)
		return false;

	//Updates the status
	m_wnd->SetStatus(L"Downloading %s", path);
	
	CHttpFile *httpFile;

	//Catch download errors
	try
	{
		//Requests the specified file
		httpFile = m_Connection->OpenRequest(L"GET", m_root + path, CConfig::PRIMARY_UPDATER, 1, NULL, NULL, INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);

		//Send the request
		httpFile->SendRequest();

		DWORD dwRet;
		//Validates the connection
		if(!httpFile->QueryInfoStatusCode(dwRet))
		{
			CLog::Instance()->AddLog(L"(Download) QueryInfoStatusCode Error: %d - File: %s", GetLastError(), path.GetString());
			delete httpFile;
			return false;
		}

		//Checks the status
		if(dwRet != HTTP_STATUS_OK)
		{
			CLog::Instance()->AddLog(L"(Download) InfoStatusCode not HTTP_STATUS_OK: %d - File: %s", dwRet, path.GetString());
			delete httpFile;
			return false;
		}

		//Gets the file length (size in bytes)
		CString fileLength;
		httpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, fileLength);

		//Storage for download progress
		unsigned long int fileSize = _wtol(fileLength);
		unsigned long int fileDownloaded = 0;

		//Some files don't have sizes in the header (normally very small files)
		if(!fileSize)
			fileSize = static_cast<unsigned long int>(httpFile->GetLength());

		//Update dialog progress bar
		m_wnd->SetProgress1(fileDownloaded, fileSize);

		//Downloads the file
		char buff[32*1024];
		unsigned int buffLength;
		
		buffLength = httpFile->Read(buff, sizeof(buff));
		while(buffLength > 0)
		{
			//Stores the downloaded data
			dest.Write(buff, buffLength);

			//Updates the bytes downloaded progress bar
			fileDownloaded += buffLength;
			m_wnd->SetProgress1(fileDownloaded, fileSize);

			//Download next chunk of file
			buffLength = httpFile->Read(buff, sizeof(buff));
		}

		//Cleans up
		delete httpFile;
	}
	catch(CInternetException *exc)
	{
		wchar_t error[128];
		exc->GetErrorMessage(error, 128);
		//error[strlen(error) - 2] = 0;
		CLog::Instance()->AddLog(L"(Download) Exception: <%d> %s - Server: %s - File: %s", exc->m_dwError, error, m_Connection->GetServerName(), path);

		delete httpFile;
		return false;
	}

	//Returns success
	return true;
}

/******************************************************************************
Downloads the specified file
******************************************************************************/
bool CDownloader::DownloadToFile(CString path, CString file)
{
	//Updates the status
	m_wnd->SetStatus(L"Downloading %s", path);

	//Relative Path to File
	CString relPath;
	relPath.Format(L"%s%s", m_root, path);

	CHttpFile *httpFile;

	//Catch download errors
	try
	{
		//Requests the specified file
		httpFile = m_Connection->OpenRequest(L"GET", relPath, L"Updater.exe", 1, NULL, NULL, INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);

		//Send the request
		httpFile->SendRequest();

		//Validates the connection
		DWORD dwRet;
		if(!httpFile->QueryInfoStatusCode(dwRet))
		{
			CLog::Instance()->AddLog(L"(Download) QueryInfoStatusCode Error: %d - File: %s", GetLastError(), path.GetString());
			return false;
		}

		//Checks the status
		if(dwRet != HTTP_STATUS_OK)
		{
			CLog::Instance()->AddLog(L"(Download) InfoStatusCode not HTTP_STATUS_OK: %d - File: %s", dwRet, path.GetString());
			return false;
		}

		//Gets file length(size in bytes)
		CString fLength;
		httpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, fLength);		

		//Store file progress
		unsigned long int fileSize = _wtol(fLength);
		unsigned long int fileDown = 0;

		//Some files don't have sizes in the header (normally small files)
		if(!fileSize)
			fileSize = (long int)httpFile->GetLength();

		//Update dialog
		m_wnd->SetProgress1(fileDown, fileSize);

		//Get target ready for data
		CString tempPath;
		tempPath.Format(L"%s.tmp", file);

		//Delete destination file if it exists
		CFileHelper::Delete(tempPath);

		CFile cFile;
		if(!cFile.Open(tempPath, CFile::modeCreate | CFile::modeWrite))
		{
			CLog::Instance()->AddLog(L"(Download) Open/Create File Failed: %s", tempPath);
			return false;
		}

		//Downloads the file
		char buff[32*1024];
		unsigned int buffLength;
		buffLength = httpFile->Read(buff, 32*1024);

		while(buffLength > 0)
		{
			//Stores the downloaded data
			cFile.Write(buff, buffLength);

			//Updates the bytes downloaded
			fileDown += buffLength;
			m_wnd->SetProgress1(fileDown, fileSize);

			buffLength = httpFile->Read(buff, 32*1024);
		}

		//Close output file
		cFile.Close();

		//Delete original file
		CFileHelper::Delete(file);

		//Rename temp file
		MoveFile(tempPath, file);

		//Cleans up
		delete httpFile;
	}
	catch(CInternetException *exc)
	{
		wchar_t error[128];
		exc->GetErrorMessage(error, 128);
		//error[strlen(error) - 2] = 0;
		CLog::Instance()->AddLog(L"(Download) Exception: <%d> %s - Server: %s - File: %s", exc->m_dwError, error, m_Connection->GetServerName(), path);

		delete httpFile;

		return false;
	}

	//Returns success
	return true;
}

bool CDownloader::DownloadToMem(CString path, CString &str)
{
	//Updates the status
	m_wnd->SetStatus(L"Downloading %s", path);

	//Relative Path to File
	CString relPath;
	relPath.Format(L"%s%s", m_root, path);

	CHttpFile *httpFile;

	//Catch download errors
	try
	{
		//Requests the specified file
		httpFile = m_Connection->OpenRequest(L"GET", relPath, L"Updater.exe", 1, NULL, NULL, INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);

		//Send the request
		httpFile->SendRequest();

		//Validates the connection
		DWORD dwRet;
		if(!httpFile->QueryInfoStatusCode(dwRet))
		{
			CLog::Instance()->AddLog(L"(Download) QueryInfoStatusCode Error: %d - File: %s", GetLastError(), path.GetString());
			return false;
		}

		//Checks the status
		if(dwRet != HTTP_STATUS_OK)
		{
			CLog::Instance()->AddLog(L"(Download) InfoStatusCode not HTTP_STATUS_OK: %d - File: %s", dwRet, path.GetString());
			return false;
		}

		//Gets file length(size in bytes)
		CString fLength;
		httpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, fLength);		

		//Store file progress
		unsigned long int fileSize = _wtol(fLength);
		unsigned long int fileDown = 0;

		//Some files don't have sizes in the header (normally small files)
		if(!fileSize)
			fileSize = (long int)httpFile->GetLength();

		//Update dialog
		m_wnd->SetProgress1(fileDown, fileSize);

		//Get target ready for data
		str.Empty();
		str.Preallocate(fileSize);

		//Downloads the file
		wchar_t buff[32*1024];
		unsigned int buffLength = httpFile->Read(buff, 32*1024);

		while(buffLength > 0)
		{
			//Stores the downloaded data
			str.Append(buff, buffLength);

			//Updates the bytes downloaded			
			fileDown += buffLength;			
			m_wnd->SetProgress1(fileDown, fileSize);

			buffLength = httpFile->Read(buff, 32*1024);
		}

		//Cleans up
		delete httpFile;
	}
	catch(CInternetException *exc)
	{
		wchar_t error[128];
		exc->GetErrorMessage(error, 128);
		//error[strlen(error) - 2] = 0;
		CLog::Instance()->AddLog(L"(Download) Exception: <%d> %s - Server: %s - File: %s", exc->m_dwError, error, m_Connection->GetServerName(), path.GetString());

		delete httpFile;

		return false;
	}

	//Returns success
	return true;
}

/******************************************************************************
Disconnects from the patching server
******************************************************************************/
void CDownloader::Disconnect()
{
	//Cleans up and disconnects
	delete m_Connection;
	m_Session.Close();

	m_Connected = false;
}

bool CDownloader::IsConnected()
{
	return m_Connected;
}