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
#include "OptionsDlg.h"
#include "PluginDefinition.h"
#include "resource.h"
#include "Util.h"

////////////////////////////////////////////////////////////////////////////////

JSLintOptions g_jsLintOptions;

////////////////////////////////////////////////////////////////////////////////

static JSLintOptions jsLintOptions;

BOOL UpdateOptions(HWND hDlg, bool bSaveOrValidate, bool bShowErrorMessage)
{
	if (bSaveOrValidate) {
		for (UINT id = IDC_CHECK1; id <= IDC_CHECK29; ++id) {
			if (Button_GetCheck(GetDlgItem(hDlg, id))) {
				jsLintOptions.CheckOption(id);
			} else {
				jsLintOptions.UncheckOption(id);
			}
		}

		// indent
		if (jsLintOptions.IsOptionChecked(TEXT("white"))) {
			TCHAR szIdent[32];
			GetWindowText(GetDlgItem(hDlg, IDC_IDENT), szIdent, _countof(szIdent));
			tstring strIdent = TrimSpaces(szIdent);
			if (!strIdent.empty()) {
				int indent;
				if(_stscanf(strIdent.c_str(), TEXT("%d"), &indent) == EOF || indent < 1) {
					if (bShowErrorMessage) {
						MessageBox(hDlg, TEXT("Identation must be an integer greater than zero"), _T("JSLint"), MB_OK | MB_ICONERROR);
						SetFocus(GetDlgItem(hDlg, IDC_IDENT));
					}
					return FALSE;
				}

				jsLintOptions.SetOption(IDC_IDENT, strIdent);
			} else {
				jsLintOptions.ResetOption(IDC_IDENT);
			}
		} else {
			jsLintOptions.ResetOption(IDC_IDENT);
		}

		// maxlen
		TCHAR szMaxlen[32];
		GetWindowText(GetDlgItem(hDlg, IDC_MAXLEN), szMaxlen, _countof(szMaxlen));
		tstring strMaxlen = TrimSpaces(szMaxlen);
		if (!strMaxlen.empty()) {
			int maxlen;
			if(_stscanf(strMaxlen.c_str(), TEXT("%d"), &maxlen) == EOF || maxlen < 1) {
				if (bShowErrorMessage) {
					MessageBox(hDlg, TEXT("Maximum line length must be an integer greater than zero"), _T("JSLint"), MB_OK | MB_ICONERROR);
					SetFocus(GetDlgItem(hDlg, IDC_MAXLEN));
				}
				return FALSE;
			}
			
			jsLintOptions.SetOption(IDC_MAXLEN, strMaxlen);
		} else {
			jsLintOptions.ResetOption(IDC_MAXLEN);
		}

		// maxerr
		if (!jsLintOptions.IsOptionChecked(TEXT("passfail"))) {
			TCHAR szMaxerr[32];
			GetWindowText(GetDlgItem(hDlg, IDC_MAXERR), szMaxerr, _countof(szMaxerr));
			tstring strMaxerr = TrimSpaces(szMaxerr);
			if (!strMaxerr.empty()) {
				int maxerr;
				if(_stscanf(strMaxerr.c_str(), TEXT("%d"), &maxerr) == EOF || maxerr < 1) {
					if (bShowErrorMessage) {
						MessageBox(hDlg, TEXT("Maximum numer of errors must be an integer greater than zero"), _T("JSLint"), MB_OK | MB_ICONERROR);
						SetFocus(GetDlgItem(hDlg, IDC_MAXERR));
					}
					return FALSE;
				}
				
				jsLintOptions.SetOption(IDC_MAXERR, strMaxerr);
			} else {
				jsLintOptions.ResetOption(IDC_MAXERR);
			}
		} else {
			jsLintOptions.ResetOption(IDC_MAXERR);
		}
	} else {
		jsLintOptions.UpdateDialog(hDlg);
	}

	SetWindowText(GetDlgItem(hDlg, IDC_PREVIEW), 
		jsLintOptions.GetOptionsString().c_str());

	EnableWindow(GetDlgItem(hDlg, IDC_IDENT), 
		jsLintOptions.IsOptionChecked(TEXT("white")));

	EnableWindow(GetDlgItem(hDlg, IDC_MAXERR), 
		!jsLintOptions.IsOptionChecked(TEXT("passfail")));

	return TRUE;
}

INT_PTR CALLBACK OptionsDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	if (uMessage == WM_INITDIALOG) {
		jsLintOptions = g_jsLintOptions;
		UpdateOptions(hDlg, false, false);
        CenterWindow(hDlg, g_nppData._nppHandle);
    } else if (uMessage == WM_COMMAND) {
		if (HIWORD(wParam) == BN_CLICKED) {
			if (LOWORD(wParam) >= IDC_CHECK1 && LOWORD(wParam) <= IDC_CHECK29) {
				UpdateOptions(hDlg, true, false);
			} else {
				switch (LOWORD(wParam)) {
					case IDC_GOOD_PARTS: {
						UpdateOptions(hDlg, true, false);
						jsLintOptions.SetGoodParts();
						UpdateOptions(hDlg, false, false);
						break;
					}
					case IDC_CLEAR_ALL: {
						UpdateOptions(hDlg, true, false);
						jsLintOptions.ClearAllOptions();
						UpdateOptions(hDlg, false, false);
						break;
					}
					case IDOK: {
						if (UpdateOptions(hDlg, true, true)) {
							g_jsLintOptions = jsLintOptions;
							EndDialog(hDlg, 1);
						}
						return 1;
					}
					case IDCANCEL: {
						EndDialog(hDlg, 0);
						return 1;
					}
					default:
						break;
				}
			}
		} else if (HIWORD(wParam) == EN_KILLFOCUS) {
			UpdateOptions(hDlg, true, false);
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

////////////////////////////////////////////////////////////////////////////////

JSLintOptions::JSLintOptions()
{
	m_options[IDC_CHECK1] = Option(TEXT("passfail"));
	m_options[IDC_CHECK2] = Option(TEXT("white"));
	m_options[IDC_CHECK3] = Option(TEXT("browser"));
	m_options[IDC_CHECK4] = Option(TEXT("devel"));
	m_options[IDC_CHECK5] = Option(TEXT("widget"));
	m_options[IDC_CHECK6] = Option(TEXT("windows"));
	m_options[IDC_CHECK7] = Option(TEXT("rhino"));
	m_options[IDC_CHECK8] = Option(TEXT("safe"));
	m_options[IDC_CHECK9] = Option(TEXT("adsafe"));

	m_options[IDC_CHECK10] = Option(TEXT("debug"));
	m_options[IDC_CHECK11] = Option(TEXT("evil"));
	m_options[IDC_CHECK12] = Option(TEXT("laxbreak"));
	m_options[IDC_CHECK13] = Option(TEXT("forin"));
	m_options[IDC_CHECK14] = Option(TEXT("sub"));
	m_options[IDC_CHECK15] = Option(TEXT("css"));
	m_options[IDC_CHECK16] = Option(TEXT("cap"));
	m_options[IDC_CHECK17] = Option(TEXT("on"));
	m_options[IDC_CHECK18] = Option(TEXT("fragment"));
	m_options[IDC_CHECK19] = Option(TEXT("es5"));

	m_options[IDC_CHECK20] = Option(TEXT("onevar"));
	m_options[IDC_CHECK21] = Option(TEXT("undef"));
	m_options[IDC_CHECK22] = Option(TEXT("nomen"));
	m_options[IDC_CHECK23] = Option(TEXT("eqeqeq"));
	m_options[IDC_CHECK24] = Option(TEXT("plusplus"));
	m_options[IDC_CHECK25] = Option(TEXT("bitwise"));
	m_options[IDC_CHECK26] = Option(TEXT("regexp"));
	m_options[IDC_CHECK27] = Option(TEXT("newcap"));
	m_options[IDC_CHECK28] = Option(TEXT("immed"));
	m_options[IDC_CHECK29] = Option(TEXT("strict"));

	m_options[IDC_IDENT] = Option(TEXT("indent"), TEXT("4"));
	m_options[IDC_MAXLEN] = Option(TEXT("maxlen"), TEXT(""));
	m_options[IDC_MAXERR] = Option(TEXT("maxerr"), TEXT("50"));

	SetGoodParts();
}

void JSLintOptions::ReadOptions()
{
	tstring strConfigFileName = GetConfigFileName();
	if (Path::IsFileExists(strConfigFileName)) {
		std::map<UINT, Option>::iterator it;
		for (it = m_options.begin(); it != m_options.end(); ++it) {
			TCHAR szValue[256];
			GetPrivateProfileString(_T("JSLint Options"), it->second.name.c_str(), NULL, szValue, _countof(szValue), strConfigFileName.c_str());
			if (_tcscmp(szValue, TEXT("")) != 0) {
				tstring strValue = TrimSpaces(szValue);
				if (it->second.type == OPTION_TYPE_BOOL) {
					if (strValue == TEXT("true") || strValue == TEXT("false")) {
						it->second.value = strValue;
					}
				} else if (it->second.type == OPTION_TYPE_INT) {
					int value;
					if(_stscanf(strValue.c_str(), TEXT("%d"), &value) != EOF && value > 0) {
						it->second.value = strValue;
					}
				}
			}
		}
	}
}

void JSLintOptions::SaveOptions()
{
	tstring strConfigFileName = GetConfigFileName();
	if (Path::IsFileExists(Path::GetDirectoryName(strConfigFileName))) {
		std::map<UINT, Option>::iterator it;
		for (it = m_options.begin(); it != m_options.end(); ++it) {
			WritePrivateProfileString(_T("JSLint Options"), it->second.name.c_str(), it->second.value.c_str(), strConfigFileName.c_str());
		}
	}
}

UINT JSLintOptions::GetOptionID(const tstring& optionName) const
{
	map<UINT, Option>::const_iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
		if (it->second.name == optionName) {
			break;
		}
	}
	return it->first;
}

tstring JSLintOptions::GetOptionsString() const
{
	tstring strOptions;

	std::map<UINT, Option>::const_iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
		if (it->second.value != it->second.defaultValue) {
			if (!strOptions.empty())
				strOptions += TEXT(", ");
			strOptions += it->second.name + TEXT(": ") + it->second.value;
		}
	}

	return TEXT("/*jslint ") + strOptions + TEXT(" */");
}

tstring JSLintOptions::GetOptionsJSONString() const
{
	tstring strOptions;

	std::map<UINT, Option>::const_iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
		if (it->second.value != it->second.defaultValue) {
			if (!strOptions.empty())
				strOptions += TEXT(", ");
			strOptions += it->second.name + TEXT(": ") + it->second.value;
		}
	}

	return TEXT("{ ") + strOptions + TEXT(" }");
}

void JSLintOptions::CheckOption(UINT id)
{
	m_options[id].value = TEXT("true");
}

void JSLintOptions::UncheckOption(UINT id)
{
	m_options[id].value = TEXT("false");
}

bool JSLintOptions::IsOptionChecked(const tstring& name)
{
	return m_options[GetOptionID(name)].value == _T("true");
}

void JSLintOptions::SetOption(UINT id, const tstring& value)
{
	m_options[id].value = value;
}

void JSLintOptions::ResetOption(UINT id)
{
	m_options[id].value = m_options[id].defaultValue;
}

void JSLintOptions::ClearAllOptions()
{
	std::map<UINT, Option>::iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
		if (it->second.type == OPTION_TYPE_BOOL)
			it->second.value = TEXT("false");
	}
}

void JSLintOptions::SetGoodParts()
{
	m_options[GetOptionID(TEXT("white"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("onevar"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("undef"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("nomen"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("eqeqeq"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("plusplus"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("bitwise"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("regexp"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("newcap"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("immed"))].value = TEXT("true");
}

void JSLintOptions::UpdateDialog(HWND hDlg)
{
	std::map<UINT, Option>::iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
		if (it->second.type == OPTION_TYPE_BOOL) {
			Button_SetCheck(GetDlgItem(hDlg, it->first), it->second.value == TEXT("true"));
		} else if (it->second.type == OPTION_TYPE_INT) {
			SetWindowText(GetDlgItem(hDlg, it->first), it->second.value.c_str());
		}
	}
}

int JSLintOptions::GetTabWidth()
{
	int indent;
	if(_stscanf(m_options[IDC_IDENT].value.c_str(), TEXT("%d"), &indent) == EOF || indent < 1)
		return 4;
	return indent;
}

tstring JSLintOptions::GetConfigFileName()
{
	return Path::GetFullPath(TEXT("Config\\JSLint.ini"),
		Path::GetDirectoryName(
			Path::GetModuleFileName((HMODULE)g_hDllModule)));
}
