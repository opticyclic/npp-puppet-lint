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
#include "Settings.h"
#include "DownloadJSLint.h"
#include "resource.h"
#include <v8.h>

using namespace v8;

extern HANDLE g_hDllModule;

////////////////////////////////////////////////////////////////////////////////

bool JSLintReportItem::IsReasonUndefVar() const
{
    return !GetUndefVar().empty();
}

tstring JSLintReportItem::GetUndefVar() const
{
    tstring var;

    if (m_type == LINT_TYPE_ERROR) {
        tstring errMsg = Settings::GetInstance().GetJSLintScriptSource() == Settings::JSLINT_SCRIPT_SOURCE_DOWNLOADED &&
            Settings::GetInstance().GetSpecUndefVarErrMsg() ? 
            Settings::GetInstance().GetUndefVarErrMsg() : DEFAULT_UNDEF_VAR_ERR_MSG;

        tstring::size_type i = errMsg.find(TEXT("%s"));
        if (i != tstring::npos) {
            int nAfter = errMsg.size() - (i + 2);
            if (m_strReason.substr(0, i) == errMsg.substr(0, i) &&
                m_strReason.substr(m_strReason.size() - nAfter) == errMsg.substr(i + 2)) 
            {
                var = m_strReason.substr(i, m_strReason.size() - nAfter - i);
            }
        }
    }

    return var;
}

////////////////////////////////////////////////////////////////////////////////

/*
void fatalErrorHandler(const char* location, const char* message)
{
}

void messageListener(Handle<Message> message, Handle<Value> data)
{
    Local<String> msg = message->Get();
    String::AsciiValue str(msg);
    const char* sz = *str;
    const char* a = sz;
    int line = message->GetLineNumber();
    int sc = message->GetStartColumn();
    int ec = message->GetEndColumn();
}
*/

void JSLint::CheckScript(const string& strOptions, const string& strScript, 
	int nppTabWidth, int jsLintTabWidth, list<JSLintReportItem>& items)
{
    //V8::SetFatalErrorHandler(fatalErrorHandler);
    //V8::AddMessageListener(messageListener);

    HandleScope handle_scope;
    Persistent<Context> context = Context::New();
    Context::Scope context_scope(context);

    Handle<Script> script;

    string strJSLintScript;
    if (Settings::GetInstance().GetJSLintScriptSource() == Settings::JSLINT_SCRIPT_SOURCE_BUILTIN) {
        strJSLintScript = LoadCustomDataResource((HMODULE)g_hDllModule, MAKEINTRESOURCE(IDR_JSLINT), TEXT("JS"));
    } else {
        strJSLintScript = DownloadJSLint::GetInstance().GetVersion(
            Settings::GetInstance().GetJSLintScriptVersion()).GetContent();
    }
    if (strJSLintScript.empty()) {
        throw JSLintException("Invalid JSLint script!");
    }
    script = Script::Compile(String::New(strJSLintScript.c_str()));
    if (script.IsEmpty()) {
        throw JSLintException("Invalid JSLint script!");
    }
    script->Run();

    // init script variable
    context->Global()->Set(String::New("script"), String::New(strScript.c_str()));

    // init options variable
    script = Script::Compile(String::New(("options = " + strOptions).c_str()));
    if (script.IsEmpty()) {
        throw JSLintException("Invalid JSLint options (probably error in additional options)!");
    }
    script->Run();

    // call JSLINT
    script = Script::Compile(String::New("JSLINT(script, options);"));
    if (script.IsEmpty()) {
        throw JSLintUnexpectedException();
    }
    script->Run();

    // get JSLINT data
    script = Script::Compile(String::New("JSLINT.data();"));
    if (script.IsEmpty()) {
        throw JSLintUnexpectedException();
    }
    Handle<Object> data = script->Run()->ToObject();

    // read errors
    Handle<Object> errors = data->Get(String::New("errors"))->ToObject();
    if (!errors.IsEmpty()) {
        int32_t length = errors->Get(String::New("length"))->Int32Value();
        for (int32_t i = 0; i < length; ++i) {
            Handle<Value> eVal = errors->Get(Int32::New(i));
            if (eVal->IsObject()) {
                Handle<Object> e = eVal->ToObject();

                int line = e->Get(String::New("line"))->Int32Value();
                int character = e->Get(String::New("character"))->Int32Value();
                String::Utf8Value reason(e->Get(String::New("reason")));
                String::Utf8Value evidence(e->Get(String::New("evidence")));

                // adjust character position if there is a difference 
                // in tab width between Notepad++ and JSLint
                if (nppTabWidth != jsLintTabWidth) {
	                character += GetNumTabs(strScript, line, character, jsLintTabWidth) * (nppTabWidth - jsLintTabWidth);
                }

                items.push_back(JSLintReportItem(JSLintReportItem::LINT_TYPE_ERROR,
                    line - 1, character - 1, 
                    TextConversion::UTF8_To_T(*reason), 
	                TextConversion::UTF8_To_T(*evidence)));
            }
        }
    }

    // read unused
    Handle<Object> unused = data->Get(String::New("unused"))->ToObject();
    if (!unused.IsEmpty()) {
        int32_t length = unused->Get(String::New("length"))->Int32Value();
        for (int32_t i = 0; i < length; ++i) {
            Handle<Value> eVal = unused->Get(Int32::New(i));
            if (eVal->IsObject()) {
                Handle<Object> e = eVal->ToObject();

                int line = e->Get(String::New("line"))->Int32Value();
                String::Utf8Value name(e->Get(String::New("name")));
                String::Utf8Value function(e->Get(String::New("function")));

                tstring reason = TEXT("'") + TextConversion::UTF8_To_T(*name) + 
                    TEXT("' in '") + TextConversion::UTF8_To_T(*function) + TEXT("'");

                items.push_back(JSLintReportItem(JSLintReportItem::LINT_TYPE_UNUSED,
                    line - 1, 0, 
                    TextConversion::UTF8_To_T(*name),
	                TextConversion::UTF8_To_T(*function)));
            }
        }
    }

    context.Dispose();
}

string JSLint::LoadCustomDataResource(HMODULE hModule, LPCTSTR lpName, LPCTSTR lpType)
{
	HRSRC hRes = FindResource(hModule, lpName, lpType);
	if (hRes == NULL) {
		throw JSLintResourceException();
	}

	DWORD dwSize = SizeofResource(hModule, hRes);
	if (dwSize == 0) {
		throw JSLintResourceException();
	}

	HGLOBAL hResLoad = LoadResource(hModule, hRes);
	if (hResLoad == NULL) {
		throw JSLintResourceException();
	}

	LPVOID pData = LockResource(hResLoad);
	if (pData == NULL) {
		throw JSLintResourceException();
	}

    return string((const char*)pData, dwSize);
}

int JSLint::GetNumTabs(const string& strScript, int line, int character, int tabWidth)
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
