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
#include "JSLint.h"
#include "resource.h"
#include "base64.h"

extern HANDLE g_hDllModule;

////////////////////////////////////////////////////////////////////////////////

bool JSLintReportItem::IsReasonVarIsNotDefined() const
{
	// check if reason matches: /'.+' is not defined/
	const TCHAR *pattern = _T("' is not defined.");
	const size_t len = _tcslen(pattern);
	return m_strReason.size() > len+1 
		&& m_strReason[0] == _T('\'') 
		&& m_strReason.substr(m_strReason.size() - len) == pattern;
}

tstring JSLintReportItem::GetUndefinedVar() const
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
void JSLint::CheckScript(const string& strOptions, const string& strScript, 
	int nppTabWidth, int jsLintTabWidth, list<JSLintReportItem>& items)
{
	if (!m_jsLintScriptFileName)
		CreateJSLintFile();

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

	// create cscript process
	tstring strCmdLine = TEXT("cscript.exe /Nologo /E:JScript \"") 
		+ m_jsLintScriptFileName.GetFileName() + TEXT("\"");
	BOOL bSuccess = CreateProcess(NULL, (LPTSTR)strCmdLine.c_str(),
		NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &siStartInfo, &piProcInfo);
	if (!bSuccess) {
		throw JSLintUnexpectedException();
	}
	Win32Handle hThread = piProcInfo.hThread;
	Win32Handle hProcess = piProcInfo.hProcess;

	// send input script to the standard input
	WriteString(hChildStdInputWrite, strOptions + "\n" + strScript);
	hChildStdInputWrite = NULL; // close std input pipe, so that JSLint can start

	// read result from the standard output
	ParseOutput(hProcess, hChildStdOutputRead, strScript, 
		nppTabWidth, jsLintTabWidth, items);

	// read data from the standard error stream
	string strError;
	ReadError(hProcess, hChildStdErrorRead, strError);

	// get exit code
	DWORD exitCode = 0;
	GetExitCodeProcess(hProcess, &exitCode);
}

void JSLint::LoadCustomDataResource(HMODULE hModule,
	LPCTSTR lpName, LPCTSTR lpType, LPVOID* ppData, LPDWORD pdwSize)
{
	*pdwSize = 0;
	*ppData = NULL; 

	HRSRC hRes = FindResource(hModule, lpName, lpType);
	if (hRes == NULL) {
		throw JSLintResourceException();
	}

	*pdwSize = SizeofResource(hModule, hRes);
	if (*pdwSize == 0) {
		throw JSLintResourceException();
	}

	HGLOBAL hResLoad = LoadResource(hModule, hRes);
	if (hResLoad == NULL) {
		throw JSLintResourceException();
	}

	*ppData = LockResource(hResLoad);
	if (*ppData == NULL) {
		throw JSLintResourceException();
	}
}

void JSLint::CreateJSLintFile()
{
	// JSLint JavaScript source file is created by augmenting 
	// jslint_output.js upon original jslint.js (WSH edition).

	// create output file
	m_jsLintScriptFileName.Create();
	Win32Handle hJSLintFile = CreateFile(m_jsLintScriptFileName, GENERIC_WRITE, 0, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
	if ((HANDLE)hJSLintFile == INVALID_HANDLE_VALUE) {
		throw IOException();
	}

	LPVOID pData;
	DWORD dwSize;
	DWORD dwWritten;

	// load jslint.js from resource and write it to the output file
	LoadCustomDataResource((HMODULE)g_hDllModule, MAKEINTRESOURCE(IDR_JSLINT), TEXT("JS"), &pData, &dwSize);
	if (WriteFile(hJSLintFile, pData, dwSize, &dwWritten, NULL) != TRUE || dwWritten != dwSize) {
		throw IOException();
	}

	// load jslint_ouput.js from resource and write it to the output file
	LoadCustomDataResource((HMODULE)g_hDllModule, MAKEINTRESOURCE(IDR_JSLINT_OUTPUT), TEXT("JS"), &pData, &dwSize);
	if (WriteFile(hJSLintFile, pData, dwSize, &dwWritten, NULL) != TRUE || dwWritten != dwSize) {
		throw IOException();
	}
}

void JSLint::WriteString(HANDLE hFile, const string& str)
{
	string strEncoded = base64_encode((unsigned char const*)str.c_str(), str.length());
	DWORD dwSize = (DWORD)strlen(strEncoded.c_str()) * sizeof(char);
	DWORD dwWritten;
	if (WriteFile(hFile, (LPVOID)strEncoded.c_str(), dwSize, &dwWritten, NULL) == FALSE || dwWritten != dwSize) {
		throw IOException();
	}
}

void JSLint::ParseOutput(HANDLE hProcess, HANDLE hPipe, const string& strScript,
	int nppTabWidth, int jsLintTabWidth, list<JSLintReportItem>& items)
{
	// read JSLint output
	string strOutput;
	while (true) {
		// read data from pipe
		char buffer[512];
		DWORD dwRead;
		if (!ReadFromPipe(hProcess, hPipe, buffer, _countof(buffer), dwRead))
			return;

		strOutput += string(buffer, dwRead);

		// parse JSLint output, each lint is delimited with empty line (see jslint_output.js)
		while (true) {
			size_t i = strOutput.find("\r\n\r\n");
			if (i == string::npos) {
				break;
			}

			string strLint = strOutput.substr(0, i);
			strOutput = strOutput.substr(i + 4);

			// read line
			i = strLint.find("\r\n");
			int line = atoi(base64_decode(strLint.substr(0, i)).c_str());
			strLint = strLint.substr(i+2);

			// read character
			i = strLint.find("\r\n");
			int character = atoi(base64_decode(strLint.substr(0, i)).c_str());
			
			// adjust character position if there is a difference 
			// in tab width between Notepad++ and JSLint
			if (nppTabWidth != jsLintTabWidth) {
				character += GetNumTabs(strScript, line, character, jsLintTabWidth) * (nppTabWidth - jsLintTabWidth);
			}
			
			strLint = strLint.substr(i+2);

			// read reason
			i = strLint.find("\r\n");
			string strReason = base64_decode(strLint.substr(0, i));
			strLint = strLint.substr(i+2);
			
			// read evidence
			string strEvidence = base64_decode(strLint);

			items.push_back(JSLintReportItem(line - 1, character - 1, 
				TextConversion::UTF8_To_T(strReason), 
				TextConversion::UTF8_To_T(strEvidence)));
		}
	}
}

int JSLint::GetNumTabs(const string& strScript, int line, int character, int tabWidth)
{
	int numTabs = 0;

	int i = 0;

	while (line-- > 0) {
		i = strScript.find('\n', i) + 1;
	}

	while (character > 0) {
		if (strScript[i++] == '\t') {
			++numTabs;
			character -= tabWidth;
		} else {
			character--;
		}

	}

	return numTabs;
}

void JSLint::ReadError(HANDLE hProcess, HANDLE hPipe, string& strError)
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

bool JSLint::ReadFromPipe(HANDLE hProcess, HANDLE hPipe, char *buffer, 
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