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

PuppetLintOptions g_puppetLintOptions;

////////////////////////////////////////////////////////////////////////////////

static PuppetLintOptions puppetLintOptions;

BOOL UpdateOptions(HWND hDlg, bool bSaveOrValidate, bool bShowErrorMessage)
{
	if (bSaveOrValidate) {
		for (UINT id = IDC_CHECK1; id <= IDC_CHECK29; ++id) {
			if (GetDlgItem(hDlg, id) == NULL)
				continue;
			if (Button_GetCheck(GetDlgItem(hDlg, id))) {
				puppetLintOptions.CheckOption(id);
			} else {
				puppetLintOptions.UncheckOption(id);
			}
		}

		// predefined
		int nPredefinedLength = GetWindowTextLength(GetDlgItem(hDlg, IDC_PREDEFINED));
		TCHAR *szPredefined = new TCHAR[nPredefinedLength+1];
		GetWindowText(GetDlgItem(hDlg, IDC_PREDEFINED), szPredefined, nPredefinedLength+1);
		tstring strPredefined = TrimSpaces(szPredefined);
		delete [] szPredefined;
		if (!strPredefined.empty()) {
			puppetLintOptions.SetOption(IDC_PREDEFINED, strPredefined);
		} else {
			puppetLintOptions.ResetOption(IDC_PREDEFINED);
		}

		// indent
		if (puppetLintOptions.IsOptionChecked(TEXT("white"))) {
			TCHAR szIdent[32];
			GetWindowText(GetDlgItem(hDlg, IDC_IDENT), szIdent, _countof(szIdent));
			tstring strIdent = TrimSpaces(szIdent);
			if (!strIdent.empty()) {
				int indent;
				if(_stscanf(strIdent.c_str(), TEXT("%d"), &indent) == EOF || indent < 1) {
					if (bShowErrorMessage) {
						MessageBox(hDlg, TEXT("Identation must be an integer greater than zero"), _T("PuppetLint"), MB_OK | MB_ICONERROR);
						SetFocus(GetDlgItem(hDlg, IDC_IDENT));
					}
					return FALSE;
				}
			}
			puppetLintOptions.SetOption(IDC_IDENT, strIdent);
		}

		// maxlen
		TCHAR szMaxlen[32];
		GetWindowText(GetDlgItem(hDlg, IDC_MAXLEN), szMaxlen, _countof(szMaxlen));
		tstring strMaxlen = TrimSpaces(szMaxlen);
		if (!strMaxlen.empty()) {
			int maxlen;
			if(_stscanf(strMaxlen.c_str(), TEXT("%d"), &maxlen) == EOF || maxlen < 1) {
				if (bShowErrorMessage) {
					MessageBox(hDlg, TEXT("Maximum line length must be an integer greater than zero"), _T("PuppetLint"), MB_OK | MB_ICONERROR);
					SetFocus(GetDlgItem(hDlg, IDC_MAXLEN));
				}
				return FALSE;
			}
		}
		puppetLintOptions.SetOption(IDC_MAXLEN, strMaxlen);

		// maxerr
		if (!puppetLintOptions.IsOptionChecked(TEXT("passfail"))) {
			TCHAR szMaxerr[32];
			GetWindowText(GetDlgItem(hDlg, IDC_MAXERR), szMaxerr, _countof(szMaxerr));
			tstring strMaxerr = TrimSpaces(szMaxerr);
			if (!strMaxerr.empty()) {
				int maxerr;
				if(_stscanf(strMaxerr.c_str(), TEXT("%d"), &maxerr) == EOF || maxerr < 1) {
					if (bShowErrorMessage) {
						MessageBox(hDlg, TEXT("Maximum numer of errors must be an integer greater than zero"), _T("PuppetLint"), MB_OK | MB_ICONERROR);
						SetFocus(GetDlgItem(hDlg, IDC_MAXERR));
					}
					return FALSE;
				}
			}
            puppetLintOptions.SetOption(IDC_MAXERR, strMaxerr);
        }
	} else {
		puppetLintOptions.UpdateDialog(hDlg);
	}

	SetWindowText(GetDlgItem(hDlg, IDC_PREVIEW), 
		puppetLintOptions.GetOptionsCommentString().c_str());

	EnableWindow(GetDlgItem(hDlg, IDC_IDENT), 
		puppetLintOptions.IsOptionChecked(TEXT("white")));

	EnableWindow(GetDlgItem(hDlg, IDC_MAXERR), 
		!puppetLintOptions.IsOptionChecked(TEXT("passfail")));

	return TRUE;
}

INT_PTR CALLBACK PredefinedControlWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	if (uMessage == WM_PASTE) {
		if (IsClipboardFormatAvailable(CF_TEXT)) {
			if (OpenClipboard(NULL)) {
				HGLOBAL hGlobal = GetClipboardData(CF_TEXT);
				if (hGlobal) {
					LPSTR lpData = (LPSTR)GlobalLock(hGlobal);
					if (lpData != NULL) {
						tstring str(TextConversion::A_To_T(lpData));
						
						vector<tstring> results;
						StringSplit(str, _T(" \t\r\n"), results);
						str = StringJoin(results, _T(", "));
						
						SendMessage(hWnd, EM_REPLACESEL, TRUE, (LPARAM)str.c_str());
					}
					GlobalUnlock(hGlobal);
				}
				CloseClipboard();
			}
		}
		return 0;
	}

	WNDPROC oldWndProc = (WNDPROC)GetProp(hWnd, TEXT("OldWndProc"));
	return (*oldWndProc)(hWnd, uMessage, wParam, lParam);
}

INT_PTR CALLBACK OptionsDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	if (uMessage == WM_INITDIALOG) {
		puppetLintOptions = g_puppetLintOptions;
		UpdateOptions(hDlg, false, false);
        CenterWindow(hDlg, g_nppData._nppHandle);

		// subclass IDC_PREDEFINED
		HWND hWndPredefined = GetDlgItem(hDlg, IDC_PREDEFINED);
		WNDPROC oldWndProc = (WNDPROC)SetWindowLongPtr(hWndPredefined, GWLP_WNDPROC, (LONG_PTR)PredefinedControlWndProc);
		SetProp(hWndPredefined, TEXT("OldWndProc"), (HANDLE)oldWndProc);
    } else if (uMessage == WM_COMMAND) {
		if (HIWORD(wParam) == BN_CLICKED) {
			if (LOWORD(wParam) >= IDC_CHECK1 && LOWORD(wParam) <= IDC_CHECK29) {
				UpdateOptions(hDlg, true, false);
			} else {
				switch (LOWORD(wParam)) {
					case IDC_GOOD_PARTS: {
						UpdateOptions(hDlg, true, false);
						puppetLintOptions.SetGoodParts();
						UpdateOptions(hDlg, false, false);
						break;
					}
					case IDC_CLEAR_ALL: {
						UpdateOptions(hDlg, true, false);
						puppetLintOptions.ClearAllOptions();
						UpdateOptions(hDlg, false, false);
						break;
					}
					case IDOK: {
						if (UpdateOptions(hDlg, true, true)) {
							g_puppetLintOptions = puppetLintOptions;
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

PuppetLintOptions::PuppetLintOptions()
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
    m_options[IDC_CHECK12] = Option(TEXT("continue"));
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
    m_options[IDC_CHECK23] = Option(TEXT("node"));
	m_options[IDC_CHECK24] = Option(TEXT("plusplus"));
	m_options[IDC_CHECK25] = Option(TEXT("bitwise"));
	m_options[IDC_CHECK26] = Option(TEXT("regexp"));
	m_options[IDC_CHECK27] = Option(TEXT("newcap"));
	m_options[IDC_CHECK29] = Option(TEXT("strict"));

	m_options[IDC_PREDEFINED] = Option(OPTION_TYPE_ARR_STRING, TEXT("predef"), TEXT(""));

	m_options[IDC_IDENT] = Option(OPTION_TYPE_INT, TEXT("indent"), TEXT("4"));
	m_options[IDC_MAXLEN] = Option(OPTION_TYPE_INT, TEXT("maxlen"), TEXT(""));
	m_options[IDC_MAXERR] = Option(OPTION_TYPE_INT, TEXT("maxerr"), TEXT("50"));

	SetGoodParts();
}

void PuppetLintOptions::ReadOptions()
{
	tstring strConfigFileName = GetConfigFileName();
	if (Path::IsFileExists(strConfigFileName)) {
		std::map<UINT, Option>::iterator it;
		for (it = m_options.begin(); it != m_options.end(); ++it) {
			TCHAR szValue[256];
			GetPrivateProfileString(TEXT("PuppetLint Options"), it->second.name.c_str(), NULL, szValue, _countof(szValue), strConfigFileName.c_str());
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
				} else if (it->second.type == OPTION_TYPE_ARR_STRING) {
					it->second.value = strValue;
				}
			}
		}
	}
}

void PuppetLintOptions::SaveOptions()
{
	tstring strConfigFileName = GetConfigFileName();
	if (Path::IsFileExists(Path::GetDirectoryName(strConfigFileName))) {
		std::map<UINT, Option>::iterator it;
		for (it = m_options.begin(); it != m_options.end(); ++it) {
			WritePrivateProfileString(TEXT("PuppetLint Options"), it->second.name.c_str(), it->second.value.c_str(), strConfigFileName.c_str());
		}
	}
}

UINT PuppetLintOptions::GetOptionID(const tstring& optionName) const
{
	map<UINT, Option>::const_iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
		if (it->second.name == optionName) {
			break;
		}
	}
	return it->first;
}

bool PuppetLintOptions::IsOptionIncluded(const Option& option) const
{
    if (option.name == TEXT("indent") && IsOptionChecked(TEXT("white")))
        return !option.value.empty();

    if (option.name == TEXT("maxerr") && !IsOptionChecked(TEXT("passfail")))
        return !option.value.empty();

    return option.value != option.defaultValue;
}

tstring PuppetLintOptions::GetOptionsCommentString() const
{
	tstring strOptions;

	std::map<UINT, Option>::const_iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
        if (IsOptionIncluded(it->second)) {
            if (it->second.type != OPTION_TYPE_ARR_STRING) {
			    if (!strOptions.empty())
				    strOptions += TEXT(", ");
			    strOptions += it->second.name + TEXT(": ") + it->second.value;
		    }
        }
	}

	return TEXT("/*jslint ") + strOptions + TEXT(" */");
}

tstring PuppetLintOptions::GetOptionsJSONString() const
{
	tstring strOptions;

	std::map<UINT, Option>::const_iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
		if (IsOptionIncluded(it->second)) {
			tstring value;

			if (it->second.type == OPTION_TYPE_ARR_STRING) {
				vector<tstring> arr;
				StringSplit(it->second.value, TEXT(","), arr);
				vector<tstring>::const_iterator itArr;
				for (itArr = arr.begin(); itArr != arr.end(); ++itArr) {
					if (value.empty())
						value += TEXT("[");
					else
						value += TEXT(", ");

					tstring element = TrimSpaces(*itArr);
					FindReplace(element, TEXT("\\"), TEXT("\\\\"));
					FindReplace(element, TEXT("\""), TEXT("\\\""));

					value += TEXT("\"") + element + TEXT("\"");
				}
				if (!value.empty())
					value += TEXT("]");
			} else {
				value = it->second.value;
			}

			if (!value.empty()) {
				if (!strOptions.empty())
					strOptions += TEXT(", ");
				strOptions += it->second.name + TEXT(": ") + value;
			}
		}
	}

	return TEXT("{ ") + strOptions + TEXT(" }");
}

void PuppetLintOptions::CheckOption(UINT id)
{
	m_options[id].value = TEXT("true");
}

void PuppetLintOptions::UncheckOption(UINT id)
{
	m_options[id].value = TEXT("false");
}

bool PuppetLintOptions::IsOptionChecked(const tstring& name) const
{
	return m_options.find(GetOptionID(name))->second.value == _T("true");
}

void PuppetLintOptions::SetOption(UINT id, const tstring& value)
{
	m_options[id].value = value;
}

void PuppetLintOptions::AppendOption(UINT id, const tstring& value)
{
	Option& option = m_options[id];
	if (option.value.empty())
		option.value = value;
	else
		option.value += _T(", ") + value;
}

void PuppetLintOptions::ResetOption(UINT id)
{
	m_options[id].value = m_options[id].defaultValue;
}

void PuppetLintOptions::ClearAllOptions()
{
	std::map<UINT, Option>::iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
		if (it->second.type == OPTION_TYPE_BOOL)
			it->second.value = TEXT("false");
	}
}

void PuppetLintOptions::SetGoodParts()
{
	m_options[GetOptionID(TEXT("white"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("onevar"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("undef"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("newcap"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("nomen"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("regexp"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("plusplus"))].value = TEXT("true");
	m_options[GetOptionID(TEXT("bitwise"))].value = TEXT("true");
}

void PuppetLintOptions::UpdateDialog(HWND hDlg)
{
	std::map<UINT, Option>::iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
		if (it->second.type == OPTION_TYPE_BOOL) {
			Button_SetCheck(GetDlgItem(hDlg, it->first), it->second.value == TEXT("true"));
		} else if (it->second.type == OPTION_TYPE_INT || it->second.type == OPTION_TYPE_ARR_STRING) {
			SetWindowText(GetDlgItem(hDlg, it->first), it->second.value.c_str());
		}
	}
}

int PuppetLintOptions::GetTabWidth()
{
	int indent;
	if(_stscanf(m_options[IDC_IDENT].value.c_str(), TEXT("%d"), &indent) == EOF || indent < 1)
		return 4;
	return indent;
}
