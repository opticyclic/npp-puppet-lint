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

////////////////////////////////////////////////////////////////////////////////

#define PROFILE_JSLINT_GROUP_NAME TEXT("JSLint")
#define PROFILE_BUILD_KEY_NAME TEXT("build")

////////////////////////////////////////////////////////////////////////////////

class Settings
{
	Settings();

public:
    static Settings& GetInstance();

	void ReadOptions();
	void SaveOptions();

	enum JSLintScriptSource {
		JSLINT_SCRIPT_SOURCE_BUILTIN,
		JSLINT_SCRIPT_SOURCE_DOWNLOADED
	};

    JSLintScriptSource GetJSLintScriptSource() { 
        return m_jsLintScriptSource; 
    }
    void SetJSLintScriptSource(JSLintScriptSource jsLintScriptSource) { 
        m_jsLintScriptSource = jsLintScriptSource; 
    }

    tstring GetJSLintScriptVersion() { 
        return m_jsLintScriptVersion; 
    }
    void SetJSLintScriptVersion(tstring jsLintScriptVersion) { 
        m_jsLintScriptVersion = jsLintScriptVersion; 
    }

    bool GetSpecUndefVarErrMsg() { 
        return m_bSpecUndefVarErrMsg; 
    }
    void SetSpecUndefVarErrMsg(bool bSpecUndefVarErrMsg) { 
        m_bSpecUndefVarErrMsg = bSpecUndefVarErrMsg; 
    }

    tstring GetUndefVarErrMsg() { 
        return m_undefVarErrMsg; 
    }
    void SetUndefVarErrMsg(const tstring& undefVarErrMsg) { 
        m_undefVarErrMsg = undefVarErrMsg; 
    }

    void ShowDialog();

private:
    JSLintScriptSource m_jsLintScriptSource;
    tstring m_jsLintScriptVersion;
    bool m_bSpecUndefVarErrMsg;
    tstring m_undefVarErrMsg;

    void LoadVersions(HWND hDlg);
    BOOL UpdateOptions(HWND hDlg, bool bSaveOrValidate);
    void UpdateControls(HWND hDlg);

    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};
