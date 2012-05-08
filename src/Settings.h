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

#pragma once

#include "DownloadJSLint.h"

////////////////////////////////////////////////////////////////////////////////

#define PROFILE_JSLINT_GROUP_NAME TEXT("JSLint")
#define PROFILE_BUILD_KEY_NAME TEXT("build")

////////////////////////////////////////////////////////////////////////////////

enum ScriptSource {
	SCRIPT_SOURCE_BUILTIN,
	SCRIPT_SOURCE_DOWNLOADED
};

struct ScriptSourceDef {
    ScriptSourceDef(Linter linter);

    Linter m_linter;

    ScriptSource m_scriptSource;
    tstring m_scriptVersion;
    bool m_bSpecUndefVarErrMsg;
    tstring m_undefVarErrMsg;

    int GetScriptResourceID();
    LPCTSTR GetDefaultUndefVarErrMsg();
    LPCSTR GetNamespace();
};

////////////////////////////////////////////////////////////////////////////////

class Settings
{
	Settings();

public:
    static Settings& GetInstance();

	void ReadOptions();
	void SaveOptions();

    void ShowDialog();

    ScriptSourceDef& GetScriptSource(Linter linter);

private:
    ScriptSourceDef m_jsLintScript;
    ScriptSourceDef m_jsHintScript;

    void LoadVersions(HWND hDlg, int versionsComboBoxID, Linter linter);
    BOOL UpdateOptions(HWND hDlg, bool bSaveOrValidate);
    void UpdateControls(HWND hDlg);

	void ReadOptions(const tstring& prefix, ScriptSourceDef& scriptSourceDef);
	void SaveOptions(const tstring& prefix, const ScriptSourceDef& scriptSourceDef);

    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};
