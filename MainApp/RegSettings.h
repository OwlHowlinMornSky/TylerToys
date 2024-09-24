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
#pragma once

#include <stdint.h>
#include <bitset>
#include <array>

class RegistrySettings final {
public:
	static RegistrySettings* instance();

private:
	RegistrySettings();
public:
	~RegistrySettings();

public:
	bool IsAllAvailable() const;

	bool IsHideWindowOnStarting() const;
	void SetHideWindowOnStarting(bool newval);

	bool IsPowerSirenEnabled() const;
	void SetPowerSirenEnabled(bool enabled);

	bool IsDisplayKeeperEnabled() const;
	void SetDisplayKeeperEnabled(bool enabled);

	bool IsDeviceKeeperEnabled() const;
	void SetDeviceKeeperEnabled(bool enabled);

	bool IsMuteHotkeyEnabled() const;
	void SetMuteHotkeyEnabled(bool enabled);

	uint16_t GetMuteKey() const;
	void SetMuteKey(uint16_t code);

protected:
	void Load();
	void Save();

protected:
	enum SettingId {
		isHideWindowOnStarting = 0,

		isPowerSirenEnabled,

		isDisplayKeeperEnabled,
		isDeviceKeeperEnabled,

		isMuteHotkeyEnabled,
		muteKey,

		COUNT
	};

	std::bitset<SettingId::COUNT> m_available;
	std::bitset<SettingId::COUNT> m_changed;
	std::array<int32_t, SettingId::COUNT> m_settings;
};
