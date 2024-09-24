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
#include "framework.h"
#include "Resource.h"
#include "Win32.h"
#include "AppGlobal.h"
#include "WinCheck.h"
#include "RegSettings.h"
#include "Mute.h"

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow
) {

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	MyLoadString(hInstance);

	if (!MyRegisterClass(hInstance)) {
		ParseWin32Error(AppNameW + L": Failed to Register Window Class");
		return 1;
	}
	HWND hwnd = MyCreateWindow(hInstance, nCmdShow);
	if (hwnd == NULL) {
		ParseWin32Error(AppNameW + L": Failed to Create Window");
		return 1;
	}
	if (!MyAddNotifyIcon(hInstance, hwnd)) {
		MessageBoxW(NULL, L"Failed to Create Notify Icon!", AppNameW.data(), MB_ICONERROR);
		return 1;
	}

	HPOWERNOTIFY hPN = RegisterPowerSettingNotification(hwnd, &GUID_ACDC_POWER_SOURCE, DEVICE_NOTIFY_WINDOW_HANDLE);

	MSG msg = {};
	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	UnregisterPowerSettingNotification(hPN);

	MyRemoveNotifyIcon();
	DestroyWindow(hwnd);

	return 0;
}
