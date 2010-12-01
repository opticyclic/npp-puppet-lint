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

#include "StdHeaders.h"
#include "Util.h"

////////////////////////////////////////////////////////////////////////////////

BOOL CenterWindow(HWND hWnd, HWND hParentWnd, BOOL bRepaint)
{
  RECT rectParent;
  RECT rect;
  INT  height, width;
  INT  x, y;

  ::GetWindowRect(hParentWnd, &rectParent);
  ::GetWindowRect(hWnd, &rect);
  width = rect.right - rect.left;
  height = rect.bottom - rect.top;
  x = ((rectParent.right - rectParent.left) - width) / 2;
  x += rectParent.left;
  y = ((rectParent.bottom - rectParent.top) - height) / 2;
  y += rectParent.top;
  return ::MoveWindow(hWnd, x, y, width, height, bRepaint);
}

tstring TrimSpaces(const tstring& str)
{
    size_t first = str.find_first_not_of(TEXT(" \t"));
    size_t last = str.find_last_not_of(TEXT(" \t"));
 
    if (first == string::npos || last == string::npos)
		return tstring();

	return str.substr(first, last - first + 1 );
}

////////////////////////////////////////////////////////////////////////////////

tstring Path::GetDirectoryName(const tstring& strPath)
{
	TCHAR szPathCopy[MAX_PATH];
	_tcsncpy_s(szPathCopy, _countof(szPathCopy), strPath.c_str(), _TRUNCATE);

	::PathRemoveFileSpec(szPathCopy);

	return szPathCopy;
}

tstring Path::GetFileName(const tstring& strPath)
{
	return ::PathFindFileName(strPath.c_str());
}

tstring Path::GetFileNameWithoutExtension(const tstring& strPath)
{
	TCHAR szPathCopy[MAX_PATH];
	_tcsncpy_s(szPathCopy, _countof(szPathCopy), GetFileName(strPath).c_str(), _TRUNCATE);

	::PathRemoveExtension(szPathCopy);

	return szPathCopy;
}

tstring Path::GetPathNameWithoutExtension(const tstring& strPath)
{
	TCHAR szPathCopy[MAX_PATH];
	_tcsncpy_s(szPathCopy, _countof(szPathCopy), strPath.c_str(), _TRUNCATE);

	::PathRemoveExtension(szPathCopy);

	return szPathCopy;
}

tstring Path::GetExtension(const tstring& strPath)
{
	return ::PathFindExtension(strPath.c_str());
}

tstring Path::GetFullPath(const tstring& strPath)
{
	TCHAR strCurrentDirectory[MAX_PATH];
	if (GetCurrentDirectory(MAX_PATH, strCurrentDirectory) == 0)
		return TEXT("");
	return GetFullPath(strPath, strCurrentDirectory);
}

tstring Path::GetFullPath(const tstring& strPath, const tstring& strBaseDir)
{
	if (!Path::IsRelative(strPath))
		return strPath;
	TCHAR strFullPath[MAX_PATH];
	return ::PathCombine(strFullPath, strBaseDir.c_str(), strPath.c_str());
}

bool Path::IsRelative(const tstring& strPath)
{
	return ::PathIsRelative(strPath.c_str()) ? true : false;
}

bool Path::IsDir(const tstring& strPath)
{
	return ::PathIsDirectory(strPath.c_str()) ? true : false;
}

bool Path::IsFileExists(const tstring& strPath)
{
	return ::PathFileExists(strPath.c_str()) ? true : false;
}

tstring Path::GetTempFileName()
{
    TCHAR szTempPath[MAX_PATH];	
	if (::GetTempPath(MAX_PATH, szTempPath) == 0) {
		return TEXT("");
	}

	TCHAR szTempFileName[MAX_PATH];  
	if (::GetTempFileName(szTempPath, NULL, 0, szTempFileName) == 0) {
		return TEXT("");
	}

	return szTempFileName;
}

tstring Path::GetModuleFileName(HMODULE hModule)
{
	TCHAR szPath[MAX_PATH] = { 0 };
	::GetModuleFileName(hModule, szPath, MAX_PATH);
	return szPath;
}
