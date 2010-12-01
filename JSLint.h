#pragma once

#include "Util.h"

////////////////////////////////////////////////////////////////////////////////

class JSLintReportItem
{
public:
	JSLintReportItem(int line, int character, const string& strReason, const string& strEvidence)
		: m_line(line)
		, m_character(character)
		, m_strReason(strReason)
		, m_strEvidence(strEvidence)
	{
	}

	int GetLine() const { return m_line; }
	int GetCharacter() const { return m_character; }
	string GetReason() const { return m_strReason; }
	string GetEvidence() const { return m_strEvidence; }

private:
	int m_line;
	int m_character;
	string m_strReason;
	string m_strEvidence;
};

////////////////////////////////////////////////////////////////////////////////

class JSLint
{
public:
	void CheckScript(const string& strOptions, const string& strScript, list<JSLintReportItem>& items);

private:
	TempFile m_jsLintScriptFileName;

	void CreateJSLintFile();
	
	void LoadCustomDataResource(HMODULE hModule, 
		LPCTSTR lpName, LPCTSTR lpType, LPVOID* ppData, LPDWORD pdwSize);
	
	static void WriteString(HANDLE hFile, const string& strScript);
	
	void ParseOutput(HANDLE hProcess, HANDLE hPipe, 
		list<JSLintReportItem>& items);
	
	void ReadError(HANDLE hProcess, HANDLE hPipe, string& strError);
	
	bool ReadFromPipe(HANDLE hProcess, HANDLE hPipe, char *buffer, 
		DWORD dwBufferSize, DWORD& dwRead);
};

////////////////////////////////////////////////////////////////////////////////

class JSLintException : public exception 
{
};


class JSLintResourceException : public JSLintException 
{
};

class JSLintUnexpectedException : public JSLintException 
{
};
