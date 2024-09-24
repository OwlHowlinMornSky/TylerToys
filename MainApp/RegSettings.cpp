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
#include "RegSettings.h"

#include "framework.h"
#include <CommCtrl.h>
#include <winreg.h>
#include "AppGlobal.h"
#include "WinCheck.h"
#include <cassert>
#include <memory>

namespace {

bool LoadBoolFromReg(std::wstring_view name, bool& value) {
	DWORD type = REG_DWORD;
	DWORD data = {};
	DWORD dataSize = sizeof(data);
	HKEY key = {};
	LSTATUS status = {};
	status = RegCreateKeyExW(
		HKEY_CURRENT_USER,
		L"SOFTWARE\\OHMS\\TylerToys\\Settings",
		NULL,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&key,
		NULL
	);
	if (status != 0) {
		ParseErrorCode(status, AppNameW + L": Failed to Open Registry Key");
		return false;
	}
	status = RegGetValueW(
		key,
		NULL,
		name.data(),
		RRF_RT_REG_DWORD,
		&type,
		&data,
		&dataSize
	);
	bool success = false;
	switch (status) {
	case ERROR_SUCCESS:
		value = data;
		success = true;
		break;
	case ERROR_FILE_NOT_FOUND:
		type = REG_DWORD;
		data = 0;
		dataSize = sizeof(data);
		RegSetValueExW(
			key,
			name.data(),
			NULL,
			type,
			(PBYTE)&data,
			dataSize
		);
		value = false;
		success = true;
		break;
	case ERROR_MORE_DATA:
	case ERROR_INVALID_PARAMETER:
	default:
		ParseErrorCode(status, AppNameW + L": Failed to Read Registry Value \'" + name.data() + L"\'");
		break;
	}
	RegCloseKey(key);
	return success;
}

bool SaveBoolToReg(std::wstring_view name, bool value) {
	DWORD type = REG_DWORD;
	DWORD data = value;
	DWORD dataSize = sizeof(data);
	HKEY key = {};
	LSTATUS status = {};
	status = RegCreateKeyExW(
		HKEY_CURRENT_USER,
		L"SOFTWARE\\OHMS\\TylerToys\\Settings",
		NULL,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&key,
		NULL
	);
	if (status != 0) {
		ParseErrorCode(status, AppNameW + L": Failed to Open Registry Key");
		return false;
	}
	status = RegSetValueExW(
		key,
		name.data(),
		NULL,
		type,
		(PBYTE)&data,
		dataSize
	);
	RegCloseKey(key);
	return true;
}

bool LoadCodeFromReg(uint16_t& value) {
	DWORD type = REG_DWORD;
	DWORD data = {};
	DWORD dataSize = sizeof(data);
	HKEY key = {};
	LSTATUS status = {};
	status = RegCreateKeyExW(
		HKEY_CURRENT_USER,
		L"SOFTWARE\\OHMS\\TylerToys\\Settings",
		NULL,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&key,
		NULL
	);
	if (status != 0) {
		ParseErrorCode(status, AppNameW + L": Failed to Open Registry Key");
		return false;
	}
	status = RegGetValueW(
		key,
		NULL,
		L"HotKeyCode",
		RRF_RT_REG_DWORD,
		&type,
		&data,
		&dataSize
	);
	bool success = false;
	switch (status) {
	case ERROR_SUCCESS:
		value = (WORD)data;
		success = true;
		break;
	case ERROR_FILE_NOT_FOUND:
		value = MAKEWORD('M', HOTKEYF_CONTROL);
		type = REG_DWORD;
		data = value;
		dataSize = sizeof(data);
		RegSetValueExW(
			key,
			L"HotKeyCode",
			NULL,
			type,
			(PBYTE)&data,
			dataSize
		);
		success = true;
		break;
	case ERROR_MORE_DATA:
	case ERROR_INVALID_PARAMETER:
	default:
		ParseErrorCode(status, AppNameW + L": Failed to Read Registry Value \'HotKeyCode\'");
		break;
	}
	RegCloseKey(key);
	return success;
}

bool SaveCodeToReg(uint16_t value) {
	DWORD type = REG_DWORD;
	DWORD data = value;
	DWORD dataSize = sizeof(data);
	HKEY key = {};
	LSTATUS status = {};
	status = RegCreateKeyExW(
		HKEY_CURRENT_USER,
		L"SOFTWARE\\OHMS\\TylerToys\\Settings",
		NULL,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&key,
		NULL
	);
	if (status != 0) {
		ParseErrorCode(status, AppNameW + L": Failed to Open Registry Key");
		return false;
	}
	status = RegSetValueExW(
		key,
		L"HotKeyCode",
		NULL,
		type,
		(PBYTE)&data,
		dataSize
	);
	RegCloseKey(key);
	return true;
}

std::unique_ptr<RegistrySettings> g_instance;

}

RegistrySettings* RegistrySettings::instance() {
	if (nullptr == g_instance)
		g_instance = std::unique_ptr<RegistrySettings>(new RegistrySettings());
	return g_instance.get();
}

RegistrySettings::RegistrySettings() {
	Load();
}

RegistrySettings::~RegistrySettings() {
	Save();
}

bool RegistrySettings::IsAllAvailable() const {
    return m_available.count() == SettingId::COUNT;
}

bool RegistrySettings::IsHideWindowOnStarting() const {
	return (bool)m_settings[SettingId::isHideWindowOnStarting];
}

void RegistrySettings::SetHideWindowOnStarting(bool newval) {
	m_settings[SettingId::isHideWindowOnStarting] = newval;
	m_available[SettingId::isHideWindowOnStarting] = true;
	m_changed[SettingId::isHideWindowOnStarting] = true;
}

bool RegistrySettings::IsPowerSirenEnabled() const {
	return (bool)m_settings[SettingId::isPowerSirenEnabled];
}

void RegistrySettings::SetPowerSirenEnabled(bool enabled) {
	m_settings[SettingId::isPowerSirenEnabled] = enabled;
	m_available[SettingId::isPowerSirenEnabled] = true;
	m_changed[SettingId::isPowerSirenEnabled] = true;
}

bool RegistrySettings::IsDisplayKeeperEnabled() const {
	return (bool)m_settings[SettingId::isDisplayKeeperEnabled];
}

void RegistrySettings::SetDisplayKeeperEnabled(bool enabled) {
	m_settings[SettingId::isDisplayKeeperEnabled] = enabled;
	m_available[SettingId::isDisplayKeeperEnabled] = true;
	m_changed[SettingId::isDisplayKeeperEnabled] = true;
}

bool RegistrySettings::IsDeviceKeeperEnabled() const {
	return (bool)m_settings[SettingId::isDeviceKeeperEnabled];
}

void RegistrySettings::SetDeviceKeeperEnabled(bool enabled) {
	m_settings[SettingId::isDeviceKeeperEnabled] = enabled;
	m_available[SettingId::isDeviceKeeperEnabled] = true;
	m_changed[SettingId::isDeviceKeeperEnabled] = true;
}

bool RegistrySettings::IsMuteHotkeyEnabled() const {
	return (bool)m_settings[SettingId::isMuteHotkeyEnabled];
}

void RegistrySettings::SetMuteHotkeyEnabled(bool enabled) {
	m_settings[SettingId::isMuteHotkeyEnabled] = enabled;
	m_available[SettingId::isMuteHotkeyEnabled] = true;
	m_changed[SettingId::isMuteHotkeyEnabled] = true;
}

uint16_t RegistrySettings::GetMuteKey() const {
	return (uint16_t)m_settings[SettingId::muteKey];
}

void RegistrySettings::SetMuteKey(uint16_t code) {
	m_settings[SettingId::muteKey] = code;
	m_available[SettingId::muteKey] = true;
	m_changed[SettingId::muteKey] = true;
}

void RegistrySettings::Load() {
	bool load;

	if (LoadBoolFromReg(L"HideAfterStart", load)) {
		m_settings[SettingId::isHideWindowOnStarting] = load;
		m_available[SettingId::isHideWindowOnStarting] = true;
	}

	if (LoadBoolFromReg(L"EnablePowerSiren", load)) {
		m_settings[SettingId::isPowerSirenEnabled] = load;
		m_available[SettingId::isPowerSirenEnabled] = true;
	}

	if (LoadBoolFromReg(L"EnableDisplayKeeper", load)) {
		m_settings[SettingId::isDisplayKeeperEnabled] = load;
		m_available[SettingId::isDisplayKeeperEnabled] = true;
	}
	if (LoadBoolFromReg(L"EnableDeviceKeeper", load)) {
		m_settings[SettingId::isDeviceKeeperEnabled] = load;
		m_available[SettingId::isDeviceKeeperEnabled] = true;
	}

	if (LoadBoolFromReg(L"EnableMuteHotkey", load)) {
		m_settings[SettingId::isMuteHotkeyEnabled] = load;
		m_available[SettingId::isMuteHotkeyEnabled] = true;
	}

	uint16_t load16;
	if (LoadCodeFromReg(load16)) {
		m_settings[SettingId::muteKey] = load16;
		m_available[SettingId::muteKey] = true;
	}

	return;
}

void RegistrySettings::Save() {
	if (m_changed.none())
		return;

	if (m_changed.test(0) && SaveBoolToReg(L"HideAfterStart", m_settings[SettingId::isHideWindowOnStarting]))
		0;

	if (m_changed.test(1) && SaveBoolToReg(L"EnablePowerSiren", m_settings[SettingId::isPowerSirenEnabled]))
		0;

	if (m_changed.test(2) && SaveBoolToReg(L"EnableDisplayKeeper", m_settings[SettingId::isDisplayKeeperEnabled]))
		0;
	if (m_changed.test(3) && SaveBoolToReg(L"EnableDeviceKeeper", m_settings[SettingId::isDeviceKeeperEnabled]))
		0;

	if (m_changed.test(4) && SaveBoolToReg(L"EnableMuteHotkey", m_settings[SettingId::isMuteHotkeyEnabled]))
		0;

	if (m_changed.test(5) && SaveCodeToReg(m_settings[SettingId::muteKey]))
		0;

	m_changed.reset();
	return;
}
