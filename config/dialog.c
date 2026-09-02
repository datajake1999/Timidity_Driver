#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <tchar.h>

#include "dialog.h"
#include "resource.h"
#include "../common/registry.h"
#include "../driver/timidity/config.h"

static HINSTANCE hInst;
static DriverConfig cfg;

static const TCHAR ReverbPresetNames[][32] =
{
	_T("GENERIC"),
	_T("PADDEDCELL"),
	_T("ROOM"),
	_T("BATHROOM"),
	_T("LIVINGROOM"),
	_T("STONEROOM"),
	_T("AUDITORIUM"),
	_T("CONCERTHALL"),
	_T("CAVE"),
	_T("ARENA"),
	_T("HANGAR"),
	_T("CARPETEDHALLWAY"),
	_T("HALLWAY"),
	_T("STONECORRIDOR"),
	_T("ALLEY"),
	_T("FOREST"),
	_T("CITY"),
	_T("MOUNTAINS"),
	_T("QUARRY"),
	_T("PLAIN"),
	_T("PARKINGLOT"),
	_T("SEWERPIPE"),
	_T("UNDERWATER"),
	_T("DRUGGED"),
	_T("DIZZY"),
	_T("PSYCHOTIC"),
	_T("CASTLE_SMALLROOM"),
	_T("CASTLE_SHORTPASSAGE"),
	_T("CASTLE_MEDIUMROOM"),
	_T("CASTLE_LARGEROOM"),
	_T("CASTLE_LONGPASSAGE"),
	_T("CASTLE_HALL"),
	_T("CASTLE_CUPBOARD"),
	_T("CASTLE_COURTYARD"),
	_T("CASTLE_ALCOVE"),
	_T("FACTORY_SMALLROOM"),
	_T("FACTORY_SHORTPASSAGE"),
	_T("FACTORY_MEDIUMROOM"),
	_T("FACTORY_LARGEROOM"),
	_T("FACTORY_LONGPASSAGE"),
	_T("FACTORY_HALL"),
	_T("FACTORY_CUPBOARD"),
	_T("FACTORY_COURTYARD"),
	_T("FACTORY_ALCOVE"),
	_T("ICEPALACE_SMALLROOM"),
	_T("ICEPALACE_SHORTPASSAGE"),
	_T("ICEPALACE_MEDIUMROOM"),
	_T("ICEPALACE_LARGEROOM"),
	_T("ICEPALACE_LONGPASSAGE"),
	_T("ICEPALACE_HALL"),
	_T("ICEPALACE_CUPBOARD"),
	_T("ICEPALACE_COURTYARD"),
	_T("ICEPALACE_ALCOVE"),
	_T("SPACESTATION_SMALLROOM"),
	_T("SPACESTATION_SHORTPASSAGE"),
	_T("SPACESTATION_MEDIUMROOM"),
	_T("SPACESTATION_LARGEROOM"),
	_T("SPACESTATION_LONGPASSAGE"),
	_T("SPACESTATION_HALL"),
	_T("SPACESTATION_CUPBOARD"),
	_T("SPACESTATION_ALCOVE"),
	_T("WOODEN_SMALLROOM"),
	_T("WOODEN_SHORTPASSAGE"),
	_T("WOODEN_MEDIUMROOM"),
	_T("WOODEN_LARGEROOM"),
	_T("WOODEN_LONGPASSAGE"),
	_T("WOODEN_HALL"),
	_T("WOODEN_CUPBOARD"),
	_T("WOODEN_COURTYARD"),
	_T("WOODEN_ALCOVE"),
	_T("SPORT_EMPTYSTADIUM"),
	_T("SPORT_SQUASHCOURT"),
	_T("SPORT_SMALLSWIMMINGPOOL"),
	_T("SPORT_LARGESWIMMINGPOOL"),
	_T("SPORT_GYMNASIUM"),
	_T("SPORT_FULLSTADIUM"),
	_T("SPORT_STADIUMTANNOY"),
	_T("PREFAB_WORKSHOP"),
	_T("PREFAB_SCHOOLROOM"),
	_T("PREFAB_PRACTISEROOM"),
	_T("PREFAB_OUTHOUSE"),
	_T("PREFAB_CARAVAN"),
	_T("DOME_TOMB"),
	_T("PIPE_SMALL"),
	_T("DOME_SAINTPAULS"),
	_T("PIPE_LONGTHIN"),
	_T("PIPE_LARGE"),
	_T("PIPE_RESONANT"),
	_T("OUTDOORS_BACKYARD"),
	_T("OUTDOORS_ROLLINGPLAINS"),
	_T("OUTDOORS_DEEPCANYON"),
	_T("OUTDOORS_CREEK"),
	_T("OUTDOORS_VALLEY"),
	_T("MOOD_HEAVEN"),
	_T("MOOD_HELL"),
	_T("MOOD_MEMORY"),
	_T("DRIVING_COMMENTATOR"),
	_T("DRIVING_PITGARAGE"),
	_T("DRIVING_INCAR_RACER"),
	_T("DRIVING_INCAR_SPORTS"),
	_T("DRIVING_INCAR_LUXURY"),
	_T("DRIVING_FULLGRANDSTAND"),
	_T("DRIVING_EMPTYGRANDSTAND"),
	_T("DRIVING_TUNNEL"),
	_T("CITY_STREETS"),
	_T("CITY_SUBWAY"),
	_T("CITY_MUSEUM"),
	_T("CITY_LIBRARY"),
	_T("CITY_UNDERPASS"),
	_T("CITY_ABANDONED"),
	_T("DUSTYROOM"),
	_T("CHAPEL"),
	_T("SMALLWATERROOM")
};

#define NUM_REVERB_PRESETS (sizeof(ReverbPresetNames)/sizeof(ReverbPresetNames[0]))

static UINT WINAPI HookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

static BOOL SetConfigFile(HWND hWnd)
{
	if (hWnd)
	{
		OPENFILENAME ofn;
		TCHAR filter[MAX_PATH];
		TCHAR filename[MAX_PATH];
		TCHAR title[MAX_PATH];
		TCHAR directory[MAX_PATH];
		TCHAR caption[MAX_PATH];
		TCHAR *separator;
		ZeroMemory(&ofn, sizeof(ofn));
		ZeroMemory(filter, sizeof(filter));
		ZeroMemory(filename, sizeof(filename));
		ZeroMemory(title, sizeof(title));
		ZeroMemory(directory, sizeof(directory));
		ZeroMemory(caption, sizeof(caption));
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
		ofn.lpstrInitialDir = directory;
		ofn.lpstrTitle = caption;
		ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
#ifdef OFN_ENABLESIZING
		ofn.Flags |= OFN_ENABLESIZING;
#endif
#ifdef OFN_DONTADDTORECENT
		ofn.Flags |= OFN_DONTADDTORECENT;
#endif
		ofn.lpfnHook = (LPOFNHOOKPROC)HookProc;
		_tcscpy(directory, cfg.szConfigFile);
		separator = _tcsrchr(directory, _T('\\'));
		if (separator) *separator = _T('\0');
		GetDlgItemText(hWnd, IDC_CFG, filename, MAX_PATH);
		if (GetOpenFileName(&ofn))
		{
			SetDlgItemText(hWnd, IDC_CFG, filename);
		}
		return TRUE;
	}
	return FALSE;
}

static BOOL SetDefaultInstrument(HWND hWnd)
{
	if (hWnd)
	{
		OPENFILENAME ofn;
		TCHAR filter[MAX_PATH];
		TCHAR filename[MAX_PATH];
		TCHAR title[MAX_PATH];
		TCHAR directory[MAX_PATH];
		TCHAR caption[MAX_PATH];
		TCHAR *separator;
		ZeroMemory(&ofn, sizeof(ofn));
		ZeroMemory(filter, sizeof(filter));
		ZeroMemory(filename, sizeof(filename));
		ZeroMemory(title, sizeof(title));
		ZeroMemory(directory, sizeof(directory));
		ZeroMemory(caption, sizeof(caption));
		LoadString(hInst, IDS_INSTFLT, filter, MAX_PATH);
		LoadString(hInst, IDS_INSTCAP, caption, MAX_PATH);
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.hInstance = hInst;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = _T("PAT");
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFileTitle = title;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.lpstrInitialDir = directory;
		ofn.lpstrTitle = caption;
		ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
#ifdef OFN_ENABLESIZING
		ofn.Flags |= OFN_ENABLESIZING;
#endif
#ifdef OFN_DONTADDTORECENT
		ofn.Flags |= OFN_DONTADDTORECENT;
#endif
		ofn.lpfnHook = (LPOFNHOOKPROC)HookProc;
		_tcscpy(directory, cfg.szDefaultInstrument);
		separator = _tcsrchr(directory, _T('\\'));
		if (separator) *separator = _T('\0');
		GetDlgItemText(hWnd, IDC_DEFINST, filename, MAX_PATH);
		if (GetOpenFileName(&ofn))
		{
			SetDlgItemText(hWnd, IDC_DEFINST, filename);
		}
		return TRUE;
	}
	return FALSE;
}

static BOOL AboutBox(HWND hWnd)
{
	if (hWnd)
	{
		TCHAR caption[MAX_PATH];
		TCHAR text[MAX_PATH*2];
		ZeroMemory(caption, sizeof(caption));
		ZeroMemory(text, sizeof(text));
		LoadString(hInst, IDS_ABOUTCAP, caption, MAX_PATH);
		LoadString(hInst, IDS_ABOUTTXT, text, MAX_PATH*2);
		MessageBox(hWnd, text, caption, MB_ICONINFORMATION);
		return TRUE;
	}
	return FALSE;
}

static BOOL WINAPI DrumDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT i;
	switch (message)
	{
	case WM_INITDIALOG:
		for (i = 0; i < 16; i++)
		{
			if (cfg.nDrumChannels & (1<<i))
			{
				CheckDlgButton(hWnd, IDC_DCHAN01+i, BST_CHECKED);
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			cfg.nDrumChannels = 0;
			for (i = 0; i < 16; i++)
			{
				if (IsDlgButtonChecked(hWnd, IDC_DCHAN01+i))
				{
					cfg.nDrumChannels |= (1<<i);
				}
			}
			EndDialog(hWnd, TRUE);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWnd, FALSE);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL WINAPI QuietDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT i;
	switch (message)
	{
	case WM_INITDIALOG:
		for (i = 0; i < 16; i++)
		{
			if (cfg.nQuietChannels & (1<<i))
			{
				CheckDlgButton(hWnd, IDC_QCHAN01+i, BST_CHECKED);
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			cfg.nQuietChannels = 0;
			for (i = 0; i < 16; i++)
			{
				if (IsDlgButtonChecked(hWnd, IDC_QCHAN01+i))
				{
					cfg.nQuietChannels |= (1<<i);
				}
			}
			EndDialog(hWnd, TRUE);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWnd, FALSE);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL WINAPI DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT i;
	switch (message)
	{
	case WM_INITDIALOG:
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
		cfg.nDefaultProgram = DEFAULT_PROGRAM;
		cfg.nDrumChannels = DEFAULT_DRUMCHANNELS;
		cfg.nQuietChannels = 0;
		cfg.fReverbEnabled = FALSE;
		cfg.nReverbLevel = 100;
		cfg.nReverbPreset = 0;
		ReadRegistry(&cfg);
		SendDlgItemMessage(hWnd, IDC_CTRATES, UDM_SETRANGE32, cfg.nSampleRate/MAX_CONTROL_RATIO, cfg.nSampleRate);
		SendDlgItemMessage(hWnd, IDC_VOICESS, UDM_SETRANGE32, 1, MAX_VOICES);
		SendDlgItemMessage(hWnd, IDC_AMPS, UDM_SETRANGE32, 0, MAX_AMPLIFICATION);
		SendDlgItemMessage(hWnd, IDC_DEFPROGS, UDM_SETRANGE32, 0, 127);
		SendDlgItemMessage(hWnd, IDC_REVERBLEVEL, TBM_SETRANGE, 0, MAKELONG(0, 100));
		SendDlgItemMessage(hWnd, IDC_REVERBLEVEL, TBM_SETPAGESIZE, 0, 10);
		for (i = 0; i < NUM_REVERB_PRESETS; i++)
		{
			SendDlgItemMessage(hWnd, IDC_REVERBPRESET, CB_ADDSTRING, 0, (LPARAM)ReverbPresetNames[i]);
		}
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
		SetDlgItemText(hWnd, IDC_DEFINST, cfg.szDefaultInstrument);
		SendDlgItemMessage(hWnd, IDC_DEFPROGS, UDM_SETPOS32, 0, cfg.nDefaultProgram);
		if (cfg.fReverbEnabled)
		{
			CheckDlgButton(hWnd, IDC_REVERBENABLED, BST_CHECKED);
		}
		SendDlgItemMessage(hWnd, IDC_REVERBLEVEL, TBM_SETPOS, TRUE, (LPARAM)cfg.nReverbLevel);
		SendDlgItemMessage(hWnd, IDC_REVERBPRESET, CB_SETCURSEL, cfg.nReverbPreset, 0);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CFGB:
			return SetConfigFile(hWnd);
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
		case IDC_DEFINSTB:
			return SetDefaultInstrument(hWnd);
		case IDC_DRUMCHANNELS:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DRUMDLG), hWnd, (DLGPROC)DrumDialogProc);
			return TRUE;
		case IDC_QUIETCHANNELS:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_QUIETDLG), hWnd, (DLGPROC)QuietDialogProc);
			return TRUE;
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
			GetDlgItemText(hWnd, IDC_DEFINST, cfg.szDefaultInstrument, MAX_PATH);
			cfg.nDefaultProgram = SendDlgItemMessage(hWnd, IDC_DEFPROGS, UDM_GETPOS32, 0, 0);
			if (cfg.nDefaultProgram > 127)
			{
				cfg.nDefaultProgram = 127;
			}
			else if (cfg.nDefaultProgram < 0)
			{
				cfg.nDefaultProgram = 0;
			}
			if (IsDlgButtonChecked(hWnd, IDC_REVERBENABLED))
			{
				cfg.fReverbEnabled = TRUE;
			}
			else
			{
				cfg.fReverbEnabled = FALSE;
			}
			cfg.nReverbLevel = SendDlgItemMessage(hWnd, IDC_REVERBLEVEL, TBM_GETPOS, 0, 0);
			cfg.nReverbPreset = SendDlgItemMessage(hWnd, IDC_REVERBPRESET, CB_GETCURSEL, 0, 0);
			WriteRegistry(&cfg);
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
	InitCommonControls();
	DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG), hWnd, (DLGPROC)DialogProc);
}
