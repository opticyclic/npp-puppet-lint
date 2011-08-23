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


#ifndef PLUGINDEFINITION_H
#define PLUGINDEFINITION_H

#include "PluginInterface.h"

//
// All definitions of plugin interface
//
const TCHAR NPP_PLUGIN_NAME[] = TEXT("JSLint");
const int NB_FUNC = 11;

extern NppData g_nppData;
extern HANDLE g_hDllModule;
extern FuncItem g_funcItem[NB_FUNC];

#define FUNC_INDEX_JSLINT_CURRENT_FILE 0
#define FUNC_INDEX_JSLINT_ALL_FILES    1
#define FUNC_INDEX_GOTO_PREV_LINT      3
#define FUNC_INDEX_GOTO_NEXT_LINT      4
#define FUNC_INDEX_SHOW_LINTS          5
#define FUNC_INDEX_JSLINT_OPTIONS      7
#define FUNC_INDEX_SETTINGS            8
#define FUNC_INDEX_ABOUT               10

//
// Initialization of your plugin data
// It will be called while plugin loading
//
void pluginInit(HANDLE hModule);

//
// Load plugin configuration 
//
void loadConfig();

//
// Cleaning of your plugin
// It will be called while plugin unloading
//
void pluginCleanUp();

//
// Initialization of your plugin commands
//
void commandMenuInit();

//
// Clean up your plugin commands allocation (if any)
//
void commandMenuCleanUp();

//
// Helper functions
//
HWND GetCurrentScintillaWindow();
tstring GetConfigFileName();

//
// Plugin command functions
//
void jsLintCurrentFile();
void jsLintAllFiles();
void gotoNextLint();
void gotoPrevLint();
void showLints();
void showJSLintOptionsDlg();
void showSettingsDlg();
void showAboutDlg();

INT_PTR pluginDialogBox(UINT idDlg, DLGPROC lpDlgProc);

#endif //PLUGINDEFINITION_H
