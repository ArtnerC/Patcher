#pragma once

#include "Command.h"
#include "Patcher.h"

class CPatcher;

class PatcherCommand : public Command
{
public:
	PatcherCommand(CPatcher *obj, bool(CPatcher:: *meth)(CPatcher::ArgList *args), CPatcher::ArgList *args)
	{
		_object = obj;
		_method = meth;
		_args = args;
	}
	~PatcherCommand()
	{
		delete _args;
	}
	bool Execute()
	{
		return (_object->* _method)(_args);
	}

	static Command* MakeCommand(CPatcher *patcher, CString cmd, CPatcher::ArgList *args)
	{
		if(cmd == "patchfile")
			return new PatcherCommand(patcher, &CPatcher::cmdPatchFile, args);
		else if(cmd == "setverif")
			return new PatcherCommand(patcher, &CPatcher::cmdSetVerIf, args);
		else if(cmd == "addfile")
			return new PatcherCommand(patcher, &CPatcher::cmdAddFile, args);
		else if(cmd == "delfile")
			return new PatcherCommand(patcher, &CPatcher::cmdDelFile, args);
		else if(cmd == "regedit")
			return new PatcherCommand(patcher, &CPatcher::cmdRegEdit, args);
		else if(cmd == "setversion")
			return new PatcherCommand(patcher, &CPatcher::cmdSetVersion, args);
		else if(cmd == "remoteversion")
			return new PatcherCommand(patcher, &CPatcher::cmdRemoteVersion, args);
		else if(cmd == "uddfile")
			return new PatcherCommand(patcher, &CPatcher::cmdUDDFile, args);
		else if(cmd == "msgbox")
			return new PatcherCommand(patcher, &CPatcher::cmdMsgBox, args);
		else if(cmd == "AddLog")
			return new PatcherCommand(patcher, &CPatcher::cmdAddLog, args);
		else if(cmd == "cmdfile")
			return new PatcherCommand(patcher, &CPatcher::cmdCmdFile, args);
		else if(cmd == "checkupdatedir")
			return new PatcherCommand(patcher, &CPatcher::cmdCheckUpdateDir, args);
		else if(cmd == "launchapp")
			return new PatcherCommand(patcher, &CPatcher::cmdLaunchApp, args);
		else
		{
			args->SetAt("cmd", cmd);
			return new PatcherCommand(patcher, &CPatcher::cmdUnknown, args);
		}
	}

private:
	CPatcher *_object;
	bool(CPatcher:: *_method)(CPatcher::ArgList *args);
	CPatcher::ArgList *_args;
};