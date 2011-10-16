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

#define PROFILE_JSLINT_OPTIONS_GROUP_NAME TEXT("JSLint Options")
#define PROFILE_ADDITIONAL_OPTIONS_KEY_NAME TEXT("jslintnpp_additional_options")

#define MIN_VERSION_BUILD 110

////////////////////////////////////////////////////////////////////////////////

JSLintOptions::JSLintOptions()
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
    m_options[IDC_CHECK_CONFUSION] = Option(TEXT("confusion"));
    m_options[IDC_CHECK_EQEQ]      = Option(TEXT("eqeq"));

	m_options[IDC_IDENT] = Option(OPTION_TYPE_INT, TEXT("indent"), TEXT("4"));
	m_options[IDC_MAXLEN] = Option(OPTION_TYPE_INT, TEXT("maxlen"), TEXT(""));
	m_options[IDC_MAXERR] = Option(OPTION_TYPE_INT, TEXT("maxerr"), TEXT("50"));

	m_options[IDC_PREDEFINED] = Option(OPTION_TYPE_ARR_STRING, TEXT("predef"), TEXT(""));
}

JSLintOptions& JSLintOptions::GetInstance()
{
    static JSLintOptions singleton;
    return singleton;
}

void JSLintOptions::ReadOptions()
{
    TCHAR szValue[65536]; // memory is cheap

    tstring strConfigFileName = GetConfigFileName();
    if (Path::IsFileExists(strConfigFileName)) {
	    GetPrivateProfileString(PROFILE_JSLINT_GROUP_NAME, PROFILE_BUILD_KEY_NAME,
            NULL, szValue, _countof(szValue), strConfigFileName.c_str());
        if (_ttoi(szValue) >= MIN_VERSION_BUILD) {
	        std::map<UINT, Option>::iterator it;
	        for (it = m_options.begin(); it != m_options.end(); ++it) {
		        GetPrivateProfileString(PROFILE_JSLINT_OPTIONS_GROUP_NAME, it->second.name.c_str(),
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

	        GetPrivateProfileString(PROFILE_JSLINT_OPTIONS_GROUP_NAME, PROFILE_ADDITIONAL_OPTIONS_KEY_NAME,
                NULL, szValue, _countof(szValue), strConfigFileName.c_str());
            m_additionalOptions = szValue;
        }
    }
}

void JSLintOptions::SaveOptions()
{
	tstring strConfigFileName = GetConfigFileName();

    std::map<UINT, Option>::iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
		WritePrivateProfileString(PROFILE_JSLINT_OPTIONS_GROUP_NAME,
            it->second.name.c_str(), it->second.value.c_str(),
            strConfigFileName.c_str());
	}

    WritePrivateProfileString(PROFILE_JSLINT_OPTIONS_GROUP_NAME, PROFILE_ADDITIONAL_OPTIONS_KEY_NAME,
        m_additionalOptions.c_str(), strConfigFileName.c_str());
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

bool JSLintOptions::IsOptionIncluded(const Option& option) const
{
    return !option.value.empty();
}

tstring JSLintOptions::GetOptionsCommentString() const
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

	return TEXT("/*jslint ") + strOptions + TEXT(" */");
}

tstring JSLintOptions::GetOptionsJSONString() const
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

void JSLintOptions::CheckOption(UINT id)
{
	m_options[id].value = TEXT("true");
}

void JSLintOptions::UncheckOption(UINT id)
{
	m_options[id].value = TEXT("false");
}

void JSLintOptions::ClearOption(UINT id)
{
	m_options[id].value = TEXT("");
}

void JSLintOptions::SetOption(UINT id, const tstring& value)
{
	m_options[id].value = value;
}

void JSLintOptions::AppendOption(UINT id, const tstring& value)
{
	Option& option = m_options[id];
	if (option.value.empty())
		option.value = value;
	else
		option.value += TEXT(", ") + value;
}

void JSLintOptions::ResetOption(UINT id)
{
	m_options[id].value = m_options[id].defaultValue;
}

void JSLintOptions::SetAdditionalOptions(const tstring& additionalOptions)
{
    m_additionalOptions = additionalOptions;
}

void JSLintOptions::ClearAllOptions()
{
	std::map<UINT, Option>::iterator it;
	for (it = m_options.begin(); it != m_options.end(); ++it) {
        if (it->first != IDC_PREDEFINED) {
            it->second.value = it->second.defaultValue;
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

BOOL JSLintOptions::UpdateOptions(HWND hDlg, bool bSaveOrValidate, bool bShowErrorMessage)
{
	if (bSaveOrValidate) {
		for (UINT id = IDC_CHECK_FIRST_OPTION; id <= IDC_CHECK_LAST_OPTION; ++id) {
            int checkState = Button_GetCheck(GetDlgItem(hDlg, id));
			if (checkState == BST_UNCHECKED) {
                UncheckOption(id);
			} else if (checkState == BST_CHECKED) {
				CheckOption(id);
            } else {
				ClearOption(id);
            }
		}

		// indent
		tstring strIndent = TrimSpaces(GetWindowText(GetDlgItem(hDlg, IDC_IDENT)));
		if (!strIndent.empty()) {
			int indent;
			if(_stscanf(strIndent.c_str(), TEXT("%d"), &indent) == EOF || indent < 0) {
				if (bShowErrorMessage) {
					MessageBox(hDlg,
                        TEXT("Indentation must be an integer greater than or equal to zero"),
                        TEXT("JSLint"), MB_OK | MB_ICONERROR);
					SetFocus(GetDlgItem(hDlg, IDC_IDENT));
				}
				return FALSE;
			}
		}
		SetOption(IDC_IDENT, strIndent);

		// maxlen
		tstring strMaxlen = TrimSpaces(GetWindowText(GetDlgItem(hDlg, IDC_MAXLEN)));
		if (!strMaxlen.empty()) {
			int maxlen;
			if(_stscanf(strMaxlen.c_str(), TEXT("%d"), &maxlen) == EOF || maxlen < 1) {
				if (bShowErrorMessage) {
					MessageBox(hDlg,
                        TEXT("Maximum line length must be an integer greater than zero"),
                        TEXT("JSLint"), MB_OK | MB_ICONERROR);
					SetFocus(GetDlgItem(hDlg, IDC_MAXLEN));
				}
				return FALSE;
			}
		}
		SetOption(IDC_MAXLEN, strMaxlen);

		// maxerr
		tstring strMaxerr = TrimSpaces(GetWindowText(GetDlgItem(hDlg, IDC_MAXERR)));
		if (!strMaxerr.empty()) {
			int maxerr;
			if(_stscanf(strMaxerr.c_str(), TEXT("%d"), &maxerr) == EOF || maxerr < 1) {
				if (bShowErrorMessage) {
					MessageBox(hDlg,
                        TEXT("Maximum numer of errors must be an integer greater than zero"),
                        TEXT("JSLint"), MB_OK | MB_ICONERROR);
					SetFocus(GetDlgItem(hDlg, IDC_MAXERR));
				}
				return FALSE;
			}
		}
        SetOption(IDC_MAXERR, strMaxerr);

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
			    Button_SetCheck(GetDlgItem(hDlg, it->first), checkState);
		    } else if (it->second.type == OPTION_TYPE_INT || it->second.type == OPTION_TYPE_ARR_STRING) {
			    SetWindowText(GetDlgItem(hDlg, it->first), it->second.value.c_str());
		    }
	    }

        SetWindowText(GetDlgItem(hDlg, IDC_ADDITIONAL_OPTIONS), m_additionalOptions.c_str());
	}

	SetWindowText(GetDlgItem(hDlg, IDC_PREVIEW), GetOptionsCommentString().c_str());

	return TRUE;
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

INT_PTR CALLBACK JSLintOptions::DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    static JSLintOptions jsLintOptions;

	if (uMessage == WM_INITDIALOG) {
        jsLintOptions = GetInstance();
		jsLintOptions.UpdateOptions(hDlg, false, false);
        CenterWindow(hDlg, g_nppData._nppHandle);

		// subclass IDC_PREDEFINED
		HWND hWndPredefined = GetDlgItem(hDlg, IDC_PREDEFINED);
		WNDPROC oldWndProc = (WNDPROC)SetWindowLongPtr(hWndPredefined, GWLP_WNDPROC,
            (LONG_PTR)PredefinedControlWndProc);
		SetProp(hWndPredefined, TEXT("OldWndProc"), (HANDLE)oldWndProc);
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
				jsLintOptions.UpdateOptions(hDlg, true, false);
			} else {
				switch (LOWORD(wParam)) {
					case IDC_CLEAR_ALL:
						jsLintOptions.UpdateOptions(hDlg, true, false);
						jsLintOptions.ClearAllOptions();
						jsLintOptions.UpdateOptions(hDlg, false, false);
						break;
					case IDOK:
						if (jsLintOptions.UpdateOptions(hDlg, true, true)) {
                            GetInstance() = jsLintOptions;
							EndDialog(hDlg, 1);
						}
						return 1;
					case IDCANCEL:
						EndDialog(hDlg, 0);
						return 1;
					default:
						break;
				}
			}
		} else if (HIWORD(wParam) == EN_KILLFOCUS) {
			jsLintOptions.UpdateOptions(hDlg, true, false);
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
	pluginDialogBox(IDD_JSLINT_OPTIONS, DlgProc);
}