#include "stdafx.h"
#include "Config.h"

const char* CConfig::APP_NAME =			"Application.exe"; //App to launch after patching
const char* CConfig::MAIN_URL =			"www.patcher.com"; //Main URL for patch files
const char* CConfig::BACKUP_URL =		"www.backup-patcher.com"; //Fail-over URL for patch files
const char* CConfig::APP_TITLE =		"Application  Auto-Updater"; //String on the title bar
const char* CConfig::PATCH_DIR =		"patcher/"; //Directory the patcher is in relative to the app
const char* CConfig::HTML_NAME =		"Notes.htm"; //File patch notes are saved to
const char* CConfig::PRIMARY_UPDATER =	"Updater.exe"; //Patcher executable name
const char* CConfig::SECONDARY_UPDATER = "UpdateCheck.exe"; //Secondary updater name
const char* CConfig::REGISTRY_SUBKEY =	"Software\\Patcher"; //Patcher app specific registry settings folder