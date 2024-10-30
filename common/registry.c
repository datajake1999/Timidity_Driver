#include <windows.h>
#include <tchar.h>
#include "registry.h"

void ReadRegistry(DriverConfig *cfg)
{
	HKEY hKey;
	DWORD dwType, dwSize, dwValue;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return;

	dwSize = sizeof(int);
	RegQueryValueEx(hKey, REG_NAME_SAMPLERATE, 0, &dwType, (LPBYTE)&cfg->nSampleRate, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_CTRLRATE, 0, &dwType, (LPBYTE)&cfg->nControlRate, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_VOICES, 0, &dwType, (LPBYTE)&cfg->nVoices, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_AMP, 0, &dwType, (LPBYTE)&cfg->nAmp, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_ADJUSTPAN, 0, &dwType, (LPBYTE)&cfg->fAdjustPanning, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_ANTIALIAS, 0, &dwType, (LPBYTE)&cfg->fAntialiasing, &dwSize);
	RegQueryValueEx(hKey, REG_NAME_FASTDEC, 0, &dwType, (LPBYTE)&cfg->fFastDecay, &dwSize);

	dwSize = sizeof(cfg->szConfigFile);
	RegQueryValueEx(hKey, REG_NAME_CONFIGFILE, 0, &dwType, (LPBYTE)&cfg->szConfigFile, &dwSize);

	RegCloseKey(hKey);
}

void WriteRegistry(DriverConfig *cfg)
{
	HKEY hKey;
	DWORD dwDispos, dwValue;

	if (RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY, 
			0, NULL, 0, KEY_WRITE, NULL, &hKey, &dwDispos) != ERROR_SUCCESS)
		return;

	RegSetValueEx(hKey, REG_NAME_SAMPLERATE, 0, REG_DWORD, (LPBYTE)&cfg->nSampleRate, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_CTRLRATE, 0, REG_DWORD, (LPBYTE)&cfg->nControlRate, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_VOICES, 0, REG_DWORD, (LPBYTE)&cfg->nVoices, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_AMP, 0, REG_DWORD, (LPBYTE)&cfg->nAmp, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_ADJUSTPAN, 0, REG_DWORD, (LPBYTE)&cfg->fAdjustPanning, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_ANTIALIAS, 0, REG_DWORD, (LPBYTE)&cfg->fAntialiasing, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_FASTDEC, 0, REG_DWORD, (LPBYTE)&cfg->fFastDecay, sizeof(DWORD));
	RegSetValueEx(hKey, REG_NAME_CONFIGFILE, 0, REG_SZ, 
				(LPBYTE)&cfg->szConfigFile, sizeof(TCHAR) * (_tcslen(cfg->szConfigFile) + 1));

	RegCloseKey(hKey);
}
