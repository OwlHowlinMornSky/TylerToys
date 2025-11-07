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
/*
* 
* https://cloud.tencent.com/developer/article/2350174
* https://learn.microsoft.com/zh-cn/windows/win32/api/winuser/nf-winuser-setwineventhook
* 
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

struct MSCOM_DELETER {
	void operator()(IUnknown* object) const {
		object->Release();
	}
};

class MSCOM_RECEIVER {
public:
	MSCOM_RECEIVER() :
		m_pointer(nullptr) {}

	~MSCOM_RECEIVER() {
		if (m_pointer)
			m_pointer->Release();
	}

	template<typename _Ty>
	_Ty** ready() {
		return (_Ty**)&m_pointer;
	}

	template<typename _T2>
	operator _T2() {
		_T2 res = (_T2)m_pointer;
		m_pointer = nullptr;
		return res;
	}

	template<typename _Tt, typename _Tf>
	HRESULT QueryInterfaceFrom(_Tf& f) {
		return f->QueryInterface<_Tt>((_Tt**)&m_pointer);
	}

	template<typename _Ty>
	std::unique_ptr<_Ty, MSCOM_DELETER> get() {
		return std::unique_ptr<_Ty, MSCOM_DELETER>((_Ty*)(*this), MSCOM_DELETER());
	}

	template<typename _Ty>
	std::shared_ptr<_Ty> get_shared() {
		return std::shared_ptr<_Ty>((_Ty*)(*this), MSCOM_DELETER());
	}

protected:
	IUnknown* m_pointer;
};

static const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
static const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
static const IID IID_IAudioSessionManager2 = __uuidof(IAudioSessionManager2);

void DoSessionCtrl(std::shared_ptr<IAudioSessionControl> pSessionCtrl, DWORD pid, bool type = false) {
	MSCOM_RECEIVER receiver;

	if (S_OK != receiver.QueryInterfaceFrom<IAudioSessionControl2>(pSessionCtrl))
		return;
	auto ctrl = receiver.get<IAudioSessionControl2>();
	if (type == false && S_OK == ctrl->IsSystemSoundsSession()) // 不操作系统会话
		return;

	if (S_OK != receiver.QueryInterfaceFrom<ISimpleAudioVolume>(pSessionCtrl))
		return;
	auto vol = receiver.get<ISimpleAudioVolume>();

	DWORD apid = NULL;
	if (ctrl->GetProcessId(&apid) != S_OK)
		return;
	if (type == false) {
		if (apid != pid)
			return;
		BOOL muted = {};
		if (vol->GetMute(&muted) != S_OK)
			return;
		vol->SetMute(!muted, NULL);
	}
	else {
		if (apid != pid)
			vol->SetMute(TRUE, NULL);
		else
			vol->SetMute(FALSE, NULL);
	}
	return;
}

void DeviceEnumSession(std::shared_ptr<IMMDevice> pDevice, DWORD pid, bool type = false) {
	MSCOM_RECEIVER receiver;

	if (S_OK != pDevice->Activate(IID_IAudioSessionManager2, CLSCTX_ALL, NULL, receiver.ready<void>()))
		return;
	auto pSessionMngr = receiver.get<IAudioSessionManager2>();

	if (S_OK != pSessionMngr->GetSessionEnumerator(receiver.ready<IAudioSessionEnumerator>()))
		return;
	auto pSessionEnum = receiver.get<IAudioSessionEnumerator>();

	int cnt = 0;
	if (S_OK != pSessionEnum->GetCount(&cnt))
		return;
	for (int i = 0; i < cnt; ++i) {
		if (S_OK != pSessionEnum->GetSession(i, receiver.ready<IAudioSessionControl>()))
			continue;
		auto pSessionCtrl = receiver.get_shared<IAudioSessionControl>();
		DoSessionCtrl(pSessionCtrl, pid, type);
	}

	return;
}

void EnumDevice(DWORD pid, bool type = false) {
	MSCOM_RECEIVER receiver;

	if (S_OK != CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, receiver.ready<void>()))
		return;
	auto pDevEnum = receiver.get<IMMDeviceEnumerator>();

	if (S_OK != pDevEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, receiver.ready<IMMDeviceCollection>()))
		return;
	auto pDevCol = receiver.get<IMMDeviceCollection>();

	UINT cnt = 0;
	if (S_OK != pDevCol->GetCount(&cnt))
		return;
	for (UINT i = 0; i < cnt; ++i) {
		if (S_OK != pDevCol->Item(i, receiver.ready<IMMDevice>()))
			continue;
		auto pDevice = receiver.get_shared<IMMDevice>();
		DeviceEnumSession(pDevice, pid, type);
	}

	return;
}

std::unique_ptr<MuteWindow> g_instance;
std::unique_ptr<OnlyForeground> g_instfore;

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
		if (nullptr == g_instfore && false == InitCOM()) {
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
	DWORD pid = NULL;
	if (GetWindowThreadProcessId(hwnd, &pid) == 0)
		return;
	try {
		EnumDevice(pid);
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

OnlyForeground* OnlyForeground::instance() {
	if (nullptr == g_instance) {
		if (nullptr == g_instance && false == InitCOM()) {
			return nullptr;
		}
		g_instfore = std::unique_ptr<OnlyForeground>(new OnlyForeground());
	}
	return g_instfore.get();
}

OnlyForeground::OnlyForeground() {}

OnlyForeground::~OnlyForeground() {}

void OnlyForeground::Trigger() {
	HWND hwnd = GetForegroundWindow();
	if (hwnd == NULL)
		return;
	DWORD pid = NULL;
	if (GetWindowThreadProcessId(hwnd, &pid) == 0)
		return;
	try {
		EnumDevice(pid, true);
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
