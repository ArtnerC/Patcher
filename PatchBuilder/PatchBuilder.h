#pragma once

/******************************************************************************
Includes for PatchBuilder
******************************************************************************/
#include <afxtempl.h>
#include "Config.h"
#include "PBDlg.h"
#include "resource.h"
#include "MD5Checksum.h"
#include "apatch.h"
#pragma comment(lib, "apatch.lib")

/******************************************************************************
Enum for PatchBuilder
******************************************************************************/
enum PatchType { NORMAL, JUMP };

/******************************************************************************
Class for PatchBuilder
******************************************************************************/
class PatchBuilder : public CWinApp
{
private:
	struct PatchSettings
	{
		PatchType Type;
		CString OutputPath;
		CString OutputFolder;
		CString UpdaterDir;
		CString UpdaterAppliedVer;
		int Options;
		CString OldVer;
		CString CurrentVer;
		CString OldVerDir;
		CString CurrentVerDir;
		CString FilePatchDir;
		CString PatchPath;
		CString PatchesDir;
		CString InstallerDir;
		CString InstallerVer;
		int MinorPatch;
	} Settings;
	struct FileNode
	{
		CString filePath;
		CString fileName;
	};

	CWinThread *bThread;
	//PatchSettings Settings;

public:
	PatchBuilder();
	virtual BOOL InitInstance();
	void BuildPatch();
	void BuildJump();
	void BuildPatchFile();
	void DirSearch(CString path, CString basePath, CArray<FileNode> *list);
	CString getMD5(CString file);
	void MakePatchFile(CString OldPath, CString NewPath, CString PatchPath);
	bool GetSettings();
	bool FExists(CString path);

protected:
	void BuildProc();
	void BuildFile();
	friend UINT PatchBuilderBuild(PatchBuilder *cpa);
	friend UINT PatchBuilderFileBuild(PatchBuilder *cpa);
	friend UINT PatchBuilderBuildJump(PatchBuilder *cpa);
};

/******************************************************************************
Globals
******************************************************************************/
extern PatchBuilder patchbuilder;