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
#include "AboutDlg.h"
#include "PluginDefinition.h"
#include "resource.h"
#include "Util.h"
#include "Version.h"

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	if (uMessage == WM_INITDIALOG) {
		HWND hWndVersionStatic = ::GetDlgItem(hDlg, IDC_VERSION_STATIC);
		
		TCHAR szVersionFormat[50];
		GetWindowText(hWndVersionStatic, szVersionFormat, _countof(szVersionFormat));

#if defined(UNICODE) || defined(_UNICODE)
		LPCTSTR szCharSet = TEXT("Unicode");
#else
		LPCTSTR szCharSet = TEXT("ANSI");
#endif		

		TCHAR szVersion[100];
		_stprintf(szVersion, szVersionFormat, szCharSet, MY_PRODUCT_VERSION);

		SetWindowText(hWndVersionStatic, szVersion);

        CenterWindow(hDlg, g_nppData._nppHandle);
    } else if (uMessage == WM_COMMAND) {
		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
				case IDOK: {
					EndDialog(hDlg, 1);
					return 1;
				}
				case IDCANCEL: {
					EndDialog(hDlg, 0);
					return 1;
				}
			}
		}
    } else if (uMessage == WM_NOTIFY) {
		switch (((LPNMHDR)lParam)->code)
		{
			case NM_CLICK:
			case NM_RETURN: {
				PNMLINK pNMLink = (PNMLINK)lParam;
				LITEM item = pNMLink->item;
				ShellExecuteW(NULL, L"open", item.szUrl, NULL, NULL, SW_SHOW);
				break;
			}
		}
	} else if (uMessage == WM_SYSCOMMAND) {
        if (wParam == SC_CLOSE) {
            // cancel
            EndDialog(hDlg, 0);
            return 1;
        }
    }

	return 0;
}

