﻿/*
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

#include "framework.h"

class ContextMenu {
public:
	enum Item {
		Exit = 40001,
		COUNT
	};

public:
	ContextMenu();
	~ContextMenu();

	void Pop(HWND hwnd, int nX, int nY);
	void Pop(HWND hwnd, const POINT& pt);

private:
	HMENU m_pPopMenu;
};
