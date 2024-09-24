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

#include "framework.h"

void MyLoadString(HINSTANCE hInst);

bool MyRegisterClass(HINSTANCE hInst);

HWND MyCreateWindow(HINSTANCE hInst, int nCmdShow);

/**
 * @brief 添加托盘图标
 * @param hInst 当前实例
 * @param hWnd 所属窗口
 * @return 是否成功
*/
bool MyAddNotifyIcon(HINSTANCE hInst, HWND hWnd);

/*
*@brief 移除托盘图标
*/
void MyRemoveNotifyIcon();
