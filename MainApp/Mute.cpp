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

#include <combaseapi.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <audioclient.h>

#pragma comment(lib,"Strmiids.lib") 

#include <exception>
#include <memory>
#include "AppGlobal.h"
#include "Mute.h"
#include "WinCheck.h"

namespace {

static const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
static const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
static const IID IID_IAudioSessionManager2 = __uuidof(IAudioSessionManager2);

DWORD pid = NULL;

void DoSessionCtrl(IAudioSessionControl* pSessionCtrl) {
	HRESULT hr = {};
	IAudioSessionControl2* ctrl = nullptr;
	hr = pSessionCtrl->QueryInterface<IAudioSessionControl2>(&ctrl);
	if (hr != S_OK)
		return;
	if (ctrl->IsSystemSoundsSession() == S_OK) // 不操作系统会话
		return;
	ISimpleAudioVolume* vol = nullptr;
	hr = pSessionCtrl->QueryInterface<ISimpleAudioVolume>(&vol);
	if (hr != S_OK)
		return;
	DWORD apid = NULL;
	if (ctrl->GetProcessId(&apid) != S_OK)
		return;
	if (apid != pid)
		return;
	BOOL muted = {};
	if (vol->GetMute(&muted) != S_OK)
		return;
	vol->SetMute(!muted, NULL);
	return;
}

void DeviceEnumSession(IMMDevice* pDevice) {
	HRESULT hr = {};
	IAudioSessionManager2* pSessionMngr = nullptr;
	hr = pDevice->Activate(IID_IAudioSessionManager2, CLSCTX_ALL, NULL, (void**)&pSessionMngr);
	if (hr != S_OK)
		return;
	IAudioSessionEnumerator* pSessionEnum = nullptr;
	hr = pSessionMngr->GetSessionEnumerator(&pSessionEnum);
	if (hr == S_OK) {
		int cnt = 0;
		hr = pSessionEnum->GetCount(&cnt);
		if (hr == S_OK) {
			for (int i = 0; i < cnt; ++i) {
				IAudioSessionControl* pSessionCtrl = nullptr;
				hr = pSessionEnum->GetSession(i, &pSessionCtrl);
				if (hr == S_OK)
					DoSessionCtrl(pSessionCtrl);
				if (pSessionCtrl)
					pSessionCtrl->Release();
			}
		}
		pSessionEnum->Release();
	}
	pSessionMngr->Release();
}

void EnumDevice() {
	HRESULT hr = {};
	IMMDeviceEnumerator* pDevEnum = nullptr;
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pDevEnum);
	if (hr != S_OK)
		return;
	IMMDeviceCollection* pDevCol = nullptr;
	hr = pDevEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevCol);
	if (hr == S_OK) {
		UINT cnt = 0;
		hr = pDevCol->GetCount(&cnt);
		if (hr == S_OK) {
			for (UINT i = 0; i < cnt; ++i) {
				IMMDevice* pDevice = nullptr;
				hr = pDevCol->Item(i, &pDevice);
				if (hr == S_OK)
					DeviceEnumSession(pDevice);
				if (pDevice)
					pDevice->Release();
			}
		}
	}
	if (pDevCol)
		pDevCol->Release();
	pDevEnum->Release();
	return;
}

std::unique_ptr<MuteWindow> g_instance;

bool InitCOM() {
	HRESULT hr = {};
	//hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); // Thread Without UI
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // Thread With UI
	switch (hr) {
	case S_OK:
	case S_FALSE:
		break;
	case RPC_E_CHANGED_MODE:
		ParseErrorCode(hr, AppNameW + L": COM Warning");
		break;
	case E_INVALIDARG:
	case E_OUTOFMEMORY:
	case E_UNEXPECTED:
	default:
		ParseErrorCode(hr, AppNameW + L": Failed to Initialize COM");
		return false;
	}
	return true;
}

void DropCOM() {
	CoUninitialize();
}

} // namespace

MuteWindow* MuteWindow::instance() {
	if (nullptr == g_instance) {
		if (false == InitCOM()) {
			return nullptr;
		}
		g_instance = std::unique_ptr<MuteWindow>(new MuteWindow());
	}
	return g_instance.get();
}

MuteWindow::MuteWindow() {}

MuteWindow::~MuteWindow() {
	DropCOM();
}

void MuteWindow::TryMuteOrUnmuteForegroundWindow() {
	HWND hwnd = GetForegroundWindow();
	if (hwnd == NULL)
		return;
	pid = NULL;
	if (GetWindowThreadProcessId(hwnd, &pid) == 0)
		return;
	try {
		EnumDevice();
	}
	catch (std::exception& e) {
		MessageBoxA(NULL, e.what(), (AppNameA + ": Exception").data(), MB_ICONERROR);
	}
	catch (int code) {
		MessageBoxA(NULL, ("Code: " + std::to_string(code)).data(), (AppNameA + ": Exception").data(), MB_ICONERROR);
	}
	catch (...) {
		MessageBoxA(NULL, "Unknown Exception.", (AppNameA + ": Exception").data(), MB_ICONERROR);
	}
	pid = NULL;
	return;
}
