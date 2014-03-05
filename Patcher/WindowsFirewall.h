#pragma once

/******************************************************************************
Includes
******************************************************************************/
#include <crtdbg.h>
#include <netfw.h>
#include <objbase.h>
#include <oleauto.h>
//#include <stdio.h>
#include "Log.h"

/******************************************************************************
Libraries
******************************************************************************/
//#pragma comment(lib, "name.lib")

/******************************************************************************
WindowsFirewall Class
******************************************************************************/
class CWindowsFirewall
{
public:
	CWindowsFirewall();
	~CWindowsFirewall();

	bool IsOn();
	bool IsAppEnabled(CString path);
	void AddApp(CString path, CString name);

	HRESULT WindowsFirewallInit();
	void WindowsFirewallCleanup();
	HRESULT WindowsFirewallIsOn(OUT BOOL* fwOn);
	HRESULT WindowsFirewallAppIsEnabled(const CString fwProcessImageFileName, OUT BOOL* fwAppEnabled);
	HRESULT WindowsFirewallAddApp(const CString fwProcessImageFileName, const CString fwName);

private:
	//Firewall Profile
	INetFwProfile* m_fwProfile;

	HRESULT m_comInit;
};