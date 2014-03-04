#include "stdafx.h"
#include "Log.h"

CLog *CLog::m_Instance = 0;

/******************************************************************************
Create a new log
******************************************************************************/
CLog::CLog()
{
	SetDirectory();
}

/******************************************************************************
Delete log
******************************************************************************/
CLog::~CLog()
{
	if(this == m_Instance)
		m_Instance = 0;
}

/******************************************************************************
Adds a Log Entry to the log file
******************************************************************************/
void CLog::AddLog(const char * fmt, ...)
{
	CString temp;
	va_list va_alist;
	struct tm* current_tm = new tm();
	time_t current_time;

	time(&current_time);
	localtime_s(current_tm, &current_time);
	temp.Format("%02d:%02d:%02d-> ", current_tm->tm_hour, current_tm->tm_min, current_tm->tm_sec);

	va_start(va_alist, fmt);
	temp.AppendFormatV(fmt, va_alist);
	va_end(va_alist);
	temp.AppendFormat("\r\n");

	CFile logFile(m_FullPath, CFile::modeCreate | CFile::modeWrite);
	logFile.SeekToEnd();
	logFile.Write(temp, temp.GetLength());
	logFile.Close();
}

/******************************************************************************
Set location and name of log file
******************************************************************************/
void CLog::SetDirectory(const CString dir)
{
	m_Directory = dir;
	m_FullPath.Format("%sUpdater Log.txt", m_Directory);
}

/******************************************************************************
Get the log class instance or create one
******************************************************************************/
CLog* CLog::Instance()
{
	if(!m_Instance)
	{
		m_Instance = new CLog();
	}
	return m_Instance;
}

/******************************************************************************
Delete the singleton log object if it exists
******************************************************************************/
void CLog::Dispose()
{
	delete m_Instance;
}