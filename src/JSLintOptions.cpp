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
#include "JSLintOptions.h"
#include "Settings.h"
#include "PluginDefinition.h"
#include "resource.h"
#include "Util.h"
#include "Version.h"

////////////////////////////////////////////////////////////////////////////////

#define PROFILE_SELECTED_LINTER_KEY_NAME TEXT("selected_linter")
#define PROFILE_JSLINT_OPTIONS_GROUP_NAME TEXT("JSLint Options")
#define PROFILE_JSHINT_OPTIONS_GROUP_NAME TEXT("JSHint Options")
#define PROFILE_ADDITIONAL_OPTIONS_KEY_NAME TEXT("jslintnpp_additional_options")

#define MIN_VERSION_BUILD 110

////////////////////////////////////////////////////////////////////////////////

LinterOptions::LinterOptions(LPCTSTR optionsGroupName)
    : m_optionsGroupName(optionsGroupName)
{
	m_options[IDC_PREDEFINED] = Option(OPTION_TYPE_ARR_STRING, TEXT("predef"), TEXT(""));
}

void LinterOptions::ReadOptions()
{
    TCHAR szValue[65536]; // memory is cheap

    tstring strConfigFileName = GetConfigFileName();
    if (Path::IsFileExists(strConfigFileName)) {
	    GetPrivateProfileString(PROFILE_JSLINT_GROUP_NAME, PROFILE_BUILD_KEY_NAME,
            NULL, szValue, _countof(szValue), strConfigFileName.c_str());
        if (_ttoi(szValue) >= MIN_VERSION_BUILD) {
	        std::map<UINT, Option>::iterator it;
	        for (it = m_options.begin(); it != m_options.end(); ++it) {
		        GetPrivateProfileString(m_optionsGroupName, it->second.name.c_str(),
                    NULL, szValue, _countof(szValue), strConfigFileName.c_str());
		        if (_tcscmp(szValue, TEXT("")) != 0) {
			        tstring strValue = TrimSpaces(szValue);
			        if (it->second.type == OPTION_TYPE_BOOL) {
                        if (strValue == TEXT("true") || strValue == TEXT("false") || strValue.empty()) {
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

	        GetPrivateProfileString(m_optionsGroupName, PROFILE_ADDITIONAL_OPTIONS_KEY_NAME,
                NULL, szValue, _countof(szValue), strConfigFileName.c_str());
            m_additionalOptions = szValue;
        }
    }
}

void LinterOptions::SaveOptions()
{
	tstring strConfigFileName = GetConfigFileName();

    std::map<UINT, Option>::iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
		WritePrivateProfileString(m_optionsGroupName,
            it->second.name.c_str(), it->second.value.c_str(),
            strConfigFileName.c_str());
	}

    WritePrivateProfileString(m_optionsGroupName, PROFILE_ADDITIONAL_OPTIONS_KEY_NAME,
        m_additionalOptions.c_str(), strConfigFileName.c_str());
}

UINT LinterOptions::GetOptionID(const tstring& optionName) const
{
	map<UINT, Option>::const_iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
		if (it->second.name == optionName) {
			break;
		}
	}
	return it->first;
}

bool LinterOptions::IsOptionIncluded(const Option& option) const
{
    return !option.value.empty();
}

tstring LinterOptions::GetOptionsCommentString() const
{
	tstring strOptions;

	std::map<UINT, Option>::const_iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
        if (IsOptionIncluded(it->second)) {
            if (it->first != IDC_PREDEFINED) {
			    if (!strOptions.empty())
				    strOptions += TEXT(", ");
			    strOptions += it->second.name + TEXT(": ") + it->second.value;
		    }
        }
	}

	return strOptions;
}

tstring LinterOptions::GetOptionsJSONString() const
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

    if (!m_additionalOptions.empty()) {
        if (!strOptions.empty())
            strOptions += TEXT(", ");
        strOptions += m_additionalOptions;
    }

	return TEXT("{ ") + strOptions + TEXT(" }");
}

void LinterOptions::CheckOption(UINT id)
{
	m_options[id].value = TEXT("true");
}

void LinterOptions::UncheckOption(UINT id)
{
	m_options[id].value = TEXT("false");
}

void LinterOptions::ClearOption(UINT id)
{
	m_options[id].value = TEXT("");
}

void LinterOptions::SetOption(UINT id, const tstring& value)
{
	m_options[id].value = value;
}

void LinterOptions::AppendOption(UINT id, const tstring& value)
{
	Option& option = m_options[id];
	if (option.value.empty())
		option.value = value;
	else
		option.value += TEXT(", ") + value;
}

void LinterOptions::ResetOption(UINT id)
{
	m_options[id].value = m_options[id].defaultValue;
}

void LinterOptions::SetAdditionalOptions(const tstring& additionalOptions)
{
    m_additionalOptions = additionalOptions;
}

void LinterOptions::ClearAllOptions()
{
	std::map<UINT, Option>::iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
        if (it->first != IDC_PREDEFINED) {
            it->second.value = it->second.defaultValue;
        }
	}
}

BOOL LinterOptions::UpdateOptions(HWND hDlg, HWND hSubDlg, bool bSaveOrValidate, bool bShowErrorMessage)
{
	if (bSaveOrValidate) {
    	std::map<UINT, Option>::iterator it;
    	for (it = m_options.begin(); it != m_options.end(); ++it) {
            if (it->second.type == OPTION_TYPE_BOOL) {
                int checkState = Button_GetCheck(GetDlgItem(hSubDlg, it->first));
			    if (checkState == BST_UNCHECKED) {
                    UncheckOption(it->first);
			    } else if (checkState == BST_CHECKED) {
				    CheckOption(it->first);
                } else {
				    ClearOption(it->first);
                }
            }
		}

		// predefined
		tstring strPredefined = TrimSpaces(GetWindowText(GetDlgItem(hDlg, IDC_PREDEFINED)));
		if (!strPredefined.empty()) {
			SetOption(IDC_PREDEFINED, strPredefined);
		} else {
			ResetOption(IDC_PREDEFINED);
		}

		// additional options
		tstring strAdditionalOptions = TrimSpaces(GetWindowText(GetDlgItem(hDlg, IDC_ADDITIONAL_OPTIONS)));
        SetAdditionalOptions(strAdditionalOptions);
    } else {
	    std::map<UINT, Option>::iterator it;
	    for (it = m_options.begin(); it != m_options.end(); ++it) {
		    if (it->second.type == OPTION_TYPE_BOOL) {
                int checkState;
                if (it->second.value == TEXT("false")) {
                    checkState = BST_UNCHECKED;
                } else if (it->second.value == TEXT("true")) {
                    checkState = BST_CHECKED;
                } else {
                    checkState = BST_INDETERMINATE;
                }
			    Button_SetCheck(GetDlgItem(hSubDlg, it->first), checkState);
		    } else if (it->second.type == OPTION_TYPE_INT || it->second.type == OPTION_TYPE_ARR_STRING) {
                if (GetDlgItem(hDlg, it->first)) {
			        SetWindowText(GetDlgItem(hDlg, it->first), it->second.value.c_str());
                } else {
			        SetWindowText(GetDlgItem(hSubDlg, it->first), it->second.value.c_str());
                }
		    }
	    }

        SetWindowText(GetDlgItem(hDlg, IDC_ADDITIONAL_OPTIONS), m_additionalOptions.c_str());
	}

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

JSLintLinterOptions::JSLintLinterOptions()
    : LinterOptions(PROFILE_JSLINT_OPTIONS_GROUP_NAME)
{
	m_options[IDC_CHECK_PASSFAIL]  = Option(TEXT("passfail"));
	m_options[IDC_CHECK_WHITE]     = Option(TEXT("white"));
	m_options[IDC_CHECK_BROWSER]   = Option(TEXT("browser"));
	m_options[IDC_CHECK_DEVEL]     = Option(TEXT("devel"));
	m_options[IDC_CHECK_WIDGET]    = Option(TEXT("widget"));
	m_options[IDC_CHECK_WINDOWS]   = Option(TEXT("windows"));
	m_options[IDC_CHECK_RHINO]     = Option(TEXT("rhino"));
	m_options[IDC_CHECK_SAFE]      = Option(TEXT("safe"));
	m_options[IDC_CHECK_ADSAFE]    = Option(TEXT("adsafe"));
	m_options[IDC_CHECK_DEBUG]     = Option(TEXT("debug"));
	m_options[IDC_CHECK_EVIL]      = Option(TEXT("evil"));
    m_options[IDC_CHECK_CONTINUE]  = Option(TEXT("continue"));
	m_options[IDC_CHECK_FORIN]     = Option(TEXT("forin"));
	m_options[IDC_CHECK_SUB]       = Option(TEXT("sub"));
	m_options[IDC_CHECK_CSS]       = Option(TEXT("css"));
	m_options[IDC_CHECK_CAP]       = Option(TEXT("cap"));
	m_options[IDC_CHECK_ON]        = Option(TEXT("on"));
	m_options[IDC_CHECK_FRAGMENT]  = Option(TEXT("fragment"));
	m_options[IDC_CHECK_ES5]       = Option(TEXT("es5"));
	m_options[IDC_CHECK_VARS]      = Option(TEXT("vars"));
	m_options[IDC_CHECK_UNDEF]     = Option(TEXT("undef"));
	m_options[IDC_CHECK_NOMEN]     = Option(TEXT("nomen"));
    m_options[IDC_CHECK_NODE]      = Option(TEXT("node"));
	m_options[IDC_CHECK_PLUSPLUS]  = Option(TEXT("plusplus"));
	m_options[IDC_CHECK_BITWISE]   = Option(TEXT("bitwise"));
	m_options[IDC_CHECK_REGEXP]    = Option(TEXT("regexp"));
	m_options[IDC_CHECK_NEWCAP]    = Option(TEXT("newcap"));
    m_options[IDC_CHECK_UNPARAM]   = Option(TEXT("unparam"));
	m_options[IDC_CHECK_SLOPPY]    = Option(TEXT("sloppy"));
    m_options[IDC_CHECK_EQEQ]      = Option(TEXT("eqeq"));
	m_options[IDC_CHECK_STUPID]    = Option(TEXT("stupid"));

	m_options[IDC_IDENT] = Option(OPTION_TYPE_INT, TEXT("indent"), TEXT("4"));
	m_options[IDC_MAXLEN] = Option(OPTION_TYPE_INT, TEXT("maxlen"), TEXT(""));
	m_options[IDC_MAXERR] = Option(OPTION_TYPE_INT, TEXT("maxerr"), TEXT("50"));
}

int JSLintLinterOptions::GetTabWidth()
{
	int indent;
	if(_stscanf(m_options[IDC_IDENT].value.c_str(), TEXT("%d"), &indent) == EOF || indent < 1)
		return 4;
	return indent;
}

BOOL JSLintLinterOptions::UpdateOptions(HWND hDlg, HWND hSubDlg, bool bSaveOrValidate, bool bShowErrorMessage)
{
    if (!LinterOptions::UpdateOptions(hDlg, hSubDlg, bSaveOrValidate, bShowErrorMessage)) {
        return FALSE;
    }

    if (bSaveOrValidate) {
		// indent
		tstring strIndent = TrimSpaces(GetWindowText(GetDlgItem(hSubDlg, IDC_IDENT)));
		if (!strIndent.empty()) {
			int indent;
			if(_stscanf(strIndent.c_str(), TEXT("%d"), &indent) == EOF || indent < 0) {
				if (bShowErrorMessage) {
					MessageBox(hSubDlg,
                        TEXT("Indentation must be an integer greater than or equal to zero"),
                        TEXT("JSLint"), MB_OK | MB_ICONERROR);
					SetFocus(GetDlgItem(hDlg, IDC_IDENT));
				}
				return FALSE;
			}
		}
		SetOption(IDC_IDENT, strIndent);

		// maxlen
		tstring strMaxlen = TrimSpaces(GetWindowText(GetDlgItem(hSubDlg, IDC_MAXLEN)));
		if (!strMaxlen.empty()) {
			int maxlen;
			if(_stscanf(strMaxlen.c_str(), TEXT("%d"), &maxlen) == EOF || maxlen < 1) {
				if (bShowErrorMessage) {
					MessageBox(hSubDlg,
                        TEXT("Maximum line length must be an integer greater than zero"),
                        TEXT("JSLint"), MB_OK | MB_ICONERROR);
					SetFocus(GetDlgItem(hDlg, IDC_MAXLEN));
				}
				return FALSE;
			}
		}
		SetOption(IDC_MAXLEN, strMaxlen);

		// maxerr
		tstring strMaxerr = TrimSpaces(GetWindowText(GetDlgItem(hSubDlg, IDC_MAXERR)));
		if (!strMaxerr.empty()) {
			int maxerr;
			if(_stscanf(strMaxerr.c_str(), TEXT("%d"), &maxerr) == EOF || maxerr < 1) {
				if (bShowErrorMessage) {
					MessageBox(hSubDlg,
                        TEXT("Maximum numer of errors must be an integer greater than zero"),
                        TEXT("JSLint"), MB_OK | MB_ICONERROR);
					SetFocus(GetDlgItem(hDlg, IDC_MAXERR));
				}
				return FALSE;
			}
		}
        SetOption(IDC_MAXERR, strMaxerr);
    }

    return TRUE;
}

tstring JSLintLinterOptions::GetOptionsCommentString() const
{
    return TEXT("/*jslint ") + LinterOptions::GetOptionsCommentString() + TEXT(" */");
}

////////////////////////////////////////////////////////////////////////////////

JSHintLinterOptions::JSHintLinterOptions()
    : LinterOptions(PROFILE_JSHINT_OPTIONS_GROUP_NAME)
{
	m_options[IDC_CHECK_DEBUG]    = Option(TEXT("debug"));
	m_options[IDC_CHECK_FORIN]    = Option(TEXT("forin"));
	m_options[IDC_CHECK_EQNULL]   = Option(TEXT("eqnull"));
	m_options[IDC_CHECK_NOARG]    = Option(TEXT("noarg"));
	m_options[IDC_CHECK_NOEMPTY]  = Option(TEXT("noempty"));
	m_options[IDC_CHECK_EQEQEQ]   = Option(TEXT("eqeqeq"));
	m_options[IDC_CHECK_BOSS]     = Option(TEXT("boss"));
	m_options[IDC_CHECK_LOOPFUNC] = Option(TEXT("loopfunc"));
	m_options[IDC_CHECK_EVIL]     = Option(TEXT("evil"));
    m_options[IDC_CHECK_LAXBREAK] = Option(TEXT("laxbreak"));
	m_options[IDC_CHECK_BITWISE]  = Option(TEXT("bitwise"));
	m_options[IDC_CHECK_STRICT]   = Option(TEXT("strict"));
	m_options[IDC_CHECK_UNDEF]    = Option(TEXT("undef"));
	m_options[IDC_CHECK_CURLY]    = Option(TEXT("curly"));
	m_options[IDC_CHECK_NONEW]    = Option(TEXT("nonew"));
	m_options[IDC_CHECK_BROWSER]  = Option(TEXT("browser"));
	m_options[IDC_CHECK_DEVEL]    = Option(TEXT("devel"));
	m_options[IDC_CHECK_JQUERY]   = Option(TEXT("jquery"));
	m_options[IDC_CHECK_ES5]      = Option(TEXT("es5"));
	m_options[IDC_CHECK_NODE]     = Option(TEXT("node"));
}

int JSHintLinterOptions::GetTabWidth()
{
	return 4;
}

tstring JSHintLinterOptions::GetOptionsCommentString() const
{
    return TEXT("/*jshint ") + LinterOptions::GetOptionsCommentString() + TEXT(" */");
}

////////////////////////////////////////////////////////////////////////////////

JSLintOptions JSLintOptions::m_options;
HWND JSLintOptions::m_hDlg;
HWND JSLintOptions::m_hWndJSLintOptionsSubdlg;
HWND JSLintOptions::m_hWndJSHintOptionsSubdlg;
HWND JSLintOptions::m_hSubDlg;

JSLintOptions::JSLintOptions()
{
}

JSLintOptions& JSLintOptions::GetInstance()
{
    static JSLintOptions singleton;
    return singleton;
}

void JSLintOptions::ReadOptions()
{
    TCHAR szValue[65536]; // memory is cheap

    m_selectedLinter = LINTER_JSLINT;

    tstring strConfigFileName = GetConfigFileName();
    if (Path::IsFileExists(strConfigFileName)) {
	    GetPrivateProfileString(PROFILE_JSLINT_GROUP_NAME, PROFILE_SELECTED_LINTER_KEY_NAME,
            NULL, szValue, _countof(szValue), strConfigFileName.c_str());
        if (_tcscmp(szValue, TEXT("JSHint")) == 0) {
            m_selectedLinter = LINTER_JSHINT;
        }
    }

    m_jsLintOptions.ReadOptions();
    m_jsHintOptions.ReadOptions();
}

void JSLintOptions::SaveOptions()
{
	tstring strConfigFileName = GetConfigFileName();

    WritePrivateProfileString(PROFILE_JSLINT_GROUP_NAME, PROFILE_SELECTED_LINTER_KEY_NAME,
        m_selectedLinter == LINTER_JSLINT ? TEXT("JSLint") : TEXT("JSHint"), strConfigFileName.c_str());

    m_jsLintOptions.SaveOptions();
    m_jsHintOptions.SaveOptions();
}

Linter JSLintOptions::GetSelectedLinter() const
{
    return m_selectedLinter;
}

void JSLintOptions::SetSelectedLinter(Linter selectedLinter)
{
    m_selectedLinter = selectedLinter;
}

LinterOptions* JSLintOptions::GetSelectedLinterOptions()
{
    if (m_selectedLinter == LINTER_JSLINT) {
        return &m_jsLintOptions;
    }
    return &m_jsHintOptions;
}

const LinterOptions* JSLintOptions::GetSelectedLinterOptions() const
{
    if (m_selectedLinter == LINTER_JSLINT) {
        return &m_jsLintOptions;
    }
    return &m_jsHintOptions;
}

tstring JSLintOptions::GetOptionsJSONString() const
{
    return GetSelectedLinterOptions()->GetOptionsJSONString();
}

int JSLintOptions::GetTabWidth()
{
    return GetSelectedLinterOptions()->GetTabWidth();
}

BOOL JSLintOptions::UpdateOptions(HWND hDlg, HWND hSubDlg, bool bSaveOrValidate, bool bShowErrorMessage)
{
    if (!GetSelectedLinterOptions()->UpdateOptions(hDlg, hSubDlg, bSaveOrValidate, bShowErrorMessage)) {
        return FALSE;
    }

    SetWindowText(GetDlgItem(hDlg, IDC_PREVIEW), GetSelectedLinterOptions()->GetOptionsCommentString().c_str());

    return TRUE;
}

void JSLintOptions::AppendOption(UINT id, const tstring& value)
{
    GetSelectedLinterOptions()->AppendOption(id, value);
}

void JSLintOptions::ClearAllOptions()
{
    GetSelectedLinterOptions()->ClearAllOptions();
}

INT_PTR CALLBACK JSLintOptions::PredefinedControlWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
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
						StringSplit(str, TEXT(" \t\r\n"), results);
						str = StringJoin(results, TEXT(", "));
						
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

INT_PTR CALLBACK JSLintOptions::SubDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    if (uMessage == WM_INITDIALOG) {
        return TRUE;
	} else if (uMessage == WM_COMMAND) {
		if (HIWORD(wParam) == BN_CLICKED) {
            UINT id = LOWORD(wParam);
			if (LOWORD(wParam) >= IDC_CHECK_FIRST_OPTION && LOWORD(wParam) <= IDC_CHECK_LAST_OPTION) {
                int checkState = Button_GetCheck(GetDlgItem(hDlg, id));
                if (checkState == BST_UNCHECKED) {
                    Button_SetCheck(GetDlgItem(hDlg, id), BST_INDETERMINATE);
                } else if (checkState == BST_CHECKED) {
                    Button_SetCheck(GetDlgItem(hDlg, id), BST_UNCHECKED);
                } else {
                    Button_SetCheck(GetDlgItem(hDlg, id), BST_CHECKED);
                }
				m_options.UpdateOptions(m_hDlg, m_hSubDlg, true, false);
			}
        }
    }

    return 0;
}

INT_PTR CALLBACK JSLintOptions::DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    m_hDlg = hDlg;

	if (uMessage == WM_INITDIALOG) {
        CenterWindow(hDlg, g_nppData._nppHandle);
        m_options = GetInstance();

        // initialize selected linter combo box
        HWND hWndSelectedLinter = GetDlgItem(hDlg, IDC_SELECTED_LINTER);
        ComboBox_AddString(hWndSelectedLinter, TEXT("JSLint"));
        ComboBox_AddString(hWndSelectedLinter, TEXT("JSHint"));
        
        // create JSLint and JSHint options subdialog
        HWND hWndOptionsPlaceholder = GetDlgItem(hDlg, IDC_OPTIONS_PLACEHOLDER);
        RECT rect;
        GetWindowRect(hWndOptionsPlaceholder, &rect);
        POINT ptTopLeft;
        ptTopLeft.x = rect.left;
        ptTopLeft.y = rect.top;
        ScreenToClient(hDlg, &ptTopLeft);

	    m_hWndJSLintOptionsSubdlg = CreateDialog((HINSTANCE)g_hDllModule, MAKEINTRESOURCE(IDD_JSLINT_OPTIONS), hDlg, SubDlgProc);
        SetWindowPos(m_hWndJSLintOptionsSubdlg, hWndOptionsPlaceholder, ptTopLeft.x, ptTopLeft.y, rect.right - rect.left, rect.bottom - rect.top, 0);

	    m_hWndJSHintOptionsSubdlg = CreateDialog((HINSTANCE)g_hDllModule, MAKEINTRESOURCE(IDD_JSHINT_OPTIONS), hDlg, SubDlgProc);
        SetWindowPos(m_hWndJSHintOptionsSubdlg, hWndOptionsPlaceholder, ptTopLeft.x, ptTopLeft.y, rect.right - rect.left, rect.bottom - rect.top, 0);

        if (m_options.GetSelectedLinter() == LINTER_JSLINT) {
            m_hSubDlg = m_hWndJSLintOptionsSubdlg;
            ComboBox_SelectString(hWndSelectedLinter, 0, TEXT("JSLint"));
        } else {
            m_hSubDlg = m_hWndJSHintOptionsSubdlg;
            ComboBox_SelectString(hWndSelectedLinter, 0, TEXT("JSHint"));
        }
		m_options.UpdateOptions(m_hDlg, m_hSubDlg, false, false);
        ShowWindow(m_hSubDlg, SW_SHOW);

        // subclass IDC_PREDEFINED
		HWND hWndPredefined = GetDlgItem(hDlg, IDC_PREDEFINED);
		WNDPROC oldWndProc = (WNDPROC)SetWindowLongPtr(hWndPredefined, GWLP_WNDPROC,
            (LONG_PTR)PredefinedControlWndProc);
		SetProp(hWndPredefined, TEXT("OldWndProc"), (HANDLE)oldWndProc);
    } else if (uMessage == WM_COMMAND) {
		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
				case IDC_CLEAR_ALL:
					m_options.UpdateOptions(m_hDlg, m_hSubDlg, true, false);
					m_options.ClearAllOptions();
					m_options.UpdateOptions(m_hDlg, m_hSubDlg, false, false);
					break;
				case IDOK:
					if (m_options.UpdateOptions(m_hDlg, m_hSubDlg, true, true)) {
                        GetInstance() = m_options;
						EndDialog(hDlg, 1);
					}
					return 1;
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return 1;
				default:
					break;
			}
        } else if (HIWORD(wParam) == CBN_SELCHANGE) {
            HWND hWndSelectedLinter = GetDlgItem(hDlg, IDC_SELECTED_LINTER);
            int curSel = ComboBox_GetCurSel(hWndSelectedLinter);
            if (curSel != CB_ERR) {
				if (m_options.UpdateOptions(m_hDlg, m_hSubDlg, true, true)) {
                    TCHAR buffer[32];
                    ComboBox_GetLBText(hWndSelectedLinter, curSel, buffer);

                    ShowWindow(m_hSubDlg, SW_HIDE);

                    if (_tcsicmp(buffer, TEXT("JSLint")) == 0) {
                        m_options.SetSelectedLinter(LINTER_JSLINT);
                        m_hSubDlg = m_hWndJSLintOptionsSubdlg;
                    } else {
                        m_options.SetSelectedLinter(LINTER_JSHINT);
                        m_hSubDlg = m_hWndJSHintOptionsSubdlg;
                    }

                    m_options.UpdateOptions(m_hDlg, m_hSubDlg, false, false);
                    ShowWindow(m_hSubDlg, SW_SHOW);
                } else {
                    if (m_options.GetSelectedLinter() == LINTER_JSLINT) {
                        ComboBox_SelectString(hWndSelectedLinter, 0, TEXT("JSLint"));
                    } else {
                        ComboBox_SelectString(hWndSelectedLinter, 0, TEXT("JSHint"));
                    }
                }
            }
		} else if (HIWORD(wParam) == EN_KILLFOCUS) {
            m_options.UpdateOptions(m_hDlg, m_hSubDlg, true, false);
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

void JSLintOptions::ShowDialog()
{
	pluginDialogBox(IDD_OPTIONS, DlgProc);
}
