//This file is part of JSLint Plugin for Notepad++
//Copyright (C) 2010 Martin Vladic <martin.vladic@gmail.com>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#define WIN32_LEAN_AND_MEAN

#include "TargetVer.h"

#include <Windows.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <Shellapi.h>
#include <Shlwapi.h>
#include <zmouse.h>
#include <Winhttp.h>

#pragma warning(disable:4996)

#include <string>
#include <tchar.h>
#include <time.h>
#include <list>
#include <map>
#include <sstream>
#include <vector>
#include <memory>
#include <algorithm>

using namespace std;

#if defined(UNICODE) || defined(_UNICODE)
typedef wstring tstring;
#else
typedef string tstring;
#endif
