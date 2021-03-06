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
#include "PuppetLint.h"
#include "resource.h"
#include "base64.h"
#include "SysMsg.h"

////////////////////////////////////////////////////////////////////////////////

bool PuppetLintReportItem::IsReasonVarIsNotDefined() const
{
	// check if reason matches: /'.+' is not defined/
	const TCHAR *pattern = _T("' is not defined.");
	const size_t len = _tcslen(pattern);
	return m_strReason.size() > len+1 
		&& m_strReason[0] == _T('\'') 
		&& m_strReason.substr(m_strReason.size() - len) == pattern;
}

tstring PuppetLintReportItem::GetUndefinedVar() const
{
	if (!IsReasonVarIsNotDefined())
		return tstring();

	tstring::size_type i1 = m_strReason.find_first_of(_T('\''));
	tstring::size_type i2 = m_strReason.find_last_of(_T('\''));

	return m_strReason.substr(i1+1, i2-i1-1);
}

////////////////////////////////////////////////////////////////////////////////

// JSLint script is executed by using cscript.exe program. 
// Input script is sent to the JSLint by using standard input.
// Result from JSLint is read from the standard output.
void PuppetLint::CheckScript(const string& strOptions, const string& strScript, 
	int nppTabWidth, int jsLintTabWidth, list<PuppetLintReportItem>& items)
{
    if (!m_jsLintScriptFileName) {
        CreateJSLintFile(strScript);
    }

	// initialize process info structure
	PROCESS_INFORMATION piProcInfo;
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// create pipes for standard input/output redirection
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	Win32Handle hChildStdOutputRead;
	Win32Handle hChildStdOutputWrite;
	if (!CreatePipe(&hChildStdOutputRead, &hChildStdOutputWrite, &saAttr, 0)) {
		throw IOException();
	}
	if (!SetHandleInformation(hChildStdOutputRead, HANDLE_FLAG_INHERIT, 0)) {
		throw IOException();
	}

	Win32Handle hChildStdErrorRead;
	Win32Handle hChildStdErrorWrite;
	if (!CreatePipe(&hChildStdErrorRead, &hChildStdErrorWrite, &saAttr, 0)) {
		throw IOException();
	}
	if (!SetHandleInformation(hChildStdErrorRead, HANDLE_FLAG_INHERIT, 0)) {
		throw IOException();
	}

	Win32Handle hChildStdInputRead;
	Win32Handle hChildStdInputWrite;
	if (!CreatePipe(&hChildStdInputRead, &hChildStdInputWrite, &saAttr, 0)) {
		throw IOException();
	}
	if (!SetHandleInformation(hChildStdInputWrite, HANDLE_FLAG_INHERIT, 0)) {
		throw IOException();
	}

	// initialize startup info structure
	STARTUPINFO siStartInfo;
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO); 
	siStartInfo.hStdOutput = hChildStdOutputWrite;
	siStartInfo.hStdError = hChildStdErrorWrite;
	siStartInfo.hStdInput = hChildStdInputRead;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    //Get the full path to puppet-lint
    TCHAR path[MAX_PATH];
    LPWSTR* ptr = NULL;
    DWORD dwRet = SearchPath(NULL, TEXT("puppet-lint.bat"), NULL, MAX_PATH, path, ptr);

    //Run puppet-lint
    tstring commandArgs = TEXT(" --with-context ") + m_jsLintScriptFileName.GetFileName();
    tstring strCmdLine = path + commandArgs;

	BOOL bSuccess = CreateProcess(NULL, (LPTSTR)strCmdLine.c_str(),
		NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &siStartInfo, &piProcInfo);

	if (!bSuccess) {
        // CreateProcess() failed
        // Get the error from the system
        systemMessage(TEXT("Puppet-Lint Failed"));
		throw PuppetLintUnexpectedException();
	}
	Win32Handle hThread = piProcInfo.hThread;
	Win32Handle hProcess = piProcInfo.hProcess;

	// read result from the standard output
	ParseOutput(hProcess, hChildStdOutputRead, strScript, nppTabWidth, jsLintTabWidth, items);

	// read data from the standard error stream
	string strError;
	ReadError(hProcess, hChildStdErrorRead, strError);

	// get exit code
	DWORD exitCode = 0;
	GetExitCodeProcess(hProcess, &exitCode);
}

void PuppetLint::LoadCustomDataResource(HMODULE hModule,
	LPCTSTR lpName, LPCTSTR lpType, LPVOID* ppData, LPDWORD pdwSize)
{
	*pdwSize = 0;
	*ppData = NULL; 

	HRSRC hRes = FindResource(hModule, lpName, lpType);
	if (hRes == NULL) {
		throw PuppetLintResourceException();
	}

	*pdwSize = SizeofResource(hModule, hRes);
	if (*pdwSize == 0) {
		throw PuppetLintResourceException();
	}

	HGLOBAL hResLoad = LoadResource(hModule, hRes);
	if (hResLoad == NULL) {
		throw PuppetLintResourceException();
	}

	*ppData = LockResource(hResLoad);
	if (*ppData == NULL) {
		throw PuppetLintResourceException();
	}
}

void PuppetLint::CreateJSLintFile(const string& content)
{
	// create temp file
	m_jsLintScriptFileName.Create();
	Win32Handle hJSLintFile = CreateFile(m_jsLintScriptFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
	if ((HANDLE)hJSLintFile == INVALID_HANDLE_VALUE) {
		throw IOException();
	}

	// Write text to the temp file
	WriteStringToFile(hJSLintFile, content);
}

void PuppetLint::WriteStringToFile(HANDLE hFile, const string& str)
{
    DWORD dwSize = (DWORD)strlen(str.c_str()) * sizeof(char);
    DWORD dwWritten;
    if (WriteFile(hFile, (LPVOID)str.c_str(), dwSize, &dwWritten, NULL) == FALSE || dwWritten != dwSize) {
        throw IOException();
    }
}

void PuppetLint::WriteString(HANDLE hFile, const string& str)
{
	string strEncoded = base64_encode((unsigned char const*)str.c_str(), str.length());
	DWORD dwSize = (DWORD)strlen(strEncoded.c_str()) * sizeof(char);
	DWORD dwWritten;
	if (WriteFile(hFile, (LPVOID)strEncoded.c_str(), dwSize, &dwWritten, NULL) == FALSE || dwWritten != dwSize) {
		throw IOException();
	}
}

void PuppetLint::ParseOutput(HANDLE hProcess, HANDLE hPipe, const string& strScript,
	int nppTabWidth, int jsLintTabWidth, list<PuppetLintReportItem>& items)
{
    //Split the original script into lines so we can match up with the lints later
    tstring strScriptW(TextConversion::A_To_T(strScript));
    vector<tstring> results;
    StringSplit(strScriptW, _T("\r"), results);

	// read Puppet-Lint output
	string strOutput;
	while (true) {
		// read data from pipe
		char buffer[512];
		DWORD dwRead;
		if (!ReadFromPipe(hProcess, hPipe, buffer, _countof(buffer), dwRead))
			return;

		strOutput += string(buffer, dwRead);

		// parse puppet-lint output, each lint is delimited with empty line e.g.
        //WARNING: double quoted string containing no variables on line 10
        //
        //  groups => "dba",
        //            ^
        //
		while (true) {
			size_t i = strOutput.find("\r\n\r\n");
			//Break if we can't find double line feed
            if (i == string::npos) {
				break;
			}

            //Save reason
			string strLint = strOutput.substr(0, i);
            string strReason = strLint;

            //Find line number
            size_t onLine = strLint.find_last_of("on line ");
            string lineNum = strLint.substr(onLine);
            int line = atoi(lineNum.c_str());

            //Find the original indents
            strOutput = strOutput.substr(i + 4);
            tstring origLine = results.at(line - 1).substr(1);
            size_t spaces = origLine.find_first_not_of(' ');

            //Find column number (^ symbol on next line of output)
            i = strOutput.find("\r\n");
            strOutput = strOutput.substr(i + 2);
            size_t carat = strOutput.find("^");
            //puppet-lint context is always indented by 2
            //not the number of indents in the original file
            int character = carat + 1 + spaces - 2;

            //Push output to panel
			items.push_back(PuppetLintReportItem(line - 1, character - 1, TextConversion::UTF8_To_T(strReason)));

            //Skip to next lint
            i = strOutput.find("\r\n\r\n");
            strOutput = strOutput.substr(i + 4);
		}
	}
}

int PuppetLint::GetNumTabs(const string& strScript, int line, int character, int tabWidth)
{
	int numTabs = 0;

	size_t i = 0;

	while (line-- > 0) {
		i = strScript.find('\n', i) + 1;
	}

	while (character > 0) {
        if (i < strScript.length() && strScript[i++] == '\t') {
			++numTabs;
			character -= tabWidth;
		} else {
			character--;
		}

	}

	return numTabs;
}

void PuppetLint::ReadError(HANDLE hProcess, HANDLE hPipe, string& strError)
{
	strError = "";
	while (true) {
		// read data from pipe
		char buffer[512];
		DWORD dwRead;
		if (!ReadFromPipe(hProcess, hPipe, buffer, _countof(buffer), dwRead))
			return;

		strError += string(buffer, dwRead);
	}
}

bool PuppetLint::ReadFromPipe(HANDLE hProcess, HANDLE hPipe, char *buffer, 
	DWORD dwBufferSize, DWORD& dwRead)
{
	// first check if there is data in pipe
	DWORD bytesAvailable;
	while (true) {
		PeekNamedPipe(hPipe, NULL, 0, NULL, &bytesAvailable, NULL);
		if (bytesAvailable > 0)
			break;

		// no data, check if process is alive
		DWORD exitCode = 0;
		GetExitCodeProcess(hProcess, &exitCode);
		if (exitCode != STILL_ACTIVE) {
			return false;
		}

		Sleep(100);
	}

	// read data from pipe
	BOOL bSuccess = ReadFile(hPipe, buffer, min(dwBufferSize, bytesAvailable), 
		&dwRead, NULL);
	if (!bSuccess || dwRead == 0)
		return false;

	return true;
}