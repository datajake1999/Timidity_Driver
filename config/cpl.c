#include <windows.h>
#include <cpl.h>

#include "dialog.h"
#include "resource.h"

static HANDLE hModule = NULL;

BOOL WINAPI DllMain(PVOID hmod, ULONG ulReason, PCONTEXT pctx)
{
	if(ulReason != DLL_PROCESS_ATTACH)
	{
		return TRUE;
	}
	else
	{
		hModule = hmod;
	}
	return TRUE;
}

LONG APIENTRY CPlApplet(HWND hwndCPL, UINT uMsg, LONG_PTR lParam1, LONG_PTR lParam2)
{
	LPCPLINFO lpCPlInfo;
	LPNEWCPLINFO lpNewCPlInfo;
	LONG retCode = 0;
	switch (uMsg)
	{
	case CPL_INIT:
		return TRUE;
	case CPL_GETCOUNT:
		return 1;
	case CPL_INQUIRE:
		lpCPlInfo = (LPCPLINFO)lParam2;
		lpCPlInfo->idIcon = IDI_ICON;
		lpCPlInfo->idName = IDS_CPLNAME;
		lpCPlInfo->idInfo = IDS_CPLDESC;
		lpCPlInfo->lData = 0;
		break;
	case CPL_NEWINQUIRE:
		lpNewCPlInfo = (LPNEWCPLINFO)lParam2;
		lpNewCPlInfo->dwSize = sizeof(NEWCPLINFO);
		lpNewCPlInfo->dwFlags = 0;
		lpNewCPlInfo->dwHelpContext = 0;
		lpNewCPlInfo->lData = 0;
		lpNewCPlInfo->hIcon = LoadIcon(hModule, MAKEINTRESOURCE(IDI_ICON));
		lpNewCPlInfo->szHelpFile[0] = '\0';
		LoadString(hModule, IDS_CPLNAME, lpNewCPlInfo->szName, 32);
		LoadString(hModule, IDS_CPLDESC, lpNewCPlInfo->szInfo, 64);
		break;
	case CPL_DBLCLK:
		ShowConfigDialog(hModule, hwndCPL);
		break;
	}
	return retCode;
}
