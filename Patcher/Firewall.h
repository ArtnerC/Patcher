#pragma once

class IFirewall
{
public:
	bool IsOn() = 0;
	bool IsAppEnabled(CString path) = 0;
	void AddApp(CString path, CString name) = 0;
}