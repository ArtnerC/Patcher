#pragma once

/******************************************************************************
Simple Logger class based on the singleton pattern.
******************************************************************************/
class CLog
{
public:
	static CLog* Instance();
	static void Dispose();

	void AddLog(const char * fmt, ...);
	void SetDirectory(const CString dir = "");

protected:

private:
	CLog();
	~CLog();

	CString m_Directory;
	CString m_FullPath;

	static CLog *m_Instance;
};