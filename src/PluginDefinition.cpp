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
#include "AboutDlg.h"
#include "JSLint.h"
#include "menuCmdID.h"
#include "OptionsDlg.h"
#include "OutputDlg.h"
#include "PluginDefinition.h"

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData g_nppData;
HANDLE g_hDllModule;
OutputDlg g_outputDlg;

//
// Plugin command functions forward declarations
//
void jsLintCurrentFile();
void jsLintAllFiles();
void gotoNextLint();
void gotoPrevLint();
void showLints();
void options();
void about();

//
// Private helper functions forward declarations
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool check0nInit = false);
void createOutputWindow();
void doJSLint();
INT_PTR pluginDialogBox(UINT idDlg, DLGPROC lpDlgProc);

//
// Interface functions
//
void pluginInit(HANDLE hModule)
{
	// Initialize your plugin data here
	// It will be called while plugin loading   
	g_hDllModule = hModule;
	g_outputDlg.init((HINSTANCE)g_hDllModule, NULL);
	g_jsLintOptions.ReadOptions();
}

void pluginCleanUp()
{
	g_jsLintOptions.SaveOptions();
}

void commandMenuInit()
{
	ShortcutKey *shKey;
	
	shKey = new ShortcutKey; // Ctrl+Shift+L
	shKey->_isAlt = false;
	shKey->_isCtrl = true;
	shKey->_isShift = true;
	shKey->_key = 0x4c;
	setCommand(0, TEXT("JSLint Current File"), jsLintCurrentFile, shKey, false);

	shKey = new ShortcutKey; // Ctrl+L
	shKey->_isAlt = false;
	shKey->_isCtrl = true;
	shKey->_isShift = false;
	shKey->_key = 0x4c;
	setCommand(1, TEXT("JSLint All Files"), jsLintAllFiles, shKey, false);

	setCommand(2, TEXT("---"), NULL, NULL, false);
	
	shKey = new ShortcutKey; // Ctrl+Shift+N
	shKey->_isAlt = false;
	shKey->_isCtrl = true;
	shKey->_isShift = true;
	shKey->_key = 0x4e;
	setCommand(3, TEXT("Go To Next Lint"), gotoNextLint, shKey, false);
	
	shKey = new ShortcutKey; // Ctrl+Shift+B
	shKey->_isAlt = false;
	shKey->_isCtrl = true;
	shKey->_isShift = true;
	shKey->_key = 0x42;
	setCommand(4, TEXT("Go To Previous Lint"), gotoPrevLint, shKey, false);
	
	setCommand(5, TEXT("Show Lints"), showLints, NULL, false);

	setCommand(6, TEXT("---"), NULL, NULL, false);
	setCommand(7, TEXT("Options"), options, NULL, false);
	setCommand(8, TEXT("About"), about, NULL, false);
}

void commandMenuCleanUp()
{
	delete funcItem[0]._pShKey;
	delete funcItem[1]._pShKey;
	delete funcItem[3]._pShKey;
	delete funcItem[4]._pShKey;
}

//
// Plugin command functions
//

void jsLintCurrentFile()
{
	createOutputWindow();

	int type;
	::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&type);
	if (type != L_JS) {
		::MessageBox(
			g_nppData._nppHandle, 
			TEXT("Current file is not JavaScript file!\nJSLint works only for the JavaScript files."),
			TEXT("JSLint"),
			MB_OK | MB_ICONINFORMATION
		);
		return;
	}

	// set hourglass cursor
	SetCursor(LoadCursor(NULL, IDC_WAIT));

	g_outputDlg.ClearAllLints();
	
	doJSLint();
	
	showLints();
	gotoNextLint();

	// restore normal cursor:
	POINT pt;
	GetCursorPos(&pt);
	SetCursorPos(pt.x, pt.y);
}

void jsLintAllFiles()
{
	// set hourglass cursor
	SetCursor(LoadCursor(NULL, IDC_WAIT));

	int numJSFiles = 0;

	int numOpenFiles = ::SendMessage(g_nppData._nppHandle, NPPM_GETNBOPENFILES, 0, PRIMARY_VIEW);
	if (numOpenFiles > 0) {
		createOutputWindow();
		g_outputDlg.ClearAllLints();

		for (int i = 0; i < numOpenFiles; ++i) {
			::SendMessage(g_nppData._nppHandle, NPPM_ACTIVATEDOC, 0, (LPARAM)i);

			int type;
			::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&type);
			if (type == L_JS) {
				++numJSFiles;
				doJSLint();
			}
		}

		showLints();
		gotoNextLint();
	}

	// restore normal cursor:
	POINT pt;
	GetCursorPos(&pt);
	SetCursorPos(pt.x, pt.y);

	if (numJSFiles == 0) {
		::MessageBox(
			g_nppData._nppHandle, 
			TEXT("There is no JavaScript file opened in Notepad++!"),
			TEXT("JSLint"),
			MB_OK | MB_ICONINFORMATION
		);
		return;
	}
}

void gotoNextLint()
{
	g_outputDlg.SelectNextLint();
}

void gotoPrevLint()
{
	g_outputDlg.SelectPrevLint();
}

void showLints()
{
	createOutputWindow();
	g_outputDlg.display();
}

void options()
{
	pluginDialogBox(IDD_OPTIONS, OptionsDlgProc);
}

void about()
{
	pluginDialogBox(IDD_ABOUT, AboutDlgProc);
}

//
// Helper functions
//

bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

HWND getCurrentScintillaWindow()
{
    int which = -1;
    ::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return NULL;
    else if (which == 0) 
		return g_nppData._scintillaMainHandle;
	else
		return g_nppData._scintillaSecondHandle;
}

void createOutputWindow()
{
	if (!g_outputDlg.isCreated())
	{
		g_outputDlg.setParent(g_nppData._nppHandle);
		
		tTbData	data = {0};
		
		g_outputDlg.create(&data);

		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_BOTTOM | DWS_ICONTAB;
		data.pszModuleName = g_outputDlg.getPluginFileName();
		data.hIconTab = g_outputDlg.GetTabIcon();
		data.dlgID = -1; /* N_EMPTY; */
		::SendMessage(g_nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
	}
}

void doJSLint()
{
	// get current file path and document index
	TCHAR filePath[MAX_PATH];
	::SendMessage(g_nppData._nppHandle, NPPM_GETFULLCURRENTPATH, 0, (LPARAM)filePath);

    // get the current scintilla window
    HWND hWndScintilla = getCurrentScintillaWindow();
	if (hWndScintilla == NULL) {
		return;
	}

	// get all the text from the scintilla window
	TextRange tr;

	tr.chrg.cpMin = 0;
    tr.chrg.cpMax = -1;

	int length = (int) ::SendMessage(hWndScintilla, SCI_GETLENGTH, 0, 0);
	tr.lpstrText = new char[length + 1];
	tr.lpstrText[0] = 0;

	::SendMessage(hWndScintilla, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
	string strScript = tr.lpstrText;
	delete tr.lpstrText;

	// get code page of the text
	int nSciCodePage = (int) ::SendMessage(hWndScintilla, SCI_GETCODEPAGE, 0, 0);
	if (nSciCodePage != SC_CP_UTF8) {
		strScript = TextConversion::A_To_UTF8(strScript); // convert to UTF-8
	}

	try {
		JSLint jsLint;

		string strOptions = TextConversion::T_To_UTF8(
			g_jsLintOptions.GetOptionsJSONString());
		list<JSLintReportItem> lints;

		int nppTabWidth = (int) ::SendMessage(hWndScintilla, SCI_GETTABWIDTH, 0, 0);
		int jsLintTabWidth = g_jsLintOptions.GetTabWidth();

		jsLint.CheckScript(strOptions, strScript, nppTabWidth, jsLintTabWidth, lints);

		g_outputDlg.AddLints(filePath, lints);

		DoEvents();
	} catch (exception&) {
		// TODO better exception handling and much more descriptive error message
		::MessageBox(
			g_nppData._nppHandle, 
			TEXT("Failed!"),
			TEXT("JSLint"),
			MB_OK | MB_ICONERROR
		);
	}
}

INT_PTR pluginDialogBox(UINT idDlg, DLGPROC lpDlgProc)
{
	HWND hWndFocus = ::GetFocus();
	INT_PTR nRet = ::DialogBox((HINSTANCE)g_hDllModule, MAKEINTRESOURCE(idDlg),
		g_nppData._nppHandle, lpDlgProc);
	::SetFocus(hWndFocus);
	return nRet;
}
