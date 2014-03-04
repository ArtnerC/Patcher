#pragma once

#include "stdafx.h"
#include <afxtempl.h>
#include "Log.h"
#include "MD5Checksum.h"

class CFileHelper
{
public:
	struct CFileObj
	{
		CString FileName;
		CString FilePath;
		CString RootDir;
		unsigned long int Length;
		CString FileTitle;
		CString FileURL;
		CString FileExt;

		CString Checksum;

		BOOL IsNormal;
		BOOL IsReadOnly;
	};

	struct CCmpObj
	{
		enum state {MODIFIED, MISSING, EXTRA};
		state FileState;
		CFileObj FileObj;
	};

	CFileHelper();
	~CFileHelper();

	static bool Exists(CString path);
	static bool Delete(CString path);
	static bool Move(CString dest, CString src);
	static bool Copy(CString dest, CString src);
	static bool SetFileNormal(CString path);
	static void DirList(CArray<CFileObj> &arr, CString path, bool recursive = false);
	static CString GetChecksum(CString path);
	static void CompareFileLists(CArray<CCmpObj> &result, CArray<CFileObj> &target, CArray<CFileObj> &test);
	static void AddSlash(CString &dir);
	static CString GetParentDir(CString path);
	static CString GetExt(CString path);

protected:
private:
};