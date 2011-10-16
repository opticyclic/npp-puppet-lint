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
#include "DownloadJSLint.h"
#include "resource.h"
#include "Util.h"
#include "Version.h"

////////////////////////////////////////////////////////////////////////////////

#define PROFILE_SETTINGS_GROUP_NAME TEXT("Settings")
#define PROFILE_JSLINT_SCRIPT_SOURCE_KEY_NAME TEXT("jslint_script_source")
#define PROFILE_JSLINT_SCRIPT_SOURCE_BUILTIN TEXT("builtin")
#define PROFILE_JSLINT_SCRIPT_SOURCE_DOWNLOADED TEXT("downloaded")
#define PROFILE_JSLINT_SCRIPT_VERSION_KEY_NAME TEXT("jslint_script_version")
#define PROFILE_SPEC_UNDEF_VAR_ERR_MSG_KEY_NAME TEXT("spec_undef_var_err_msg")
#define PROFILE_UNDEF_VAR_ERR_MSG_KEY_NAME TEXT("undef_var_err_msg")

#define MIN_VERSION_BUILD 110

////////////////////////////////////////////////////////////////////////////////

Settings::Settings()
    : m_jsLintScriptSource(JSLINT_SCRIPT_SOURCE_BUILTIN)
    , m_bSpecUndefVarErrMsg(false)
    , m_undefVarErrMsg(DEFAULT_UNDEF_VAR_ERR_MSG)
{
}

Settings& Settings::GetInstance()
{
    static Settings singleton;
    return singleton;
}

void Settings::ReadOptions()
{
    TCHAR szValue[65536]; // memory is cheap

    tstring strConfigFileName = GetConfigFileName();
    if (Path::IsFileExists(strConfigFileName)) {
	    GetPrivateProfileString(PROFILE_JSLINT_GROUP_NAME, PROFILE_BUILD_KEY_NAME,
            NULL, szValue, _countof(szValue), strConfigFileName.c_str());
        if (_ttoi(szValue) >= MIN_VERSION_BUILD) {
            if (_ttoi(szValue) >= VERSION_BUILD) {
	            GetPrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
                    PROFILE_JSLINT_SCRIPT_SOURCE_KEY_NAME,
                    NULL, szValue, _countof(szValue), strConfigFileName.c_str());
                if (_tcscmp(szValue, PROFILE_JSLINT_SCRIPT_SOURCE_DOWNLOADED) == 0) {
                    m_jsLintScriptSource = JSLINT_SCRIPT_SOURCE_DOWNLOADED;
                } else {
                    m_jsLintScriptSource = JSLINT_SCRIPT_SOURCE_BUILTIN;
                }
            } else {
                // if old config switch to builtin source
                m_jsLintScriptSource = JSLINT_SCRIPT_SOURCE_BUILTIN;
            }

	        GetPrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
                PROFILE_JSLINT_SCRIPT_VERSION_KEY_NAME,
                NULL, szValue, _countof(szValue), strConfigFileName.c_str());
            m_jsLintScriptVersion = szValue;

	        GetPrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
                PROFILE_SPEC_UNDEF_VAR_ERR_MSG_KEY_NAME,
                NULL, szValue, _countof(szValue), strConfigFileName.c_str());
            m_bSpecUndefVarErrMsg = _tcscmp(szValue, TEXT("true")) == 0;

	        GetPrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
                PROFILE_UNDEF_VAR_ERR_MSG_KEY_NAME,
                NULL, szValue, _countof(szValue), strConfigFileName.c_str());
            if (_tcslen(szValue) > 0) {
                m_undefVarErrMsg = szValue;
            } else {
                m_undefVarErrMsg = DEFAULT_UNDEF_VAR_ERR_MSG;
            }
        }
    }
}

void Settings::SaveOptions()
{
	tstring strConfigFileName = GetConfigFileName();

    WritePrivateProfileString(PROFILE_JSLINT_GROUP_NAME, PROFILE_BUILD_KEY_NAME,
        STR(VERSION_BUILD), strConfigFileName.c_str());

    if (m_jsLintScriptSource == JSLINT_SCRIPT_SOURCE_BUILTIN) {
        WritePrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
            PROFILE_JSLINT_SCRIPT_SOURCE_KEY_NAME,
            PROFILE_JSLINT_SCRIPT_SOURCE_BUILTIN, strConfigFileName.c_str());
    } else {
        WritePrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
            PROFILE_JSLINT_SCRIPT_SOURCE_KEY_NAME,
            PROFILE_JSLINT_SCRIPT_SOURCE_DOWNLOADED, strConfigFileName.c_str());
    }

    WritePrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
        PROFILE_JSLINT_SCRIPT_VERSION_KEY_NAME,
        m_jsLintScriptVersion.c_str(), strConfigFileName.c_str());

    WritePrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
        PROFILE_SPEC_UNDEF_VAR_ERR_MSG_KEY_NAME,
        m_bSpecUndefVarErrMsg ? TEXT("true") : TEXT("false"), 
        strConfigFileName.c_str());

    WritePrivateProfileString(PROFILE_SETTINGS_GROUP_NAME, 
        PROFILE_UNDEF_VAR_ERR_MSG_KEY_NAME,
        m_undefVarErrMsg.c_str(), strConfigFileName.c_str());
}

void Settings::LoadVersions(HWND hDlg)
{
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION));

    const map<tstring, JSLintVersion>& versions = DownloadJSLint::GetInstance().GetVersions();
    for (map<tstring, JSLintVersion>::const_iterator it = versions.begin(); it != versions.end(); ++it) {
        ComboBox_AddString(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION), it->first.c_str());
    }
}

BOOL Settings::UpdateOptions(HWND hDlg, bool bSaveOrValidate)
{
    if (bSaveOrValidate) {
        if (IsDlgButtonChecked(hDlg, IDC_RADIO1)) {
            SetJSLintScriptSource(Settings::JSLINT_SCRIPT_SOURCE_BUILTIN);
        } else {
            SetJSLintScriptSource(Settings::JSLINT_SCRIPT_SOURCE_DOWNLOADED);

            int nCurSel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION));
            if (nCurSel == CB_ERR) {
			    MessageBox(hDlg,
                    TEXT("Please select JSLint script version!"),
                    TEXT("JSLint"), MB_OK | MB_ICONERROR);
			    SetFocus(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION));
                return FALSE;
            }
            SetJSLintScriptVersion(GetWindowText(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION)));

            if (IsDlgButtonChecked(hDlg, IDC_SPEC_UNDEF_VAR_ERR_MSG)) {
                SetSpecUndefVarErrMsg(true);
                
                tstring undefVarErrMsg = GetWindowText(GetDlgItem(hDlg, IDC_UNDEF_VAR_ERR_MSG));
                if (undefVarErrMsg.empty()) {
				    MessageBox(hDlg,
                        TEXT("Please enter 'undefined variable' error message text!"),
                        TEXT("JSLint"), MB_OK | MB_ICONERROR);
				    SetFocus(GetDlgItem(hDlg, IDC_UNDEF_VAR_ERR_MSG));
                    return FALSE;
                }
                SetUndefVarErrMsg(undefVarErrMsg);
            } else {
                SetSpecUndefVarErrMsg(false);
            }
        }
    } else {
        if (GetJSLintScriptSource() == Settings::JSLINT_SCRIPT_SOURCE_BUILTIN) {
            CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
        } else {
            CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
        }

        ComboBox_SelectString(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION), -1,  GetJSLintScriptVersion().c_str());

        if (GetSpecUndefVarErrMsg()) {
            CheckDlgButton(hDlg, IDC_SPEC_UNDEF_VAR_ERR_MSG, BST_CHECKED);
        } else {
            CheckDlgButton(hDlg, IDC_SPEC_UNDEF_VAR_ERR_MSG, BST_UNCHECKED);
        }

        SetWindowText(GetDlgItem(hDlg, IDC_UNDEF_VAR_ERR_MSG), 
            GetUndefVarErrMsg().c_str());
    }

    return TRUE;
}

void Settings::UpdateControls(HWND hDlg)
{
    BOOL bDownload = IsDlgButtonChecked(hDlg, IDC_RADIO2);
    EnableWindow(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION_LABEL), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_DOWNLOAD_LATEST), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_SPEC_UNDEF_VAR_ERR_MSG), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_UNDEF_VAR_ERR_MSG), bDownload);
    EnableWindow(GetDlgItem(hDlg, IDC_UNDEF_VAR_ERR_MSG), bDownload &&
        IsDlgButtonChecked(hDlg, IDC_SPEC_UNDEF_VAR_ERR_MSG));
}

INT_PTR CALLBACK Settings::DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    static Settings settings;

    if (uMessage == WM_INITDIALOG) {
        settings = GetInstance();
        settings.LoadVersions(hDlg);
        settings.UpdateOptions(hDlg, false);
        settings.UpdateControls(hDlg);
        CenterWindow(hDlg, g_nppData._nppHandle);
    } else if (uMessage == WM_COMMAND) {
		if (HIWORD(wParam) == BN_CLICKED) {
            tstring latestVersion;

		    switch (LOWORD(wParam)) {
                case IDC_RADIO1:
                case IDC_RADIO2:
                case IDC_SPEC_UNDEF_VAR_ERR_MSG:
                    settings.UpdateControls(hDlg);
                    break;
                case IDC_DOWNLOAD_LATEST:
                    switch (DownloadJSLint::GetInstance().DownloadLatest(latestVersion)) {
                        case DownloadJSLint::DOWNLOAD_OK:
                            settings.LoadVersions(hDlg);
                            ComboBox_SelectString(GetDlgItem(hDlg, IDC_JSLINT_SCRIPT_VERSION),
                                -1,  latestVersion.c_str());
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