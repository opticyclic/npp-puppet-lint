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
#include "DownloadJSLint.h"
#include "PluginDefinition.h"
#include "Util.h"
#include "resource.h"

////////////////////////////////////////////////////////////////////////////////


#define JSLINT_GITHUB_URL_W L"https://raw.github.com/douglascrockford/JSLint/master/jslint.js"
#define JSLINT_GITHUB_URL_T TEXT("https://raw.github.com/douglascrockford/JSLint/master/jslint.js")

#define JSHINT_GITHUB_URL_W L"https://raw.github.com/jshint/jshint/master/dist/jshint.js"
#define JSHINT_GITHUB_URL_T TEXT("https://raw.github.com/jshint/jshint/master/dist/jshint.js")

#define WM_DOWNLOAD_FINISHED WM_USER + 1

////////////////////////////////////////////////////////////////////////////////

string JSLintVersion::GetContent()
{
    if (m_content.empty()) {
        FILE* fp = _tfopen(m_fileName.c_str(), TEXT("rb"));
        if (fp != NULL) {
            fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            if (size > 0) {
                fseek(fp, 0, SEEK_SET);
                char* buffer = new char[size + 1];
                size_t nRead = fread(buffer, 1, size, fp);
                if (nRead == size) {
                    m_content = string(buffer, size);
                }
                delete [] buffer;
            }
        }
    }

    return m_content;
}

////////////////////////////////////////////////////////////////////////////////

Linter DownloadJSLint::m_linter;

DownloadJSLint::DownloadJSLint()
{
}

DownloadJSLint& DownloadJSLint::GetInstance()
{
    static DownloadJSLint singleton;
    return singleton;
}

void DownloadJSLint::LoadVersions()
{
    LoadVersions(TEXT("jslint.*.js"), m_jsLintVersions);
    LoadVersions(TEXT("jshint.*.js"), m_jsHintVersions);
}

void DownloadJSLint::LoadVersions(const tstring& fileSpec, map<tstring, JSLintVersion>& versions)
{
    TCHAR szConfigDir[MAX_PATH];
    szConfigDir[0] = 0;
    ::SendMessage(g_nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, 
	    MAX_PATH, (LPARAM)szConfigDir);
    m_versionsFolder = Path::GetFullPath(TEXT("JSLint"), szConfigDir);
    if (!Path::IsFileExists(m_versionsFolder)) {
        CreateDirectory(m_versionsFolder.c_str(), NULL);
    } 

    WIN32_FIND_DATA findFileData;
    HANDLE findFileHandle = FindFirstFile(Path::GetFullPath(fileSpec.c_str(), m_versionsFolder).c_str(), &findFileData);
    if (findFileHandle != INVALID_HANDLE_VALUE) {
        do {
            versions.insert(make_pair<tstring, JSLintVersion>(
                    tstring(findFileData.cFileName).substr(7, _tcslen(findFileData.cFileName) - 10),
                    JSLintVersion(Path::GetFullPath(findFileData.cFileName, m_versionsFolder))
                ));
        } while (FindNextFile(findFileHandle, &findFileData));
        FindClose(findFileHandle);
    }
}

const map<tstring, JSLintVersion>& DownloadJSLint::GetVersions(Linter linter) const
{ 
    return linter == LINTER_JSLINT ? m_jsLintVersions : m_jsHintVersions;
}

bool DownloadJSLint::HasVersion(Linter linter, const tstring& version)
{
    if (linter == LINTER_JSLINT) {
        return m_jsLintVersions.find(version) != m_jsLintVersions.end();
    } else {
        return m_jsHintVersions.find(version) != m_jsHintVersions.end();
    }
}

JSLintVersion& DownloadJSLint::GetVersion(Linter linter, const tstring& version)
{
    return linter == LINTER_JSLINT ? m_jsLintVersions[version] : m_jsHintVersions[version];
}

DownloadJSLint::DownloadResult DownloadJSLint::DownloadLatest(Linter linter, tstring& latestVersion)
{
    m_linter = linter;    

    if (pluginDialogBox(IDD_DOWNLOAD_PROGRESS, JSLintDownloadProgressDlgProc) == IDOK) {
        latestVersion = m_version;
        if (latestVersion.empty()) {
            SYSTEMTIME time;
            GetLocalTime(&time);

            TCHAR szTime[1024];
            _stprintf(szTime, TEXT("%.4d-%.2d-%.2d %.2d-%.2d-%.2d"),
                time.wYear, time.wMonth, time.wDay,
                time.wHour, time.wMinute, time.wSecond);
        
            latestVersion = szTime;
        }

        tstring fileName = Path::GetFullPath((linter == LINTER_JSLINT ? TEXT("jslint.") : TEXT("jshint.")) + latestVersion + TEXT(".js"), m_versionsFolder);

        size_t nWritten = 0;

        FILE* fp = _tfopen(fileName.c_str(), TEXT("wb+"));
        if (fp != NULL) {
            nWritten = fwrite(m_lpBuffer, 1, m_dwTotalSize, fp);
            fclose(fp);
        }

        string content(m_lpBuffer, m_dwTotalSize);

        if (nWritten == m_dwTotalSize) {
            if (linter == LINTER_JSLINT) {
                m_jsLintVersions.insert(std::make_pair(latestVersion, JSLintVersion(fileName, content)));
            } else {
                m_jsHintVersions.insert(std::make_pair(latestVersion, JSLintVersion(fileName, content)));
            }
            m_result = DOWNLOAD_OK;
        } else {
            m_result = DOWNLOAD_FAILED;
        }
    }

    delete [] m_lpBuffer;
    m_lpBuffer = NULL;

    return m_result;
}

void DownloadJSLint::CleanupContext()
{
    if (m_hRequest) {
        WinHttpSetStatusCallback(m_hRequest, NULL, NULL, NULL);
        WinHttpCloseHandle(m_hRequest);
		m_hRequest = NULL;
    }

    if (m_hConnect) {
        WinHttpCloseHandle(m_hConnect);
		m_hConnect = NULL;
    }

    if (m_hSession != NULL) {
        WinHttpCloseHandle(m_hSession);
        m_hSession = NULL;
    }
}

void DownloadJSLint::DownloadOK()
{
    CleanupContext();
    m_result = DownloadJSLint::DOWNLOAD_OK;
    PostMessage(m_hDlg, WM_DOWNLOAD_FINISHED, 1, 0);
}

void DownloadJSLint::DownloadNoNewVersion()
{
    CleanupContext();
    m_result = DownloadJSLint::DOWNLOAD_NO_NEW_VERSION;
    PostMessage(m_hDlg, WM_DOWNLOAD_FINISHED, 0, 0);
}

void DownloadJSLint::DownloadFailed()
{
    CleanupContext();
    m_result = DownloadJSLint::DOWNLOAD_FAILED;
    PostMessage(m_hDlg, WM_DOWNLOAD_FINISHED, 0, 0);
}

bool DownloadJSLint::CheckVersion()
{
    if (m_version.empty()) {
        if (m_linter == LINTER_JSLINT) {
            // JSLint has version identification, in the form of date (for example: 2013-11-23), on the second line of source file
            char *j = NULL; // skipt first line
            for (char* i = m_lpBuffer; i < m_lpBuffer + m_dwTotalSize; ++i) {
                if (*i == '\n') {
                    if (j) {
                        j += 3; // skip '// '
                        m_version = TextConversion::A_To_T(string(j, i - j));
                        m_version = TrimSpaces(m_version);
                        if (HasVersion(m_linter, m_version)) {
                            return false;
                        }
                        return true;
                    } else {
                        j = i;
                    }
                }
            }
        } else if (m_linter == LINTER_JSHINT) {
            // JSLint has version identification, in the form of version number (for example 2.3.0), on the second line of source file
            for (char* i = m_lpBuffer; i < m_lpBuffer + m_dwTotalSize; ++i) {
                if (*i == '\n') {
                    char *j = m_lpBuffer + 3; // skip '// '
                    m_version = TextConversion::A_To_T(string(j, i - j));
                    m_version = TrimSpaces(m_version);
                    if (HasVersion(m_linter, m_version)) {
                        return false;
                    }
                    return true;
                }
            }
        }
    }
    return true;
}

void CALLBACK DownloadJSLint::AsyncCallback(HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength)
{
    GetInstance().AsyncCallbackHandler(dwInternetStatus, 
        lpvStatusInformation, dwStatusInformationLength);
}

void DownloadJSLint::AsyncCallbackHandler(DWORD dwInternetStatus,
    LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
    // Create a string that reflects the status flag.
    switch (dwInternetStatus) {
        case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
            if (!WinHttpReceiveResponse(m_hRequest, NULL)) {
                DownloadFailed();
            }
            break;

        case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
            if (!WinHttpQueryDataAvailable(m_hRequest, NULL)) {
                DownloadFailed();
            }
            break;

        case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
            m_dwSize = *((LPDWORD)lpvStatusInformation);

            if (m_dwSize == 0) {
                if (m_dwTotalSize) {
                    DownloadOK();
                } else {
                    DownloadFailed();
                }
            } else {
                LPSTR lpOutBuffer = new char[m_dwSize + 1];
                ZeroMemory(lpOutBuffer, m_dwSize + 1);
                if (!WinHttpReadData(m_hRequest, (LPVOID)lpOutBuffer, m_dwSize, NULL)) {
                    delete [] lpOutBuffer;
                    DownloadFailed();
                }
            }
            break;

        case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
            if (dwStatusInformationLength != 0) {
                LPSTR lpReadBuffer = (LPSTR)lpvStatusInformation;
                DWORD dwBytesRead = dwStatusInformationLength;

                m_dwSize = dwBytesRead;

                if (!m_lpBuffer) {
                    m_lpBuffer = lpReadBuffer;
                } else {
                    LPSTR lpOldBuffer = m_lpBuffer;
                    m_lpBuffer = new char[m_dwTotalSize + m_dwSize];

                    memcpy(m_lpBuffer, lpOldBuffer, m_dwTotalSize);
                    memcpy(m_lpBuffer + m_dwTotalSize, lpReadBuffer, m_dwSize);

                    delete [] lpOldBuffer;
                    delete [] lpReadBuffer;
                }
                
                m_dwTotalSize += m_dwSize;

                TCHAR szStatus[1024];
                _stprintf(szStatus, TEXT("Received %d bytes"), m_dwTotalSize);
                SetWindowText(GetDlgItem(m_hDlg, m_nStatusID), szStatus);

                if (!CheckVersion()) {
                    DownloadNoNewVersion();
                } else {
                    if (!WinHttpQueryDataAvailable(m_hRequest, NULL)) {
                        DownloadFailed();
                    }
                }
            }
            break;

        case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
            DownloadFailed();
            break;
    }
}

void DownloadJSLint::StartDownload(HWND hDlg, int nStatusID)
{
    m_hDlg = hDlg;
    m_nStatusID = nStatusID;
    m_hSession = 0;
    m_hConnect = 0;
    m_hRequest = 0;
    m_lpBuffer = NULL;
    m_dwSize = 0;
    m_dwTotalSize = 0;
    m_version = TEXT("");

    m_hSession = WinHttpOpen(L"JSLint Plugin for Notepad++",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        WINHTTP_FLAG_ASYNC);

    if (m_hSession == NULL) {
        DownloadFailed();
        return;
    }

    WCHAR szHost[256];
    URL_COMPONENTS urlComp;

    ZeroMemory(&urlComp, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.lpszHostName = szHost;
    urlComp.dwHostNameLength = sizeof(szHost) / sizeof(szHost[0]);
    urlComp.dwUrlPathLength = (DWORD)-1;
    urlComp.dwSchemeLength = (DWORD)-1;
    WinHttpCrackUrl(m_linter == LINTER_JSLINT ? JSLINT_GITHUB_URL_W : JSHINT_GITHUB_URL_W, 0, 0, &urlComp);

    m_hConnect = WinHttpConnect(m_hSession, szHost, urlComp.nPort, 0);

    DWORD dwOpenRequestFlag = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ?
        WINHTTP_FLAG_SECURE : 0;

    m_hRequest = WinHttpOpenRequest(m_hConnect, 
        L"GET", urlComp.lpszUrlPath, NULL, WINHTTP_NO_REFERER, 
        WINHTTP_DEFAULT_ACCEPT_TYPES, dwOpenRequestFlag);
    if (m_hRequest == NULL) {
        DownloadFailed();
        return;
    }

    WINHTTP_STATUS_CALLBACK pCallback = WinHttpSetStatusCallback(m_hRequest,
        (WINHTTP_STATUS_CALLBACK)AsyncCallback,
        WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS | WINHTTP_CALLBACK_FLAG_REDIRECT, 0);
    if (pCallback != NULL) {
        DownloadFailed();
        return;
    }

    if (!WinHttpSendRequest(m_hRequest, 
                        WINHTTP_NO_ADDITIONAL_HEADERS, 0, 
                        WINHTTP_NO_REQUEST_DATA, 0, 0, 
                        NULL)) {
        DownloadFailed();
        return;
    }
}

INT_PTR CALLBACK DownloadJSLint::JSLintDownloadProgressDlgProc(
    HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	if (uMessage == WM_INITDIALOG) {
		TCHAR szTitleFormat[100];
		GetWindowText(hDlg, szTitleFormat, _countof(szTitleFormat));
        
		TCHAR szTitle[100];
		_stprintf(szTitle, szTitleFormat, m_linter == LINTER_JSLINT ? TEXT("JSLint") : TEXT("JSHint"));
        SetWindowText(hDlg, szTitle);

        SetWindowText(GetDlgItem(hDlg, IDC_URL), m_linter == LINTER_JSLINT ? JSLINT_GITHUB_URL_T : JSHINT_GITHUB_URL_T);
        SetWindowText(GetDlgItem(hDlg, IDC_PROGRESS), TEXT("Starting ..."));
        GetInstance().StartDownload(hDlg, IDC_PROGRESS);

        CenterWindow(hDlg, g_nppData._nppHandle);
    } else if (uMessage == WM_DOWNLOAD_FINISHED) {
        EndDialog(hDlg, wParam);
    }

	return 0;
}