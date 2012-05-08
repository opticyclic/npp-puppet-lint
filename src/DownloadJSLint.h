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

#include "JSLintOptions.h"

////////////////////////////////////////////////////////////////////////////////

class JSLintVersion {
public:
    JSLintVersion() {}
    JSLintVersion(const tstring& fileName) : m_fileName(fileName) {}
    JSLintVersion(const tstring& fileName, const string& content) 
        : m_fileName(fileName), m_content(content) {}

    tstring GetFileName() const { return m_fileName; }
    string GetContent();

private:
    tstring m_fileName;
    string m_content;
};

////////////////////////////////////////////////////////////////////////////////

class DownloadJSLint
{
    DownloadJSLint();

public:
    static DownloadJSLint& GetInstance();

    void LoadVersions();
    
    enum DownloadResult {
        DOWNLOAD_OK,
        DOWNLOAD_NO_NEW_VERSION,
        DOWNLOAD_FAILED
    };
    DownloadResult DownloadLatest(Linter linter, tstring& latestVersion);

    const map<tstring, JSLintVersion>& GetVersions(Linter linter) const;
    bool HasVersion(Linter linter, const tstring& version);
    JSLintVersion& GetVersion(Linter linter, const tstring& version);

private:
    tstring m_versionsFolder;
    map<tstring, JSLintVersion> m_jsLintVersions;
    map<tstring, JSLintVersion> m_jsHintVersions;

    HWND m_hDlg;
    int m_nStatusID;
    HINTERNET m_hSession;
    HINTERNET m_hConnect;
    HINTERNET m_hRequest;
    DWORD m_dwSize;
    DWORD m_dwTotalSize;
    LPSTR m_lpBuffer;
    tstring m_version;
    DownloadResult m_result;

    static Linter m_linter;

    void LoadVersions(const tstring& fileSpec, map<tstring, JSLintVersion>& versions);

    tstring GetVersionsFolder();

    void CleanupContext();
    void DownloadOK();
    void DownloadNoNewVersion();
    void DownloadFailed();
    bool CheckVersion();
    static void CALLBACK AsyncCallback(HINTERNET hInternet,
        DWORD_PTR dwContext,
        DWORD dwInternetStatus,
        LPVOID lpvStatusInformation,
        DWORD dwStatusInformationLength);
    void AsyncCallbackHandler(DWORD dwInternetStatus,
        LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
    void StartDownload(HWND hDlg, int nStatusID);
    static INT_PTR CALLBACK JSLintDownloadProgressDlgProc(HWND hDlg, UINT uMessage,
        WPARAM wParam, LPARAM lParam);
};
