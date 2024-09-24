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
#include "WinCheck.h"
#include "framework.h"

#include <string>

void ParseErrorCode(long code, std::wstring_view errorText) {
	DWORD lasterrcode = code;
	LPWSTR pBuffer = NULL;
	std::wstring msgstr_en, msgstr_user;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, lasterrcode, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR)&pBuffer, 0, NULL);
	if (pBuffer) {
		pBuffer[lstrlenW(pBuffer) - 2] = '\0';
		msgstr_en.append(pBuffer);
		LocalFree(pBuffer);
	}
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, lasterrcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&pBuffer, 0, NULL);
	if (pBuffer) {
		pBuffer[lstrlenW(pBuffer) - 2] = '\0';
		msgstr_user.append(pBuffer);
		LocalFree(pBuffer);
	}
	if ((!msgstr_user.empty()) && (msgstr_user != msgstr_en)) {
		msgstr_en.append(L"\r\n");
		msgstr_en.append(msgstr_user);
	}
	else if (msgstr_en.empty()) {
		msgstr_en.assign(L"NULL.");
	}
	MessageBoxW(NULL, msgstr_en.data(), errorText.data(), MB_ICONERROR);
	return;
}

void ParseWin32Error(std::wstring_view errorText) {
	return ParseErrorCode(GetLastError(), errorText);
}
