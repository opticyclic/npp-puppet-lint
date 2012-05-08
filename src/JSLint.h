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

#include "Util.h"

////////////////////////////////////////////////////////////////////////////////

#define JSLINT_DEFAULT_UNDEF_VAR_ERR_MSG TEXT("'%s' was used before it was defined.")
#define JSHINT_DEFAULT_UNDEF_VAR_ERR_MSG TEXT("'%s' is not defined.")

////////////////////////////////////////////////////////////////////////////////

class JSLintReportItem
{
public:
    enum Type {
        LINT_TYPE_ERROR,
        LINT_TYPE_UNUSED
    };

	JSLintReportItem(Type type, int line, int character, const tstring& strReason, const tstring& strEvidence)
		: m_type(type)
        , m_line(line)
		, m_character(character)
		, m_strReason(strReason)
		, m_strEvidence(strEvidence)
	{
	}

	Type GetType() const { return m_type; }
	int GetLine() const { return m_line; }
	int GetCharacter() const { return m_character; }
	tstring GetReason() const { return m_strReason; }
	tstring GetEvidence() const { return m_strEvidence; }

	bool IsReasonUndefVar() const;
	tstring GetUndefVar() const;

private:
    Type m_type;
	int m_line;
	int m_character;
	tstring m_strReason;
	tstring m_strEvidence;
};

////////////////////////////////////////////////////////////////////////////////

class JSLint
{
public:
	void CheckScript(const string& strOptions, const string& strScript, 
		int nppTabWidth, int jsLintTabWidth, list<JSLintReportItem>& items);

private:
	string LoadCustomDataResource(HMODULE hModule, LPCTSTR lpName, LPCTSTR lpType);
	
	int GetNumTabs(const string& strScript, int line, int character, int tabWidth);
};

////////////////////////////////////////////////////////////////////////////////

class JSLintException : public exception 
{
public:
    JSLintException(const char* what) : exception(what) {}
};

class JSLintResourceException : public JSLintException
{
public:
    JSLintResourceException() : JSLintException("Failed to load JSLINT script from resource!") {}
};

class JSLintUnexpectedException : public JSLintException 
{
public:
    JSLintUnexpectedException() : JSLintException("Unexpected error while running JSLINT script!") {}
};
