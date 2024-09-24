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
#include "ContextMenu.h"

ContextMenu::ContextMenu() {
	m_pPopMenu = CreatePopupMenu();
	if (m_pPopMenu) {
		InsertMenuW(m_pPopMenu, (-1), MF_BYPOSITION, Item::Exit, L"Exit");
	}
}

ContextMenu::~ContextMenu() {
	if (m_pPopMenu) {
		DestroyMenu(m_pPopMenu);
		m_pPopMenu = NULL;
	}
}

void ContextMenu::Pop(HWND hwnd, int nX, int nY) {
	if (m_pPopMenu && hwnd) {
		SetForegroundWindow(hwnd);
		TrackPopupMenu(
			m_pPopMenu,
			TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON | TPM_VERNEGANIMATION,
			nX, nY, NULL, hwnd, NULL
		);
		PostMessageW(hwnd, WM_NULL, 0, 0);
	}
}

void ContextMenu::Pop(HWND hwnd, const POINT& pt) {
	Pop(hwnd, pt.x, pt.y);
}
