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
#include "Settings.h"
#include "PluginDefinition.h"
#include "JSLint.h"
#include "resource.h"
#include "Util.h"
#include "Version.h"

////////////////////////////////////////////////////////////////////////////////

#define PROFILE_SETTINGS_GROUP_NAME TEXT("Settings")

#define PROFILE_SCRIPT_SOURCE_KEY_NAME TEXT("_script_source")
#define PROFILE_SCRIPT_VERSION_KEY_NAME TEXT("_script_version")
#define PROFILE_SPEC_UNDEF_VAR_ERR_MSG_KEY_NAME TEXT("_spec_undef_var_err_msg")
#define PROFILE_UNDEF_VAR_ERR_MSG_KEY_NAME TEXT("_undef_var_err_msg")

#define PROFILE_SCRIPT_SOURCE_BUILTIN TEXT("builtin")
#define PROFILE_SCRIPT_SOURCE_DOWNLOADED TEXT("downloaded")

#define MIN_VERSION_BUILD 110

////////////////////////////////////////////////////////////////////////////////

ScriptSourceDef::ScriptSourceDef(Linter linter)
    : m_linter(linter)
    , m_scriptSource(SCRIPT_SOURCE_BUILTIN)
    , m_bSpecUndefVarErrMsg(false)
    , m_undefVarErrMsg(GetDefaultUndefVarErrMsg())
{
}

int ScriptSourceDef::GetScriptResourceID()
{
    return m_linter == LINTER_JSLINT ? IDR_JSLINT : IDR_JSHINT;
}

LPCTSTR ScriptSourceDef::GetDefaultUndefVarErrMsg()
{
    return m_linter == LINTER_JSLINT ? JSLINT_DEFAULT_UNDEF_VAR_ERR_MSG : JSHINT_DEFAULT_UNDEF_VAR_ERR_MSG;
}

LPCSTR ScriptSourceDef::GetNamespace()
{
    return m_linter == LINTER_JSLINT ? "JSLINT" : "JSHINT";
}

////////////////////////////////////////////////////////////////////////////////

Settings::Settings()
    : m_jsLintScript(LINTER_JSLINT)
    , m_jsHintScript(LINTER_JSHINT)
{
}

Settings& Settings::GetInstance()
{
    static Settings singleton;
    return singleton;
}

void Settings::ReadOptions()
{
    ReadOptions(TEXT("jslint"), m_jsLintScript);
    ReadOptions(TEXT("jshint"), m_jsHintScript);
}

void Settings::SaveOptions()
{
    SaveOptions(TEXT("jslint"), m_jsLintScript);
    SaveOptions(TEXT("jshint"), m_jsHintScript);
}

void Settings::ReadOptions(const tstring& prefix, ScriptSourceDef& scriptSourceDef)
{
    TCHAR szValue[65536]; // memory is cheap

    tstring strConfigFileName = GetConfigFileName();
    if (Path::IsFileExists(strConfigFileName)) {
	    GetPrivateProfileString(PROFILE_JSLINT_GROUP_NAME, PROFILE_BUILD_KEY_NAME,
            NULL, szValue, _countof(szValue), strConfigFileName.c_str());
        if (_ttoi(szValue) >= MIN_VERSION_BUILD) {
            if (_ttoi(szValue) >= VERSION_BUILD) {
	            GetPrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
                    (prefix + PROFILE_SCRIPT_SOURCE_KEY_NAME).c_str(),
                    NULL, szValue, _countof(szValue), strConfigFileName.c_str());
                if (_tcscmp(szValue, PROFILE_SCRIPT_SOURCE_DOWNLOADED) == 0) {
                    scriptSourceDef.m_scriptSource = SCRIPT_SOURCE_DOWNLOADED;
                } else {
                    scriptSourceDef.m_scriptSource = SCRIPT_SOURCE_BUILTIN;
                }
            } else {
                // if old config switch to builtin source
                scriptSourceDef.m_scriptSource = SCRIPT_SOURCE_BUILTIN;
            }

	        GetPrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
                (prefix + PROFILE_SCRIPT_VERSION_KEY_NAME).c_str(),
                NULL, szValue, _countof(szValue), strConfigFileName.c_str());
            scriptSourceDef.m_scriptVersion = szValue;

	        GetPrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
                (prefix + PROFILE_SPEC_UNDEF_VAR_ERR_MSG_KEY_NAME).c_str(),
                NULL, szValue, _countof(szValue), strConfigFileName.c_str());
            scriptSourceDef.m_bSpecUndefVarErrMsg = _tcscmp(szValue, TEXT("true")) == 0;

	        GetPrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
                (prefix + PROFILE_UNDEF_VAR_ERR_MSG_KEY_NAME).c_str(),
                NULL, szValue, _countof(szValue), strConfigFileName.c_str());
            if (_tcslen(szValue) > 0) {
                scriptSourceDef.m_undefVarErrMsg = szValue;
            } else {
                scriptSourceDef.m_undefVarErrMsg = scriptSourceDef.GetDefaultUndefVarErrMsg();
            }
        }
    }
}

void Settings::SaveOptions(const tstring& prefix, const ScriptSourceDef& scriptSourceDef)
{
	tstring strConfigFileName = GetConfigFileName();

    WritePrivateProfileString(PROFILE_JSLINT_GROUP_NAME, PROFILE_BUILD_KEY_NAME,
        STR(VERSION_BUILD), strConfigFileName.c_str());

    if (scriptSourceDef.m_scriptSource == SCRIPT_SOURCE_BUILTIN) {
        WritePrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
            (prefix + PROFILE_SCRIPT_SOURCE_KEY_NAME).c_str(),
            PROFILE_SCRIPT_SOURCE_BUILTIN, strConfigFileName.c_str());
    } else {
        WritePrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
            (prefix + PROFILE_SCRIPT_SOURCE_KEY_NAME).c_str(),
            PROFILE_SCRIPT_SOURCE_DOWNLOADED, strConfigFileName.c_str());
    }

    WritePrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
        (prefix + PROFILE_SCRIPT_VERSION_KEY_NAME).c_str(),
        scriptSourceDef.m_scriptVersion.c_str(), strConfigFileName.c_str());

    WritePrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
        (prefix + PROFILE_SPEC_UNDEF_VAR_ERR_MSG_KEY_NAME).c_str(),
        scriptSourceDef.m_bSpecUndefVarErrMsg ? TEXT("true") : TEXT("false"), 
        strConfigFileName.c_str());

    WritePrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
        (prefix + PROFILE_UNDEF_VAR_ERR_MSG_KEY_NAME).c_str(),
        scriptSourceDef.m_undefVarErrMsg.c_str(), strConfigFileName.c_str());
}

void Settings::LoadVersions(HWND hDlg, int versionsComboBoxID, Linter linter)
{
    ComboBox_ResetContent(GetDlgItem(hDlg, versionsComboBoxID));

    const map<tstring, JSLintVersion>& versions = DownloadJSLint::GetInstance().GetVersions(linter);
    for (map<tstring, JSLintVersion>::const_iterator it = versions.begin(); it != versions.end(); ++it) {
        ComboBox_AddString(GetDlgItem(hDlg, versionsComboBoxID), it->first.c_str());
    }
}

BOOL Settings::UpdateOptions(HWND hDlg, bool bSaveOrValidate)
{
    if (bSaveOrValidate) {
        if (IsDlgButtonChecked(hDlg, IDC_RADIO1)) {
            m_jsLintScript.m_scriptSource = SCRIPT_SOURCE_BUILTIN;
        } else {
            m_jsLintScript.m_scriptSource = SCRIPT_SOURCE_DOWNLOADED;

            int nCurSel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION));
            if (nCurSel == CB_ERR) {
			    MessageBox(hDlg,
                    TEXT("Please select JSLint script version!"),
                    TEXT("JSLint"), MB_OK | MB_ICONERROR);
			    SetFocus(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION));
                return FALSE;
            }
            m_jsLintScript.m_scriptVersion = GetWindowText(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION));

            if (IsDlgButtonChecked(hDlg, IDC_JSLINT_SPEC_UNDEF_VAR_ERR_MSG)) {
                m_jsLintScript.m_bSpecUndefVarErrMsg = true;
                
                tstring undefVarErrMsg = GetWindowText(GetDlgItem(hDlg, IDC_JSLINT_UNDEF_VAR_ERR_MSG));
                if (undefVarErrMsg.empty()) {
				    MessageBox(hDlg,
                        TEXT("Please enter 'undefined variable' error message text!"),
                        TEXT("JSLint"), MB_OK | MB_ICONERROR);
				    SetFocus(GetDlgItem(hDlg, IDC_JSLINT_UNDEF_VAR_ERR_MSG));
                    return FALSE;
                }
                m_jsLintScript.m_undefVarErrMsg = undefVarErrMsg;
            } else {
                m_jsLintScript.m_bSpecUndefVarErrMsg = false;
            }
        }
    } else {
        if (m_jsLintScript.m_scriptSource == SCRIPT_SOURCE_BUILTIN) {
            CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
        } else {
            CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
        }

        ComboBox_SelectString(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION), -1,  m_jsLintScript.m_scriptVersion.c_str());

        if (m_jsLintScript.m_bSpecUndefVarErrMsg) {
            CheckDlgButton(hDlg, IDC_JSLINT_SPEC_UNDEF_VAR_ERR_MSG, BST_CHECKED);
        } else {
            CheckDlgButton(hDlg, IDC_JSLINT_SPEC_UNDEF_VAR_ERR_MSG, BST_UNCHECKED);
        }

        SetWindowText(GetDlgItem(hDlg, IDC_JSLINT_UNDEF_VAR_ERR_MSG), 
            m_jsLintScript.m_undefVarErrMsg.c_str());
    }

    if (bSaveOrValidate) {
        if (IsDlgButtonChecked(hDlg, IDC_RADIO3)) {
            m_jsHintScript.m_scriptSource = SCRIPT_SOURCE_BUILTIN;
        } else {
            m_jsHintScript.m_scriptSource = SCRIPT_SOURCE_DOWNLOADED;

            int nCurSel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_JSHINT_SCRIPT_VERSION));
            if (nCurSel == CB_ERR) {
			    MessageBox(hDlg,
                    TEXT("Please select JSHint script version!"),
                    TEXT("JSLint"), MB_OK | MB_ICONERROR);
			    SetFocus(GetDlgItem(hDlg, IDC_JSHINT_SCRIPT_VERSION));
                return FALSE;
            }
            m_jsHintScript.m_scriptVersion = GetWindowText(GetDlgItem(hDlg, IDC_JSHINT_SCRIPT_VERSION));

            if (IsDlgButtonChecked(hDlg, IDC_JSHINT_SPEC_UNDEF_VAR_ERR_MSG)) {
                m_jsHintScript.m_bSpecUndefVarErrMsg = true;
                
                tstring undefVarErrMsg = GetWindowText(GetDlgItem(hDlg, IDC_JSHINT_UNDEF_VAR_ERR_MSG));
                if (undefVarErrMsg.empty()) {
				    MessageBox(hDlg,
                        TEXT("Please enter 'undefined variable' error message text!"),
                        TEXT("JSLint"), MB_OK | MB_ICONERROR);
				    SetFocus(GetDlgItem(hDlg, IDC_JSHINT_UNDEF_VAR_ERR_MSG));
                    return FALSE;
                }
                m_jsHintScript.m_undefVarErrMsg = undefVarErrMsg;
            } else {
                m_jsHintScript.m_bSpecUndefVarErrMsg = false;
            }
        }
    } else {
        if (m_jsHintScript.m_scriptSource == SCRIPT_SOURCE_BUILTIN) {
            CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO3);
        } else {
            CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO4);
        }

        ComboBox_SelectString(GetDlgItem(hDlg, IDC_JSHINT_SCRIPT_VERSION), -1,  m_jsHintScript.m_scriptVersion.c_str());

        if (m_jsHintScript.m_bSpecUndefVarErrMsg) {
            CheckDlgButton(hDlg, IDC_JSHINT_SPEC_UNDEF_VAR_ERR_MSG, BST_CHECKED);
        } else {
            CheckDlgButton(hDlg, IDC_JSHINT_SPEC_UNDEF_VAR_ERR_MSG, BST_UNCHECKED);
        }

        SetWindowText(GetDlgItem(hDlg, IDC_JSHINT_UNDEF_VAR_ERR_MSG), 
            m_jsHintScript.m_undefVarErrMsg.c_str());
    }

    return TRUE;
}

void Settings::UpdateControls(HWND hDlg)
{
    BOOL bDownload;

    bDownload = IsDlgButtonChecked(hDlg, IDC_RADIO2);
    EnableWindow(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION_LABEL), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_JSLINT_DOWNLOAD_LATEST), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_JSLINT_SPEC_UNDEF_VAR_ERR_MSG), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_JSLINT_UNDEF_VAR_ERR_MSG), bDownload &&
        IsDlgButtonChecked(hDlg, IDC_JSLINT_SPEC_UNDEF_VAR_ERR_MSG));

    bDownload = IsDlgButtonChecked(hDlg, IDC_RADIO4);
    EnableWindow(GetDlgItem(hDlg, IDC_JSHINT_SCRIPT_VERSION_LABEL), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_JSHINT_SCRIPT_VERSION), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_JSHINT_DOWNLOAD_LATEST), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_JSHINT_SPEC_UNDEF_VAR_ERR_MSG), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_JSHINT_UNDEF_VAR_ERR_MSG), bDownload &&
        IsDlgButtonChecked(hDlg, IDC_JSHINT_SPEC_UNDEF_VAR_ERR_MSG));
}

INT_PTR CALLBACK Settings::DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    static Settings settings;

    if (uMessage == WM_INITDIALOG) {
        settings = GetInstance();
        settings.LoadVersions(hDlg, IDC_JSLINT_SCRIPT_VERSION, LINTER_JSLINT);
        settings.LoadVersions(hDlg, IDC_JSHINT_SCRIPT_VERSION, LINTER_JSHINT);
        settings.UpdateOptions(hDlg, false);
        settings.UpdateControls(hDlg);
        CenterWindow(hDlg, g_nppData._nppHandle);
    } else if (uMessage == WM_COMMAND) {
		if (HIWORD(wParam) == BN_CLICKED) {
            tstring latestVersion;

		    switch (LOWORD(wParam)) {
                case IDC_RADIO1:
                case IDC_RADIO2:
                case IDC_JSLINT_SPEC_UNDEF_VAR_ERR_MSG:
                case IDC_RADIO3:
                case IDC_RADIO4:
                case IDC_JSHINT_SPEC_UNDEF_VAR_ERR_MSG:
                    settings.UpdateControls(hDlg);
                    break;
                case IDC_JSLINT_DOWNLOAD_LATEST:
                    switch (DownloadJSLint::GetInstance().DownloadLatest(LINTER_JSLINT, latestVersion)) {
                        case DownloadJSLint::DOWNLOAD_OK:
                            settings.LoadVersions(hDlg, IDC_JSLINT_SCRIPT_VERSION, LINTER_JSLINT);
                            ComboBox_SelectString(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION),
                                -1, latestVersion.c_str());
                            break;
                        case DownloadJSLint::DOWNLOAD_NO_NEW_VERSION:
                            MessageBox(hDlg, TEXT("You already have the latest version!"), TEXT("JSLint"), MB_OK | MB_ICONEXCLAMATION);
                            break;
                        case DownloadJSLint::DOWNLOAD_FAILED:
                            MessageBox(hDlg, TEXT("Download error!"), TEXT("JSLint"), MB_OK | MB_ICONERROR);
                            break;
                    }
                    break;
                case IDC_JSHINT_DOWNLOAD_LATEST:
                    switch (DownloadJSLint::GetInstance().DownloadLatest(LINTER_JSHINT, latestVersion)) {
                        case DownloadJSLint::DOWNLOAD_OK:
                            settings.LoadVersions(hDlg, IDC_JSHINT_SCRIPT_VERSION, LINTER_JSHINT);
                            ComboBox_SelectString(GetDlgItem(hDlg, IDC_JSHINT_SCRIPT_VERSION),
                                -1, latestVersion.c_str());
                            break;
                        case DownloadJSLint::DOWNLOAD_NO_NEW_VERSION:
                            MessageBox(hDlg, TEXT("You already have the latest version!"), TEXT("JSLint"), MB_OK | MB_ICONEXCLAMATION);
                            break;
                        case DownloadJSLint::DOWNLOAD_FAILED:
                            MessageBox(hDlg, TEXT("Download error!"), TEXT("JSLint"), MB_OK | MB_ICONERROR);
                            break;
                    }
                    break;
			    case IDOK:
                    if (settings.UpdateOptions(hDlg, true)) {
                        GetInstance() = settings;
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
    } else if (uMessage == WM_SYSCOMMAND) {
        if (wParam == SC_CLOSE) {
            // cancel
            EndDialog(hDlg, 0);
            return 1;
        }
    }

	return 0;
}

void Settings::ShowDialog()
{
	pluginDialogBox(IDD_SETTINGS, DlgProc);
}

ScriptSourceDef& Settings::GetScriptSource(Linter linter)
{
    return linter == LINTER_JSLINT ? m_jsLintScript : m_jsHintScript;
}