#include <windows.h>
#include "dialog.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	ShowConfigDialog(hInstance, NULL);
	return 0;
}
