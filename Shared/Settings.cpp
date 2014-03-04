#include "stdafx.h"
#include "Settings.h"

CSettings::CSettings(CString registrysubkey) : m_UseRegistry(false), m_RegistrySubKey("")
{
	if(!registrysubkey.IsEmpty())
	{
		m_UseRegistry = true;
		m_RegistrySubKey = registrysubkey; //"Software\\Patcher"
	}
}

CSettings::~CSettings()
{
}

CSettings::CSettings(const CSettings &csettings) : m_UseRegistry(csettings.m_UseRegistry), m_RegistrySubKey(csettings.m_RegistrySubKey)
{
	*this = csettings;
}

void CSettings::operator = (const CSettings &csettings)
{
	m_UseRegistry = csettings.m_UseRegistry;
	m_RegistrySubKey = csettings.m_RegistrySubKey;
	for(int i = 0; i < csettings.m_Settings.GetSize(); i++)
		m_Settings.Add(csettings.m_Settings.GetAt(i));
}

void CSettings::SetStr(CString name, CString value, bool inregistry /* = false */)
{
	SettingPair temp;
	for(int i = 0; i < m_Settings.GetSize(); i++)
	{
		temp = m_Settings.GetAt(i);
		if(temp.Name == name)
			m_Settings.RemoveAt(i);
	}
	
	temp.Name = name;
	temp.Value = value;
	m_Settings.Add(temp);

	if(inregistry && m_UseRegistry)
		SetToRegistry(name, value);
}

void CSettings::SetInt(CString name, int value, bool inregistry /* = false */)
{
	CString temp;
	temp.Format("%d", value);
	SetStr(name, temp, inregistry);
}

CString CSettings::GetStr(CString name, bool inregistry /* = false */)
{
	SettingPair temp;
	bool bFound = false;
	for(int i = 0; i < m_Settings.GetSize(); i++)
	{
		temp = m_Settings.GetAt(i);
		if(temp.Name == name)
		{
			bFound = true;
			break;
		}
	}

	if(!bFound || temp.Value.IsEmpty())
	{
		temp.Name.Empty();
		temp.Value.Empty();

		if(inregistry && m_UseRegistry)
			temp.Value = GetFromRegistry(name);

		SetStr(name, temp.Value);
	}

	return temp.Value;
}

int CSettings::GetInt(CString name, bool inregistry /* = false */)
{
	CString temp;
	temp = GetStr(name, inregistry);
	return atoi(temp);
}

void CSettings::SetToRegistry(CString name, CString value)
{
	//Initializes variable
	HKEY regKey;

	//Opens the key
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_RegistrySubKey, 0, KEY_ALL_ACCESS, &regKey) != ERROR_SUCCESS)
		if(RegCreateKey(HKEY_LOCAL_MACHINE, m_RegistrySubKey, &regKey) != ERROR_SUCCESS)
		{
			CLog::Instance()->AddLog("(SetRegValue) Error opening/creating registry key: HKEY_LOCAL_MACHINE\\%s", m_RegistrySubKey);
			return;
		}

		//Sets key to value
		if(RegSetValueEx(regKey, name, 0, REG_SZ, (UCHAR*)value.GetString(), (DWORD)value.GetLength() + 1) != ERROR_SUCCESS)
		{
			CLog::Instance()->AddLog("(SetRegValue) Error setting value! Key: %s  Value: %s", name, value);
			return;
		}

		//Closes the key
		RegCloseKey(regKey);

		//Return
		return;
}

CString CSettings::GetFromRegistry(CString name)
{
	//Initializes variables
	CHAR value[1024];
	ULONG vLength = sizeof(value);
	HKEY regKey;
	CString ret = "";

	//Loads the stored value from the registry (Would like to change location of all values)
	if(RegOpenKey(HKEY_LOCAL_MACHINE, m_RegistrySubKey, &regKey) == ERROR_SUCCESS)
	{
		//Get variable from registry
		if(RegQueryValueEx(regKey, name, 0, 0, (UCHAR*)value, &vLength) == ERROR_SUCCESS)
			ret.Format("%s", value);
		else
			SetToRegistry(name, ret);

		RegCloseKey(regKey);
	}
	else
		CLog::Instance()->AddLog("(GetRegValues) Error opening registry key: HKEY_LOCAL_MACHINE\\%s", m_RegistrySubKey);

	//Return value
	return ret;
}