#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "resource.h"
#include "dialog.h"
#include "../common/registry.h"
#include "../driver/timidity/config.h"

static HINSTANCE hInst;
static DriverConfig cfg;

static UINT WINAPI HookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

static BOOL AboutBox(HWND hWnd)
{
	if (hWnd)
	{
		char caption[MAX_PATH];
		char text[MAX_PATH*2];
		ZeroMemory(caption, sizeof(caption));
		ZeroMemory(text, sizeof(text));
		LoadString(hInst, IDS_ABOUTCAP, caption, MAX_PATH);
		LoadString(hInst, IDS_ABOUTTXT, text, MAX_PATH*2);
		MessageBox(hWnd, text, caption, MB_ICONINFORMATION);
		return TRUE;
	}
	return FALSE;
}

static BOOL WINAPI DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	OPENFILENAME ofn;
	_TCHAR filter[MAX_PATH];
	_TCHAR filename[MAX_PATH];
	_TCHAR title[MAX_PATH];
	_TCHAR caption[MAX_PATH];
	ZeroMemory(&ofn, sizeof(ofn));
	ZeroMemory(filter, sizeof(filter));
	ZeroMemory(filename, sizeof(filename));
	ZeroMemory(title, sizeof(title));
	ZeroMemory(caption, sizeof(caption));
	switch (message)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hWnd, IDC_CTRATES, UDM_SETRANGE32, cfg.nSampleRate/MAX_CONTROL_RATIO, cfg.nSampleRate);
		SendDlgItemMessage(hWnd, IDC_VOICESS, UDM_SETRANGE32, 1, MAX_VOICES);
		SendDlgItemMessage(hWnd, IDC_AMPS, UDM_SETRANGE32, 0, MAX_AMPLIFICATION);
		SetDlgItemText(hWnd, IDC_CFG, cfg.szConfigFile);
		SetDlgItemInt(hWnd, IDC_SAMPRATE, cfg.nSampleRate, FALSE);
		SendDlgItemMessage(hWnd, IDC_CTRATES, UDM_SETPOS32, 0, cfg.nControlRate);
		SendDlgItemMessage(hWnd, IDC_VOICESS, UDM_SETPOS32, 0, cfg.nVoices);
		SendDlgItemMessage(hWnd, IDC_AMPS, UDM_SETPOS32, 0, cfg.nAmp);
		if (cfg.fAdjustPanning)
		{
			CheckDlgButton(hWnd, IDC_PAN, BST_CHECKED);
		}
		if (cfg.fMono)
		{
			CheckDlgButton(hWnd, IDC_MONO, BST_CHECKED);
		}
		if (cfg.f8Bit)
		{
			CheckDlgButton(hWnd, IDC_8BIT, BST_CHECKED);
		}
		if (cfg.fAntialiasing)
		{
			CheckDlgButton(hWnd, IDC_ANTI, BST_CHECKED);
		}
		if (cfg.fPreResample)
		{
			CheckDlgButton(hWnd, IDC_PRERES, BST_CHECKED);
		}
		if (cfg.fFastDecay)
		{
			CheckDlgButton(hWnd, IDC_FASTDEC, BST_CHECKED);
		}
		if (cfg.fDynamicLoad)
		{
			CheckDlgButton(hWnd, IDC_DYNALOAD, BST_CHECKED);
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CFGB:
			LoadString(hInst, IDS_CFGFLT, filter, MAX_PATH);
			LoadString(hInst, IDS_CFGCAP, caption, MAX_PATH);
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.hInstance = hInst;
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.lpstrDefExt = _T("CFG");
			ofn.lpstrFile = filename;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = title;
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrTitle = caption;
			ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
#ifdef OFN_ENABLESIZING
			ofn.Flags |= OFN_ENABLESIZING;
#endif
#ifdef OFN_DONTADDTORECENT
			ofn.Flags |= OFN_DONTADDTORECENT;
#endif
			ofn.lpfnHook = (LPOFNHOOKPROC)HookProc;
			GetDlgItemText(hWnd, IDC_CFG, filename, MAX_PATH);
			if (GetOpenFileName(&ofn))
			{
				SetDlgItemText(hWnd, IDC_CFG, filename);
			}
			return TRUE;
		case IDC_SAMPRATE:
			switch (HIWORD(wParam))
			{
			case EN_KILLFOCUS:
				if (GetDlgItemInt(hWnd, IDC_SAMPRATE, NULL, FALSE) > MAX_OUTPUT_RATE)
				{
					SetDlgItemInt(hWnd, IDC_SAMPRATE, MAX_OUTPUT_RATE, FALSE);
				}
				else if (GetDlgItemInt(hWnd, IDC_SAMPRATE, NULL, FALSE) < MIN_OUTPUT_RATE)
				{
					SetDlgItemInt(hWnd, IDC_SAMPRATE, MIN_OUTPUT_RATE, FALSE);
				}
				SendDlgItemMessage(hWnd, IDC_CTRATES, UDM_SETRANGE32, GetDlgItemInt(hWnd, IDC_SAMPRATE, NULL, FALSE)/MAX_CONTROL_RATIO, GetDlgItemInt(hWnd, IDC_SAMPRATE, NULL, FALSE));
				if (SendDlgItemMessage(hWnd, IDC_CTRATES, UDM_GETPOS32, 0, 0) > GetDlgItemInt(hWnd, IDC_SAMPRATE, NULL, FALSE))
				{
					SendDlgItemMessage(hWnd, IDC_CTRATES, UDM_SETPOS32, 0, GetDlgItemInt(hWnd, IDC_SAMPRATE, NULL, FALSE));
				}
				else if (SendDlgItemMessage(hWnd, IDC_CTRATES, UDM_GETPOS32, 0, 0) < GetDlgItemInt(hWnd, IDC_SAMPRATE, NULL, FALSE)/MAX_CONTROL_RATIO)
				{
					SendDlgItemMessage(hWnd, IDC_CTRATES, UDM_SETPOS32, 0, GetDlgItemInt(hWnd, IDC_SAMPRATE, NULL, FALSE)/MAX_CONTROL_RATIO);
				}
				return TRUE;
			default:
				return FALSE;
			}
		case IDC_ABOUT:
			return AboutBox(hWnd);
		case IDOK:
		case IDC_APPLY:
			GetDlgItemText(hWnd, IDC_CFG, cfg.szConfigFile, MAX_PATH);
			cfg.nSampleRate = GetDlgItemInt(hWnd, IDC_SAMPRATE, NULL, FALSE);
			if (cfg.nSampleRate > MAX_OUTPUT_RATE)
			{
				cfg.nSampleRate = MAX_OUTPUT_RATE;
			}
			else if (cfg.nSampleRate < MIN_OUTPUT_RATE)
			{
				cfg.nSampleRate = MIN_OUTPUT_RATE;
			}
			cfg.nControlRate = SendDlgItemMessage(hWnd, IDC_CTRATES, UDM_GETPOS32, 0, 0);
			if (cfg.nControlRate > cfg.nSampleRate)
			{
				cfg.nControlRate = cfg.nSampleRate;
			}
			else if (cfg.nControlRate < cfg.nSampleRate/MAX_CONTROL_RATIO)
			{
				cfg.nControlRate = cfg.nSampleRate/MAX_CONTROL_RATIO;
			}
			cfg.nVoices = SendDlgItemMessage(hWnd, IDC_VOICESS, UDM_GETPOS32, 0, 0);
			if (cfg.nVoices > MAX_VOICES)
			{
				cfg.nVoices = MAX_VOICES;
			}
			else if (cfg.nVoices < 1)
			{
				cfg.nVoices = 1;
			}
			cfg.nAmp = SendDlgItemMessage(hWnd, IDC_AMPS, UDM_GETPOS32, 0, 0);
			if (cfg.nAmp > MAX_AMPLIFICATION)
			{
				cfg.nAmp = MAX_AMPLIFICATION;
			}
			else if (cfg.nAmp < 0)
			{
				cfg.nAmp = 0;
			}
			if (IsDlgButtonChecked(hWnd, IDC_PAN))
			{
				cfg.fAdjustPanning = TRUE;
			}
			else
			{
				cfg.fAdjustPanning = FALSE;
			}
			if (IsDlgButtonChecked(hWnd, IDC_MONO))
			{
				cfg.fMono = TRUE;
			}
			else
			{
				cfg.fMono = FALSE;
			}
			if (IsDlgButtonChecked(hWnd, IDC_8BIT))
			{
				cfg.f8Bit = TRUE;
			}
			else
			{
				cfg.f8Bit = FALSE;
			}
			if (IsDlgButtonChecked(hWnd, IDC_ANTI))
			{
				cfg.fAntialiasing = TRUE;
			}
			else
			{
				cfg.fAntialiasing = FALSE;
			}
			if (IsDlgButtonChecked(hWnd, IDC_PRERES))
			{
				cfg.fPreResample = TRUE;
			}
			else
			{
				cfg.fPreResample = FALSE;
			}
			if (IsDlgButtonChecked(hWnd, IDC_FASTDEC))
			{
				cfg.fFastDecay = TRUE;
			}
			else
			{
				cfg.fFastDecay = FALSE;
			}
			if (IsDlgButtonChecked(hWnd, IDC_DYNALOAD))
			{
				cfg.fDynamicLoad = TRUE;
			}
			else
			{
				cfg.fDynamicLoad = FALSE;
			}
			if (LOWORD(wParam) == IDOK)
			{
				EndDialog(hWnd, TRUE);
			}
			return TRUE;
		case IDCANCEL:
			EndDialog(hWnd, FALSE);
			return TRUE;
		}
	}
	return FALSE;
}

void ShowConfigDialog(HINSTANCE hInstance, HWND hWnd)
{
	hInst = hInstance;
	ZeroMemory(&cfg, sizeof(cfg));
	cfg.nSampleRate = DEFAULT_RATE;
	cfg.nControlRate = CONTROLS_PER_SECOND;
	cfg.nVoices = DEFAULT_VOICES;
	cfg.nAmp = DEFAULT_AMPLIFICATION;
	cfg.fAdjustPanning = TRUE;
	cfg.fMono = FALSE;
	cfg.f8Bit = FALSE;
	cfg.fAntialiasing = TRUE;
	cfg.fPreResample = TRUE;
	cfg.fFastDecay = TRUE;
	cfg.fDynamicLoad = FALSE;
	InitCommonControls();
	ReadRegistry(&cfg);
	if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG), hWnd, (DLGPROC)DialogProc))
	{
		WriteRegistry(&cfg);
	}
}
