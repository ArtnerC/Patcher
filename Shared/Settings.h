#pragma once

#include "stdafx.h"
#include <afxtempl.h>

#include "Log.h"

class CSettings
{
public:
	CSettings(CString registrysubkey = "");
	~CSettings();
	CSettings(const CSettings &csettings);
	void operator = (const CSettings &csettings);

	void SetStr(CString name, CString value, bool inregistry = false);
	void SetInt(CString name, int value, bool inregistry = false);

	CString GetStr(CString name, bool inregistry = false);
	int GetInt(CString name, bool inregistry = false);

protected:
private:

	void SetToRegistry(CString name, CString value);
	CString GetFromRegistry(CString name);

	struct SettingPair
	{
		CString Name;
		CString Value;
	};

	CArray<SettingPair> m_Settings;

	bool m_UseRegistry;
	CString m_RegistrySubKey;
};