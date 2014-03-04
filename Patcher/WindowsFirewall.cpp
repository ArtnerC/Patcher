/******************************************************************************
Includes
******************************************************************************/
#include "stdafx.h"
#include "WindowsFirewall.h"

/******************************************************************************
CPatcher Constructor
******************************************************************************/
CWindowsFirewall::CWindowsFirewall() : m_comInit(NULL), m_fwProfile(NULL)
{
	HRESULT hr = S_OK;

	//Initialize COM.
	m_comInit = CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	//Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
	//initialized with a different mode. Since we dont care what the mode is,
	//we'll just use the existing mode.
	if(m_comInit != RPC_E_CHANGED_MODE)
	{
		hr = m_comInit;
		if(FAILED(hr))
		{
			CLog::Instance()->AddLog("CoInitializeEx failed: 0x%08lx\n", hr);
			throw;
		}
	}

	//Retrieve the firewall profile currently in effect.
	hr = WindowsFirewallInit();
	//hr = WindowsFirewallInitialize();
	if(FAILED(hr))
	{
		CLog::Instance()->AddLog("WindowsFirewallInitialize failed: 0x%08lx\n", hr);
		throw;
	}
}

/******************************************************************************
CPatcher Destructor
******************************************************************************/
CWindowsFirewall::~CWindowsFirewall()
{
	//Release the firewall profile.
	WindowsFirewallCleanup();

	//Uninitialize COM.
	if(SUCCEEDED(m_comInit))
		CoUninitialize();
}

bool CWindowsFirewall::IsOn()
{
	BOOL fwOn;
	HRESULT hr = WindowsFirewallIsOn(&fwOn);
	if(FAILED(hr))
		throw "Check Failed";

	if(fwOn == TRUE)
		return true;
	else
		return false;
}

bool CWindowsFirewall::IsAppEnabled(CString path)
{
	BOOL fwAppEnabled;
	HRESULT hr = WindowsFirewallAppIsEnabled(path.GetString(), &fwAppEnabled);
	if(FAILED(hr))
		throw "Check Failed";

	if(fwAppEnabled == TRUE)
		return true;
	else
		return false;
}

void CWindowsFirewall::AddApp(CString path, CString name)
{
	//Add application to the authorized application collection.
	HRESULT hr = WindowsFirewallAddApp(path.GetString(), name.GetString());
	if(FAILED(hr))
	{
		CLog::Instance()->AddLog("WindowsFirewallAddApp failed: 0x08lx\n", hr);
		throw;
	}
}

HRESULT CWindowsFirewall::WindowsFirewallInit()
{
	HRESULT hr = S_OK;
	INetFwMgr* fwMgr = NULL;
	INetFwPolicy* fwPolicy = NULL;

	m_fwProfile = NULL;

	//Create an instance of the firewall settings manager
	hr = CoCreateInstance(__uuidof(NetFwMgr), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwMgr), (void**)&fwMgr);
	if(FAILED(hr))
	{
		CLog::Instance()->AddLog("CoCreateInstance failed: 0x%08lx\n", hr);
		throw;
	}

	//Retrieve the local firewall policy.
	hr = fwMgr->get_LocalPolicy(&fwPolicy);
	if(FAILED(hr))
	{
		CLog::Instance()->AddLog("get_LocalPolicy failed: 0x%08lx\n", hr);
		throw;
	}

	//Retrieve the firewall profile currently in effect.
	hr = fwPolicy->get_CurrentProfile(&m_fwProfile);
	if(FAILED(hr))
	{
		CLog::Instance()->AddLog("get_CurrentProfile failed: 0x08lx\n", hr);
		throw;
	}
		
	//Release the local firewall policy
	if(fwPolicy != NULL)
		fwPolicy->Release();

	//Release the firewall settings manager
	if(fwMgr != NULL)
		fwMgr->Release();

	return hr;
}

void CWindowsFirewall::WindowsFirewallCleanup()
{

	//Release the firewall profile
	if(m_fwProfile != NULL)
	{
		m_fwProfile->Release();
	}
}

HRESULT CWindowsFirewall::WindowsFirewallIsOn(BOOL *fwOn)
{
	HRESULT hr = S_OK;
	VARIANT_BOOL fwEnabled;

	_ASSERT(fwOn != NULL);

	*fwOn = FALSE;

	//Get the current state of the firewall.
	hr = m_fwProfile->get_FirewallEnabled(&fwEnabled);
	if(FAILED(hr))
	{
		CLog::Instance()->AddLog("get_FirewallEnabled failed: 0x08lx\n", hr);
		throw;
	}

	//Check to see if the firewall is on.
	if(fwEnabled != VARIANT_FALSE)
	{
		*fwOn = TRUE;
	}

	return hr;
}

HRESULT CWindowsFirewall::WindowsFirewallAppIsEnabled(const char * fwProcessImageFileName, BOOL *fwAppEnabled)
{
	HRESULT hr = S_OK;
	BSTR fwBstrProcessImageFileName = NULL;
	VARIANT_BOOL fwEnabled;
	INetFwAuthorizedApplication* fwApp = NULL;
	INetFwAuthorizedApplications* fwApps = NULL;

	_ASSERT(fwProcessImageFileName != NULL);
	_ASSERT(fwAppEnabled != NULL);

	*fwAppEnabled = FALSE;

	//Retrieve the authorized application collection.
	hr = m_fwProfile->get_AuthorizedApplications(&fwApps);

	if(FAILED(hr))
	{
		CLog::Instance()->AddLog("get_AuthorizedApplications failed: 0x%08lx\n", hr);
		throw;
	}

	//Allocate a BSTR for the process image file name.
	fwBstrProcessImageFileName = SysAllocString((const OLECHAR *)fwProcessImageFileName);

	if(fwBstrProcessImageFileName == NULL)
	{
		hr = E_OUTOFMEMORY;
		CLog::Instance()->AddLog("SysAllocString failed: 0x%08lx\n", hr);
		throw;
	}

	//Attempt to retrieve the authorized application.
	hr = fwApps->Item(fwBstrProcessImageFileName, &fwApp);


	if(SUCCEEDED(hr))
	{
		//Find out if the authorized application is enabled.
		hr = fwApp->get_Enabled(&fwEnabled);
		if(FAILED(hr))
		{
			CLog::Instance()->AddLog("get_Enabled failed: 0x%08lx\n", hr);
			throw;
		}

		if(fwEnabled != VARIANT_FALSE)
		{
			//The authorized application is enabled.
			*fwAppEnabled = TRUE;
		}
	}
	else
	{
		//The authorized application was not in the collection.
		hr = S_OK;
	}

	//Free the BSTR.
	SysFreeString(fwBstrProcessImageFileName);

	//Release the authorized application instance.
	if(fwApp != NULL)
		fwApp->Release();

	//Release the authorized application collection.
	if(fwApps != NULL)
		fwApps->Release();

	return hr;
}

HRESULT CWindowsFirewall::WindowsFirewallAddApp(const char * fwProcessImageFileName, const char * fwName)
{
	HRESULT hr = S_OK;
	BOOL fwAppEnabled;
	BSTR fwBstrName = NULL;
	BSTR fwBstrProcessImageFileName = NULL;
	INetFwAuthorizedApplication* fwApp = NULL;
	INetFwAuthorizedApplications* fwApps = NULL;

	_ASSERT(fwProcessImageFileName != NULL);
	_ASSERT(fwName != NULL);

	//First check to see if the application is already authorized.
	hr = WindowsFirewallAppIsEnabled(fwProcessImageFileName, &fwAppEnabled);
	if(FAILED(hr))
	{
		CLog::Instance()->AddLog("WindowsFirewallAppIsEnabled failed: 0x%08lx\n", hr);
		throw;
	}

	//Only add the application if it isn't already authorized.
	if(!fwAppEnabled)
	{
		//Retrieve the authorized application collection.
		hr = m_fwProfile->get_AuthorizedApplications(&fwApps);
		if(FAILED(hr))
		{
			CLog::Instance()->AddLog("get_AuthorizedApplications failed: 0x%08lx\n", hr);
			throw;
		}

		//Create an instance of an authorized application.
		hr = CoCreateInstance(__uuidof(NetFwAuthorizedApplication), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwAuthorizedApplication), (void**)&fwApp);
		if(FAILED(hr))
		{
			CLog::Instance()->AddLog("CoCreateInstance failed: 0x%08lx\n", hr);
			throw;
		}

		//Allocate a BSTR for the process image file name.
		fwBstrProcessImageFileName = SysAllocString((const OLECHAR *)fwProcessImageFileName);
		if(fwBstrProcessImageFileName == NULL)
		{
			hr = E_OUTOFMEMORY;
			CLog::Instance()->AddLog("SysAllocString failed: 0x%08lx\n", hr);
			throw;
		}

		//Set the process image file name.
		hr = fwApp->put_ProcessImageFileName(fwBstrProcessImageFileName);
		if(FAILED(hr))
		{
			CLog::Instance()->AddLog("put_ProcessImageFileName failed: 0x%08lx\n", hr);
			throw;
		}

		//Allocate a BSTR for the application friendly name.
		fwBstrName = SysAllocString((const OLECHAR *)fwName);
		if(SysStringLen(fwBstrName) == 0)
		{
			hr = E_OUTOFMEMORY;
			CLog::Instance()->AddLog("SysAllocString failed: 0x%08lx\n", hr);
		}

		//Set the application friendly name.
		hr = fwApp->put_Name(fwBstrName);
		if(FAILED(hr))
		{
			CLog::Instance()->AddLog("put_Name failed: 0x%08lx\n", hr);
			throw;
		}

		//Add the application to the collection.
		hr = fwApps->Add(fwApp);
		if(FAILED(hr))
		{
			CLog::Instance()->AddLog("Add failed: 0x%08lx\n", hr);
			throw;
		}

		//Authorized application is now enabled in the firewall.
	}

	//Free the BSTRs.
	SysFreeString(fwBstrName);
	SysFreeString(fwBstrProcessImageFileName);

	//Release the authorized application instance.
	if(fwApp != NULL)
		fwApp->Release();

	//Release the authorized application collection.
	if(fwApps != NULL)
		fwApps->Release();

	return hr;
}