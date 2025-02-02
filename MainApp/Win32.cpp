/*
*                              TylerToys
*
*      Copyright 2024  Tyler Parret True
*
*    Licensed under the Apache License, Version 2.0 (the "License");
*    you may not use this file except in compliance with the License.
*    You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
*    Unless required by applicable law or agreed to in writing, software
*    distributed under the License is distributed on an "AS IS" BASIS,
*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*    See the License for the specific language governing permissions and
*    limitations under the License.
*
* @Authors
*    Tyler Parret True <mysteryworldgod@outlook.com><https://github.com/OwlHowlinMornSky>
*/
#include <format>

#include "Win32.h"

#include <windowsx.h>
#include <CommCtrl.h>
#include <shellapi.h>
#include <strsafe.h>
#include "Mute.h"
#include "resource.h"
#include "WinCheck.h"
#include "AppGlobal.h"
#include "RegSettings.h"
#include "ContextMenu.h"

namespace {

constexpr size_t MAX_LOADSTRING = 128;
WCHAR szTitle[MAX_LOADSTRING]; // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING]; // 主窗口类名

WCHAR szMainBtnRun[MAX_LOADSTRING];
WCHAR szMainBtnStop[MAX_LOADSTRING];

WCHAR szHideOnStarting[MAX_LOADSTRING];
WCHAR szEnabled[MAX_LOADSTRING];
WCHAR szDeviceKeep[MAX_LOADSTRING];
WCHAR szDisplayKeep[MAX_LOADSTRING];

WCHAR szLocalDeviceName[MAX_LOADSTRING];

WCHAR szSenderPath[MAX_LOADSTRING];

WCHAR szPowerTitleAC[MAX_LOADSTRING];
WCHAR szPowerTitleDC[MAX_LOADSTRING];
WCHAR szPowerTitleERR[MAX_LOADSTRING];
WCHAR szPowerTextAC[MAX_LOADSTRING];
WCHAR szPowerTextDC[MAX_LOADSTRING];
WCHAR szPowerTextERR[MAX_LOADSTRING];

constexpr UINT MYWM_CALLBACK = WM_APP + 233;
constexpr int NOTIFYICON_ID = 233;

HFONT hFontTitle = NULL;
HFONT hFontText = NULL;

HWND hBtnMain = NULL;
HWND hBtnSettingHideOnStarting = NULL;

HWND hBoxMuteHotKey = NULL;
HWND hBtnMuteHotKeySettingEnabled = NULL;
HWND hHotMuteHotKeySetBox = NULL;

bool isRunning = false;
bool isMuteHotKeyRunning = false;

HWND hBoxPowerSiren = NULL;
HWND hBtnPowerSirenSettingEnabled = NULL;

bool isPowerSirenRunning = false;

static constexpr size_t ID_TIMER_AC = 0;
static constexpr size_t ID_TIMER_DC = 1;
static constexpr size_t ID_TIMER_UNKNOWN = 2;
UINT_PTR g_timers[3] = {};
UINT_PTR g_sendSt[3] = {};
UINT g_timethres = 60 * 1000;
bool g_ErrorSent = false;
bool g_DcSent = false;

HWND hBoxKeeper = NULL;
HWND hBtnDeviceKeeperSettingEnabled = NULL;
HWND hBtnDisplayKeeperSettingEnabled = NULL;

UINT ModFromCode(WORD wHotkey) {
	UINT res = 0;
	BYTE code = HIBYTE(wHotkey);
	if (code & HOTKEYF_SHIFT)
		res |= MOD_SHIFT;
	if (code & HOTKEYF_CONTROL)
		res |= MOD_CONTROL;
	if (code & HOTKEYF_ALT)
		res |= MOD_ALT;
	return res;
}

UINT VkFromCode(WORD wHotkey) {
	return LOBYTE(wHotkey);
}

void TestSender() {
	SHELLEXECUTEINFOW info{};

	info.cbSize = sizeof(info);
	info.lpVerb = L"open";
	info.lpFile = szSenderPath;
	info.lpParameters = L"--test";
	ShellExecuteExW(&info);
	return;
}

void Send(std::wstring_view title, std::wstring_view text) {
	if (!isRunning || !isPowerSirenRunning)
		return;

	SHELLEXECUTEINFOW info{};

	info.cbSize = sizeof(info);
	info.lpVerb = L"open";
	info.lpFile = szSenderPath;

	std::wstring str;
	str.assign(L"-sub \"");
	str.append(title);
	str.append(L"\" -t \"");
	str.append(text);
	str.append(L"\"");

	info.lpParameters = str.c_str();
	ShellExecuteExW(&info);
	return;
}

void InitGuiFromSettings() {
	Button_SetCheck(hBtnSettingHideOnStarting, RegistrySettings::instance()->IsHideWindowOnStarting() ? BST_CHECKED : BST_UNCHECKED);

	Button_SetCheck(hBtnDeviceKeeperSettingEnabled, RegistrySettings::instance()->IsDeviceKeeperEnabled() ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(hBtnDisplayKeeperSettingEnabled, RegistrySettings::instance()->IsDisplayKeeperEnabled() ? BST_CHECKED : BST_UNCHECKED);

	Button_SetCheck(hBtnPowerSirenSettingEnabled, RegistrySettings::instance()->IsPowerSirenEnabled() ? BST_CHECKED : BST_UNCHECKED);

	Button_SetCheck(hBtnMuteHotKeySettingEnabled, RegistrySettings::instance()->IsMuteHotkeyEnabled() ? BST_CHECKED : BST_UNCHECKED);
	SendMessageW(hHotMuteHotKeySetBox, HKM_SETHOTKEY, RegistrySettings::instance()->GetMuteKey(), NULL);

	return;
}

void CopyGuiToSettings() {
	RegistrySettings::instance()->SetHideWindowOnStarting(Button_GetCheck(hBtnSettingHideOnStarting) == BST_CHECKED);

	RegistrySettings::instance()->SetDeviceKeeperEnabled(Button_GetCheck(hBtnDeviceKeeperSettingEnabled) == BST_CHECKED);
	RegistrySettings::instance()->SetDisplayKeeperEnabled(Button_GetCheck(hBtnDisplayKeeperSettingEnabled) == BST_CHECKED);

	RegistrySettings::instance()->SetPowerSirenEnabled(Button_GetCheck(hBtnPowerSirenSettingEnabled) == BST_CHECKED);

	RegistrySettings::instance()->SetMuteHotkeyEnabled(Button_GetCheck(hBtnMuteHotKeySettingEnabled) == BST_CHECKED);
	RegistrySettings::instance()->SetMuteKey((WORD)SendMessageW(hHotMuteHotKeySetBox, HKM_GETHOTKEY, 0, 0));

	return;
}

bool MySetKeepDisplay(bool keepDisplay, bool keepSystem) {
	EXECUTION_STATE state = ES_CONTINUOUS;
	if (keepDisplay)
		state |= ES_DISPLAY_REQUIRED;
	if (keepSystem)
		state |= ES_SYSTEM_REQUIRED;
	return SetThreadExecutionState(state);
}

void ResetAllParts(HWND hWnd) {
	if (isMuteHotKeyRunning) {
		if (UnregisterHotKey(hWnd, 999)) {
			isMuteHotKeyRunning = false;
		}
		else {
			ParseWin32Error(AppNameW + L": Failed to Unregister Hotkey");
		}
	}

	isPowerSirenRunning = false;
	g_ErrorSent = false;
	g_DcSent = false;
	for (int i = 0; i < 3; ++i) {
		g_sendSt[i] = 0;
		if (g_timers[i]) {
			KillTimer(hWnd, i);
			g_timers[i] = 0;
		}
	}

	MySetKeepDisplay(false, false);

	SetWindowTextW(hBtnMain, szMainBtnRun);
	isRunning = false;

	EnableWindow(hBtnMuteHotKeySettingEnabled, TRUE);
	EnableWindow(hHotMuteHotKeySetBox, TRUE);

	EnableWindow(hBtnPowerSirenSettingEnabled, TRUE);

	EnableWindow(hBtnDeviceKeeperSettingEnabled, TRUE);
	EnableWindow(hBtnDisplayKeeperSettingEnabled, TRUE);
	return;
}

void Run(HWND hWnd) {
	EnableWindow(hBtnMuteHotKeySettingEnabled, FALSE);
	EnableWindow(hHotMuteHotKeySetBox, FALSE);

	EnableWindow(hBtnPowerSirenSettingEnabled, FALSE);

	EnableWindow(hBtnDeviceKeeperSettingEnabled, FALSE);
	EnableWindow(hBtnDisplayKeeperSettingEnabled, FALSE);

	if (RegistrySettings::instance()->IsMuteHotkeyEnabled()) {
		if (RegistrySettings::instance()->GetMuteKey() == 0) {
			MessageBoxW(hWnd, L"Please set a hotkey combination.", AppNameW.data(), MB_ICONINFORMATION);
			return;
		}

		if (RegisterHotKey(
			hWnd, 999,
			ModFromCode(RegistrySettings::instance()->GetMuteKey()) | MOD_NOREPEAT,
			VkFromCode(RegistrySettings::instance()->GetMuteKey())
		)) {
			isMuteHotKeyRunning = true;
		}
		else {
			ParseWin32Error(AppNameW + L": Failed to Register Hotkey");
		}
	}

	if (RegistrySettings::instance()->IsPowerSirenEnabled()) {
		TestSender();
		isPowerSirenRunning = true;
	}

	MySetKeepDisplay(
		RegistrySettings::instance()->IsDisplayKeeperEnabled(),
		RegistrySettings::instance()->IsDeviceKeeperEnabled()
	);

	SetWindowTextW(hBtnMain, szMainBtnStop);
	isRunning = true;
	return;
}

void OnBtnMain_Clicked(HWND hWnd) {
	bool running = isRunning;
	ResetAllParts(hWnd);
	if (!running) {
		CopyGuiToSettings();
		Run(hWnd);
	}
	return;
}

void OnWindowDestroy(HWND hWnd) {
	ResetAllParts(hWnd);
	CopyGuiToSettings();
}

ContextMenu g_contextMenu;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		hFontTitle = CreateFontW(
			20, 0, 0, 0, FW_DONTCARE,
			FALSE, FALSE, FALSE,
			GB2312_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			L"Segoe UI"
		);
		hFontText = CreateFontW(
			16, 0, 0, 0, FW_DONTCARE,
			FALSE, FALSE, FALSE,
			GB2312_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			L"Segoe UI"
		);

		hBtnMain = CreateWindowExW(
			0,
			WC_BUTTONW, szMainBtnRun,
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			270, 120, 100, 40,
			hWnd, NULL, GetModuleHandleW(NULL), NULL
		);
		SendMessageW(hBtnMain, WM_SETFONT, (WPARAM)hFontTitle, TRUE);

		hBtnSettingHideOnStarting = CreateWindowExW(
			0,
			WC_BUTTONW, szHideOnStarting,
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
			10, 190, 240, 20,
			hWnd, NULL, GetModuleHandleW(NULL), NULL
		);
		SendMessageW(hBtnSettingHideOnStarting, WM_SETFONT, (WPARAM)hFontText, TRUE);

		{ // MutHotKey
			hBoxMuteHotKey = CreateWindowExW(
				0,
				WC_BUTTONW, L"MuteHotKey",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
				10, 10, 200, 80,
				hWnd, NULL, GetModuleHandleW(NULL), NULL
			);
			SendMessageW(hBoxMuteHotKey, WM_SETFONT, (WPARAM)hFontTitle, TRUE);

			hBtnMuteHotKeySettingEnabled = CreateWindowExW(
				0,
				WC_BUTTONW, szEnabled,
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
				15, 52, 100, 20,
				hBoxMuteHotKey, NULL, GetModuleHandleW(NULL), NULL
			);
			SendMessageW(hBtnMuteHotKeySettingEnabled, WM_SETFONT, (WPARAM)hFontText, TRUE);

			hHotMuteHotKeySetBox = CreateWindowExW(
				0,
				HOTKEY_CLASSW, L"",
				WS_TABSTOP | WS_CHILD | WS_VISIBLE,
				10, 25, 180, 21,
				hBoxMuteHotKey, NULL, GetModuleHandleW(NULL), NULL
			);
			SendMessageW(hHotMuteHotKeySetBox, WM_SETFONT, (WPARAM)hFontText, TRUE);
			SendMessageW(hHotMuteHotKeySetBox, HKM_SETRULES, (WPARAM)HKCOMB_NONE, MAKELPARAM(HOTKEYF_ALT, 0));
		}

		{
			hBoxPowerSiren = CreateWindowExW(
				0,
				WC_BUTTONW, L"PowerSiren",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
				10, 100, 200, 80,
				hWnd, NULL, GetModuleHandleW(NULL), NULL
			);
			SendMessageW(hBoxPowerSiren, WM_SETFONT, (WPARAM)hFontTitle, TRUE);

			hBtnPowerSirenSettingEnabled = CreateWindowExW(
				0,
				WC_BUTTONW, szEnabled,
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
				15, 25, 100, 20,
				hBoxPowerSiren, NULL, GetModuleHandleW(NULL), NULL
			);
			SendMessageW(hBtnPowerSirenSettingEnabled, WM_SETFONT, (WPARAM)hFontText, TRUE);
		}

		{
			hBoxKeeper = CreateWindowExW(
				0,
				WC_BUTTONW, L"SystemKeeper",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
				220, 10, 200, 80,
				hWnd, NULL, GetModuleHandleW(NULL), NULL
			);
			SendMessageW(hBoxKeeper, WM_SETFONT, (WPARAM)hFontTitle, TRUE);

			hBtnDeviceKeeperSettingEnabled = CreateWindowExW(
				0,
				WC_BUTTONW, szDeviceKeep,
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
				15, 25, 100, 20,
				hBoxKeeper, NULL, GetModuleHandleW(NULL), NULL
			);
			SendMessageW(hBtnDeviceKeeperSettingEnabled, WM_SETFONT, (WPARAM)hFontText, TRUE);

			hBtnDisplayKeeperSettingEnabled = CreateWindowExW(
				0,
				WC_BUTTONW, szDisplayKeep,
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
				15, 50, 100, 20,
				hBoxKeeper, NULL, GetModuleHandleW(NULL), NULL
			);
			SendMessageW(hBtnDisplayKeeperSettingEnabled, WM_SETFONT, (WPARAM)hFontText, TRUE);
		}

		InitGuiFromSettings();

		if (RegistrySettings::instance()->IsHideWindowOnStarting()) {
			OnBtnMain_Clicked(hWnd);
		}
		break;

	case WM_DESTROY:
		OnWindowDestroy(hWnd);

		DestroyWindow(hBoxKeeper);

		DestroyWindow(hBoxPowerSiren);

		//DestroyWindow(hHotMuteHotKeySetBox);
		//DestroyWindow(hBtnMuteHotKeySettingEnabled);
		DestroyWindow(hBoxMuteHotKey);

		DestroyWindow(hBtnSettingHideOnStarting);
		DestroyWindow(hBtnMain);

		DeleteObject(hFontText);
		DeleteObject(hFontTitle);
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			if ((HWND)lParam == hBtnMain) {
				OnBtnMain_Clicked(hWnd);
			}
			else if (LOWORD(wParam) == ContextMenu::Exit) {
				PostMessageW(hWnd, WM_CLOSE, NULL, NULL);
			}
		}
		break;

	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_MINIMIZE)
			ShowWindow(hWnd, SW_HIDE);
		else
			return DefWindowProcW(hWnd, message, wParam, lParam);
		break;

	case WM_HOTKEY:
		if (wParam == 999) {
			MuteWindow* mute = MuteWindow::instance();
			if (nullptr != mute)
				mute->TryMuteOrUnmuteForegroundWindow();
		}
		else {
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		break;

	case MYWM_CALLBACK:
		if (LOWORD(lParam) == NIN_SELECT)
			ShowWindow(hWnd, SW_RESTORE);
		else if (LOWORD(lParam) == WM_CONTEXTMENU)
			g_contextMenu.Pop(hWnd, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam));
		break;

	case WM_TIMER:
		switch (wParam) {
		case ID_TIMER_AC:
			if (!g_DcSent || g_sendSt[ID_TIMER_AC]) {
				KillTimer(hWnd, ID_TIMER_AC);
				g_timers[ID_TIMER_AC] = 0;
				break;
			}
			g_DcSent = false;
			g_sendSt[ID_TIMER_AC] = 1;
			Send(
				szPowerTitleAC,
				std::vformat(szPowerTextAC, std::make_wformat_args(szLocalDeviceName))
			);
			KillTimer(hWnd, ID_TIMER_AC);
			g_timers[ID_TIMER_AC] = 0;
			break;
		case ID_TIMER_DC:
			if (g_sendSt[ID_TIMER_DC]) {
				KillTimer(hWnd, ID_TIMER_DC);
				g_timers[ID_TIMER_DC] = 0;
				break;
			}
			g_DcSent = true;
			g_sendSt[ID_TIMER_DC] = 1;
			Send(
				szPowerTitleDC,
				std::vformat(szPowerTextDC, std::make_wformat_args(szLocalDeviceName))
			);
			KillTimer(hWnd, ID_TIMER_DC);
			g_timers[ID_TIMER_DC] = 0;
			break;
		case ID_TIMER_UNKNOWN:
			if (g_ErrorSent || g_sendSt[ID_TIMER_UNKNOWN]) {
				KillTimer(hWnd, ID_TIMER_UNKNOWN);
				g_timers[ID_TIMER_UNKNOWN] = 0;
				break;
			}
			g_ErrorSent = true;
			g_sendSt[ID_TIMER_UNKNOWN] = 1;
			Send(
				szPowerTitleERR, szPowerTextERR
			);
			KillTimer(hWnd, ID_TIMER_UNKNOWN);
			g_timers[ID_TIMER_UNKNOWN] = 0;
			break;
		}
		break;

	case WM_POWERBROADCAST:
		if (!isPowerSirenRunning || !isRunning)
			return TRUE;
		if (PBT_APMPOWERSTATUSCHANGE == wParam) {
			SYSTEM_POWER_STATUS sps = {};
			if (0 == GetSystemPowerStatus(&sps)) {
				if (0 == g_timers[ID_TIMER_UNKNOWN]) {
					g_timers[ID_TIMER_UNKNOWN] = SetTimer(hWnd, ID_TIMER_UNKNOWN, g_timethres, NULL);
					g_sendSt[ID_TIMER_UNKNOWN] = 0;
				}
				return TRUE;
			}
			else {
				if (0 != g_timers[ID_TIMER_UNKNOWN]) {
					KillTimer(hWnd, ID_TIMER_UNKNOWN);
					g_timers[ID_TIMER_UNKNOWN] = 0;
				}
			}
			switch (sps.ACLineStatus) {
			case 0:
				//MessageBoxW(hWnd, L"电池", L"TEST", 0);
				if (0 != g_timers[ID_TIMER_AC]) {
					KillTimer(hWnd, ID_TIMER_AC);
					g_timers[ID_TIMER_AC] = 0;
				}
				if (0 == g_timers[ID_TIMER_DC]) {
					g_timers[ID_TIMER_DC] = SetTimer(hWnd, ID_TIMER_DC, g_timethres, NULL);
					g_sendSt[ID_TIMER_DC] = 0;
				}
				if (0 != g_timers[ID_TIMER_UNKNOWN]) {
					KillTimer(hWnd, ID_TIMER_UNKNOWN);
					g_timers[ID_TIMER_UNKNOWN] = 0;
				}
				break;
			case 1:
				//MessageBoxW(hWnd, L"插电", L"TEST", 0);
				if (0 == g_timers[ID_TIMER_AC]) {
					g_timers[ID_TIMER_AC] = SetTimer(hWnd, ID_TIMER_AC, g_timethres, NULL);
					g_sendSt[ID_TIMER_AC] = 0;
				}
				if (0 != g_timers[ID_TIMER_DC]) {
					KillTimer(hWnd, ID_TIMER_DC);
					g_timers[ID_TIMER_DC] = 0;
				}
				if (0 != g_timers[ID_TIMER_UNKNOWN]) {
					KillTimer(hWnd, ID_TIMER_UNKNOWN);
					g_timers[ID_TIMER_UNKNOWN] = 0;
				}
				break;
			case 255:
				//MessageBoxW(hWnd, L"未知", L"TEST", 0);
				if (0 != g_timers[ID_TIMER_AC]) {
					KillTimer(hWnd, ID_TIMER_AC);
					g_timers[ID_TIMER_AC] = 0;
				}
				if (0 != g_timers[ID_TIMER_DC]) {
					KillTimer(hWnd, ID_TIMER_DC);
					g_timers[ID_TIMER_DC] = 0;
				}
				if (0 == g_timers[ID_TIMER_UNKNOWN]) {
					g_timers[ID_TIMER_UNKNOWN] = SetTimer(hWnd, ID_TIMER_UNKNOWN, g_timethres, NULL);
					g_sendSt[ID_TIMER_UNKNOWN] = 0;
				}
				break;
			}
		}
		return TRUE;

	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
	return 0;
}

} // namespace

void MyLoadString(HINSTANCE hInst) {
	// 初始化字符串
	LoadStringW(hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInst, IDC_TYLERTOYS, szWindowClass, MAX_LOADSTRING);

	LoadStringW(hInst, IDS_MAINBTNRUN, szMainBtnRun, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_MAINBTNSTOP, szMainBtnStop, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_ENABLED, szEnabled, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_HIDEONSTARTING, szHideOnStarting, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_KEEPDEVICE, szDeviceKeep, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_KEEPDISPLAY, szDisplayKeep, MAX_LOADSTRING);

	LoadStringW(hInst, IDS_SENDERPATH, szSenderPath, MAX_LOADSTRING);

	LoadStringW(hInst, IDS_HASBEENDCFOR1MIN, szPowerTextDC, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_HASBEENACFOR1MIN, szPowerTextAC, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_POWERINFOERRORTEXT, szPowerTextERR, MAX_LOADSTRING);

	LoadStringW(hInst, IDS_POWERTITLEAC, szPowerTitleAC, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_POWERTITLEDC, szPowerTitleDC, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_POWERTITLEERR, szPowerTitleERR, MAX_LOADSTRING);

	AppNameW.assign(szTitle);

	CHAR szTitleA[MAX_LOADSTRING];
	LoadStringA(hInst, IDS_APP_TITLE, szTitleA, MAX_LOADSTRING);

	AppNameA.assign(szTitleA);

	DWORD size = MAX_LOADSTRING;
	GetComputerNameW(szLocalDeviceName, &size);
	return;
}

bool MyRegisterClass(HINSTANCE hInst) {
	WNDCLASSEXW wcex{ 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInst;
	wcex.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_TYLERTOYS));
	wcex.hIconSm = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_SMALL));
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszClassName = szWindowClass;
	return RegisterClassExW(&wcex);
}

HWND MyCreateWindow(HINSTANCE hInst, int nCmdShow) {
	HWND hWnd = CreateWindowExW(
		WS_EX_APPWINDOW,
		szWindowClass, szTitle,
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, 446, 270,
		NULL, NULL, hInst, NULL
	);
	if (!hWnd)
		return NULL;
	ShowWindow(hWnd, RegistrySettings::instance()->IsHideWindowOnStarting() ? SW_HIDE : nCmdShow);
	UpdateWindow(hWnd);
	return hWnd;
}

bool MyAddNotifyIcon(HINSTANCE hInst, HWND hWnd) {
	NOTIFYICONDATA nid{ 0 };
	nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uVersion = NOTIFYICON_VERSION_4;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_MESSAGE;
	nid.uCallbackMessage = MYWM_CALLBACK;
	nid.uID = NOTIFYICON_ID;
	// This text will be shown as the icon's tooltip.
	StringCchCopyW(nid.szTip, ARRAYSIZE(nid.szTip), szTitle);

	nid.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_SMALL));

	// Add the icon
	if (Shell_NotifyIconW(NIM_ADD, &nid) == FALSE)
		return false;
	// Set the version
	if (Shell_NotifyIconW(NIM_SETVERSION, &nid) == FALSE)
		return false;
	return true;
}

void MyRemoveNotifyIcon() {
	NOTIFYICONDATA nid{ 0 };
	nid.cbSize = sizeof(nid);
	nid.uID = NOTIFYICON_ID;
	Shell_NotifyIconW(NIM_DELETE, &nid);
	return;
}
