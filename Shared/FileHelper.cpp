#include "stdafx.h"
#include "FileHelper.h"

CFileHelper::CFileHelper()
{
}

CFileHelper::~CFileHelper()
{
}

/******************************************************************************
Checks if a file exists
******************************************************************************/
bool CFileHelper::Exists(CString path)
{
	WIN32_FIND_DATA wfd;

	HANDLE hFind = FindFirstFile(path.GetString(), &wfd);

	if(hFind == INVALID_HANDLE_VALUE)
		return false;

	FindClose(hFind);
	return true;
}
/******************************************************************************
Delete a file w/ error checking
******************************************************************************/
bool CFileHelper::Delete(CString path)
{
	if(Exists(path))
	{
		SetFileNormal(path);

		if(!DeleteFile(path.GetString()))
		{
			CLog::Instance()->AddLog("(Delete File) DeleteFile Error: %d File: %s", GetLastError(), path);
			return false;
		}
	}

	return true;
}

bool CFileHelper::Move(CString dest, CString src)
{
	if(MoveFile(src, dest) == FALSE)
	{
		CLog::Instance()->AddLog("(Move File) MoveFile Error: %d Source: %s  Dest: %s", GetLastError(), src, dest);
		return false;
	}

	return true;
}

bool CFileHelper::Copy(CString dest, CString src)
{
	if(CopyFile(src, dest, false) == FALSE)
	{
		CLog::Instance()->AddLog("(Copy File) CopyFile Error: %d Source: %s  Dest: %s", GetLastError(), src, dest);
		return false;
	}

	return true;
}

bool CFileHelper::SetFileNormal(CString path)
{
	if(Exists(path) && !SetFileAttributes(path.GetString(), FILE_ATTRIBUTE_NORMAL))
	{
		CLog::Instance()->AddLog("(Set File Normal) SetFileAttributes Error: %d File: %s", GetLastError(), path);
		return false;
	}

	return true;
}

void CFileHelper::DirList(CArray<CFileObj> &arr, CString path, bool recursive)
{
	//Makes a list of files in directory
	CFileFind exFile;
	CString findPath = path;
	AddSlash(findPath);
	findPath.AppendFormat("*.*");
	BOOL bContinue = exFile.FindFile(findPath);

	while(bContinue)
	{
		bContinue = exFile.FindNextFile();

		if(!exFile.IsDirectory() && !exFile.IsDots())
		{
			CFileObj temp;
			temp.FileName = exFile.GetFileName();
			temp.FilePath = exFile.GetFilePath();
			temp.RootDir = exFile.GetRoot();
			temp.Length = (unsigned long int)exFile.GetLength();
			temp.FileTitle = exFile.GetFileTitle();
			temp.FileURL = exFile.GetFileURL();

			temp.FileExt = GetExt(temp.FilePath);
			temp.Checksum = GetChecksum(temp.FilePath);

			temp.IsNormal = exFile.IsNormal();
			temp.IsReadOnly = exFile.IsReadOnly();
			arr.Add(temp);
		}
		else if(recursive && exFile.IsDirectory() && !exFile.IsDots())
		{
			DirList(arr, exFile.GetFilePath(), recursive);
		}
	}

}

CString CFileHelper::GetChecksum(CString path)
{
	CString md5Str;
	CFile md5File;

	//Gets the checksum
	if(md5File.Open(path, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
	{
		md5Str = CMD5Checksum::GetMD5(md5File);
		md5File.Close();
	}
	else
	{
		CLog::Instance()->AddLog("(Get Checksum) Error Determining MD5 Checksum for: %s", path);
		md5Str.Empty();
	}

	return md5Str;
}

void CFileHelper::CompareFileLists(CArray<CCmpObj> &result, CArray<CFileObj> &target, CArray<CFileObj> &test)
{
	CCmpObj tResult;
	CFileObj tTarget;
	CFileObj tTest;

	//Iterate through files in test list
	for(int i = (int)test.GetUpperBound(); i >= 0; i--)
	{
		bool bExists = false; //Determine if test file exists in target list
		tTest = test.GetAt(i);

		//Cycle through target list
		for(int j = (int)target.GetUpperBound(); (j >= 0) && !bExists; j--)
		{
			tTarget = target.GetAt(j);
			//If target and test files match
			if(tTest.FileName == tTarget.FileName)
			{
				bExists = true; //Test file exists on target list
				//If checksum doesn't = NULL and Target checksum doesn't match Test
				if((tTest.Checksum != tTarget.Checksum) && (tTest.Checksum != CString("NULL")))
				{
					//Show that file was modified
					tResult.FileObj = tTest;
					tResult.FileState = CCmpObj::MODIFIED;
					result.Add(tResult);
				}				
				//Remove from test list
				target.RemoveAt(j);
			}
		}
		//If file didn't exist
		if(!bExists && (tTest.Checksum != CString("NULL")))
		{
			//Show that the file was missing
			tResult.FileObj = tTest;
			tResult.FileState = CCmpObj::MISSING;
			result.Add(tResult);
		}
	}

	//Iterate through remaining Target files
	for(int i = 0; i < target.GetSize(); i++)
	{
		//Show that any extra files were not in the test set
		tTarget = target.GetAt(i);
		tResult.FileObj = tTarget;
		tResult.FileState = CCmpObj::EXTRA;
		result.Add(tResult);
	}
}

void CFileHelper::AddSlash(CString &dir)
{
	if(dir.GetAt(dir.GetLength() - 1) != '\\')
		dir.AppendChar('\\');
}

CString CFileHelper::GetParentDir(CString path)
{
	if(path.GetAt(path.GetLength() - 1) == '\\')
		path.Truncate(path.GetLength() - 2);

	int length = path.ReverseFind('\\');
	if(length >= 0)
		path.Truncate(length);

	AddSlash(path);
	return path;
}

CString CFileHelper::GetExt(CString path)
{
	return path.Mid(path.ReverseFind('.') + 1);
}