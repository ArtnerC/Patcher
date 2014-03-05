#include "stdafx.h"
#include "Config.h"

const CString CConfig::PRODUCT_NAME			= L"Application";				//Friendly name of main app
const CString CConfig::APP_NAME				= L"Application.exe";			//App to launch after patching
const CString CConfig::MAIN_URL				= L"www.patcher.com";			//Main URL for patch files
const CString CConfig::BACKUP_URL			= L"www.backup-patcher.com";	//Fail-over URL for patch files
const CString CConfig::APP_TITLE			= L"Application  Auto-Updater"; //String on the title bar
const CString CConfig::PATCH_DIR			= L"patcher/";					//Directory the patcher is in relative to the app
const CString CConfig::HTML_NAME			= L"Notes.htm";					//File patch notes are saved to
const CString CConfig::PRIMARY_UPDATER		= L"Updater.exe";				//Patcher executable name
const CString CConfig::SECONDARY_UPDATER	= L"UpdateCheck.exe";			//Secondary updater name
const CString CConfig::REGISTRY_SUBKEY		= L"Software\\Patcher";			//Patcher app specific registry settings folder
const CString CConfig::PATCH_EXT			= L"pfx";						//Extension for patch files
const CString CConfig::NET_SESSION			= CConfig::APP_TITLE;			//Name used in the internet session
const CString CConfig::HELP_STRING			= L"Check Application.com for updates."; //Help string when downloads fail
const CString CConfig::LAUNCH_FILE			= L"app.applaunch";				//File to launch the app with, given on the command line