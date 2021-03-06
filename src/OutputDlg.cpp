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
#include "OutputDlg.h"
#include "PluginDefinition.h"

////////////////////////////////////////////////////////////////////////////////

#define ID_COPY_LINTS     1500
#define ID_SHOW_LINT      1501
#define ID_ADD_PREDEFINED 1502
#define ID_SELECT_ALL     1503

#define	IDM_TOOLBAR 2000

#define	IDM_TB_JSLINT_CURRENT_FILE (IDM_TOOLBAR + 1)
#define	IDM_TB_JSLINT_ALL_FILES    (IDM_TOOLBAR + 2)
#define	IDM_TB_PREV_LINT           (IDM_TOOLBAR + 3)
#define	IDM_TB_NEXT_LINT           (IDM_TOOLBAR + 4)
#define	IDM_TB_LINT_OPTIONS        (IDM_TOOLBAR + 5)

////////////////////////////////////////////////////////////////////////////////

OutputDlg::OutputDlg()
	: DockingDlgInterface(IDD_OUTPUT)
	, m_hWndListView(NULL)
{
}

OutputDlg::~OutputDlg()
{
	DestroyIcon(m_hTabIcon);
}

BOOL CALLBACK OutputDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_INITDIALOG:
			InitializeToolbar();
			InitializeListViewCtrl();
			break;

		case WM_COMMAND: {
				if ((HWND)lParam == m_toolbar.getHSelf()) {
					OnToolbarCmd(LOWORD(wParam));
					return TRUE;
				} else {
					if (LOWORD(wParam) == ID_COPY_LINTS) {
						CopyToClipboard();
						return TRUE;
					} else if (LOWORD(wParam) == ID_SHOW_LINT) {
						int i = ListView_GetNextItem(m_hWndListView, -1, LVIS_FOCUSED | LVIS_SELECTED);
						if (i != -1) {
							ShowLint(i);
						}
						return TRUE;
					} else if (LOWORD(wParam) == ID_ADD_PREDEFINED) {
						int iFocused = ListView_GetNextItem(m_hWndListView, -1, LVIS_FOCUSED | LVIS_SELECTED);
						if (iFocused != -1) {
							const FileLint& fileLint = m_fileLints[iFocused];
							tstring var = fileLint.lint.GetUndefinedVar();
							if (!var.empty()) {
								//TODO: This used to set an option but we removed the options.......
							}
						}
						return TRUE;
					} else if (LOWORD(wParam) == ID_SELECT_ALL) {
						ListView_SetItemState(m_hWndListView, -1, LVIS_SELECTED, LVIS_SELECTED);
						return TRUE;
					}
				}
			}
			break;

		case WM_NOTIFY: {
				LPNMHDR pNMHDR = (LPNMHDR) lParam;
				if (pNMHDR->idFrom == IDC_OUTPUT && pNMHDR->code == LVN_KEYDOWN) {
					LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
					if (pnkd->wVKey == 'A' && (::GetKeyState(VK_CONTROL) >> 15 & 1)) {
						ListView_SetItemState(m_hWndListView, -1, LVIS_SELECTED, LVIS_SELECTED);
					} else if (pnkd->wVKey == 'C' && (::GetKeyState(VK_CONTROL) >> 15 & 1)) {
						CopyToClipboard();
					}
				} else if (pNMHDR->idFrom == IDC_OUTPUT && pNMHDR->code == NM_DBLCLK) {
					LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
					if (lpnmitem->iItem != -1) {
						ShowLint(lpnmitem->iItem);
					}
				} else if (pNMHDR->hwndFrom == m_toolbar.getHSelf() && pNMHDR->code == TBN_DROPDOWN) {
					OnToolbarDropDown((LPNMTOOLBAR) lParam);
					return TBDDRET_NODEFAULT;
				} else if (pNMHDR->hwndFrom == m_rebar.getHSelf() && pNMHDR->code == RBN_CHEVRONPUSHED) {
					NMREBARCHEVRON* lpnm = (NMREBARCHEVRON*) pNMHDR;
					if (lpnm->wID == REBAR_BAR_TOOLBAR) {
						POINT pt;
						pt.x = lpnm->rc.left;
						pt.y = lpnm->rc.bottom;
						ClientToScreen(pNMHDR->hwndFrom, &pt);
						OnToolbarCmd(m_toolbar.doPopop(pt));
						return TRUE;
					}
				} else if (pNMHDR->code == TTN_GETDISPINFO) {
					LPTOOLTIPTEXT lpttt; 

					lpttt = (LPTOOLTIPTEXT)pNMHDR; 
					lpttt->hinst = _hInst;

					// Specify the resource identifier of the descriptive 
					// text for the given button.
					int resId = int(lpttt->hdr.idFrom);

					TCHAR	tip[MAX_PATH];
					GetNameStrFromCmd(resId, tip, sizeof(tip));
					lpttt->lpszText = tip;
					return TRUE;
				}
				DockingDlgInterface::run_dlgProc(message, wParam, lParam);
				return FALSE;
			}
			break;

		case WM_CONTEXTMENU: {
				// build context menu
				HMENU menu = ::CreatePopupMenu();

				int numSelected = ListView_GetSelectedCount(m_hWndListView);

				int iFocused = -1;
				if (numSelected > 0) {
					iFocused = ListView_GetNextItem(m_hWndListView, -1, LVIS_FOCUSED | LVIS_SELECTED);
				}

				bool reasonIsVarIsNotDefined = false;
				if (iFocused != -1) {
					const FileLint& fileLint = m_fileLints[iFocused];
					reasonIsVarIsNotDefined = fileLint.lint.IsReasonVarIsNotDefined();
				}

				if (iFocused != -1) {
					AppendMenu(menu, MF_ENABLED, ID_SHOW_LINT, _T("Show"));
				}

				if (reasonIsVarIsNotDefined) {
					AppendMenu(menu, MF_ENABLED, ID_ADD_PREDEFINED, _T("Add to the Predefined List"));
				}

				if (GetMenuItemCount(menu) > 0)
					AppendMenu(menu, MF_SEPARATOR, 0, NULL);

				if (numSelected > 0) {
					AppendMenu(menu, MF_ENABLED, ID_COPY_LINTS, _T("Copy"));
				}

				AppendMenu(menu, MF_ENABLED, ID_SELECT_ALL, _T("Select All"));

				// determine context menu position
				POINT point;
				point.x = LOWORD(lParam);
				point.y = HIWORD(lParam);
				if (point.x == 65535 || point.y == 65535) {
					point.x = 0;
					point.y = 0;
					ClientToScreen(m_hWndListView, &point);
				}

				// show context menu
				TrackPopupMenu(menu, 0, point.x, point.y, 0, _hSelf, NULL);
			}
			break;

		case WM_SIZE:
		case WM_MOVE:
			Resize();
			break;

		case WM_PAINT:
			::RedrawWindow(m_toolbar.getHSelf(), NULL, NULL, TRUE);
			break;

		default:
			return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
	}

	return FALSE;
}

void OutputDlg::OnToolbarCmd(UINT message)
{
	switch (message) {
		case IDM_TB_JSLINT_CURRENT_FILE:
			puppetLintCurrentFile();
			break;
		case IDM_TB_JSLINT_ALL_FILES:
			puppetLintAllFiles();
			break;
		case IDM_TB_NEXT_LINT:
			gotoNextLint();
			break;
		case IDM_TB_PREV_LINT:
			gotoPrevLint();
			break;
		case IDM_TB_LINT_OPTIONS:
			options();
			break;
	}
}

void OutputDlg::OnToolbarDropDown(LPNMTOOLBAR lpnmtb)
{
}

void OutputDlg::InitializeToolbar()
{
	static ToolBarButtonUnit toolBarIcons[] = {
		{IDM_TB_JSLINT_CURRENT_FILE, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDB_TB_JSLINT_CURRENT_FILE, 0},
		{IDM_TB_JSLINT_ALL_FILES,    IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDB_TB_JSLINT_ALL_FILES, 0},	 
		{0,                          IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, 0},
		{IDM_TB_PREV_LINT,           IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDB_TB_PREV_LINT, 0},
		{IDM_TB_NEXT_LINT,           IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDB_TB_NEXT_LINT, 0},
		{0,                          IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, 0},
		{IDM_TB_LINT_OPTIONS,        IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDB_TB_LINT_OPTIONS, 0},
	};

	m_toolbar.init(_hInst, _hSelf, TB_STANDARD, toolBarIcons, sizeof(toolBarIcons) / sizeof(ToolBarButtonUnit));
	m_rebar.init(_hInst, _hSelf);
	m_toolbar.addToRebar(&m_rebar);
	m_rebar.setIDVisible(REBAR_BAR_TOOLBAR, true);
}

void OutputDlg::InitializeListViewCtrl()
{
	m_hWndListView = ::GetDlgItem(_hSelf, IDC_OUTPUT);

	ListView_SetExtendedListViewStyle(m_hWndListView, LVS_EX_FULLROWSELECT);

	LVCOLUMN lvc;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	lvc.iSubItem = COL_NUM;
	lvc.pszText = TEXT("");
	lvc.cx = 28;
	lvc.fmt = LVCFMT_RIGHT;
	ListView_InsertColumn(m_hWndListView, COL_NUM, &lvc);

	lvc.iSubItem = COL_REASON;
	lvc.pszText = TEXT("Reason");
	lvc.cx = 500;
	lvc.fmt = LVCFMT_LEFT;
	ListView_InsertColumn(m_hWndListView, COL_REASON, &lvc);

	lvc.iSubItem = COL_FILE;
	lvc.pszText = TEXT("File");
	lvc.cx = 200;
	lvc.fmt = LVCFMT_LEFT;
	ListView_InsertColumn(m_hWndListView, COL_FILE, &lvc);

	lvc.iSubItem = COL_LINE;
	lvc.pszText = TEXT("Line");
	lvc.cx = 50;
	lvc.fmt = LVCFMT_RIGHT;
	ListView_InsertColumn(m_hWndListView, COL_LINE, &lvc);

	lvc.iSubItem = COL_COLUMN;
	lvc.pszText = TEXT("Column");
	lvc.cx = 50;
	lvc.fmt = LVCFMT_RIGHT;
	ListView_InsertColumn(m_hWndListView, COL_COLUMN, &lvc);
}

void OutputDlg::Resize()
{
	RECT rc;
	getClientRect(rc);

	m_toolbar.reSizeTo(rc);
	m_rebar.reSizeTo(rc);

	getClientRect(rc);
	rc.top += 25;
	InflateRect(&rc, -1, -1);
	::MoveWindow(m_hWndListView, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

HICON OutputDlg::GetTabIcon()
{
	if (m_hTabIcon == NULL) {
		m_hTabIcon = (HICON) ::LoadImage((HINSTANCE)g_hDllModule,
			MAKEINTRESOURCE(IDI_JSLINT_TAB), IMAGE_ICON, 0, 0,
			LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT );
	}
	return m_hTabIcon;
}

void OutputDlg::GetNameStrFromCmd(UINT resID, LPTSTR tip, UINT count)
{
	// NOTE: On change, keep sure to change order of IDM_EX_... in toolBarIcons also
	static LPTSTR szToolTip[] = {
		_T("PuppetLint Current File"),
		_T("PuppetLint All Files"),
		_T("Go To Previous Lint"),
		_T("Go To Next Lint"),
	};

	_tcscpy(tip, szToolTip[resID - IDM_TB_JSLINT_CURRENT_FILE]);
}

void OutputDlg::ClearAllLints()
{
	m_fileLints.clear();
	ListView_DeleteAllItems(m_hWndListView);
}

void OutputDlg::AddLints(const tstring& strFilePath, const list<PuppetLintReportItem>& lints)
{
	basic_stringstream<TCHAR> stream;

	LVITEM lvI;
	lvI.mask = LVIF_TEXT | LVIF_STATE;

	for (list<PuppetLintReportItem>::const_iterator it = lints.begin(); it != lints.end(); ++it) {
		const PuppetLintReportItem& lint = *it;

		lvI.iSubItem = 0;
		lvI.iItem = ListView_GetItemCount(m_hWndListView);
		lvI.state = 0;
		lvI.stateMask = 0;

		stream.str(TEXT(""));
		stream << lvI.iItem + 1;
		tstring strNum = stream.str();

		lvI.pszText = (LPTSTR)strNum.c_str();
		
		ListView_InsertItem(m_hWndListView, &lvI);

		tstring strReason = lint.GetReason();
		ListView_SetItemText(m_hWndListView, lvI.iItem, COL_REASON, (LPTSTR)strReason.c_str());

		tstring strFile = Path::GetFileName(strFilePath);
		ListView_SetItemText(m_hWndListView, lvI.iItem, COL_FILE, (LPTSTR)strFile.c_str());

		stream.str(TEXT(""));
		stream << lint.GetLine() + 1;
		tstring strLine = stream.str();
		ListView_SetItemText(m_hWndListView, lvI.iItem, COL_LINE, (LPTSTR)strLine.c_str());
		
		stream.str(TEXT(""));
		stream << lint.GetCharacter() + 1;
		tstring strColumn = stream.str();
		ListView_SetItemText(m_hWndListView, lvI.iItem, COL_COLUMN, (LPTSTR)strColumn.c_str());

		m_fileLints.push_back(FileLint(strFilePath, lint));
	}
}

void OutputDlg::SelectNextLint()
{
	if (_hSelf == NULL)
		return;

	int count = ListView_GetItemCount(m_hWndListView);
	if (count == 0)
		return;

	int i = ListView_GetNextItem(m_hWndListView, -1, LVNI_FOCUSED | LVNI_SELECTED);
	if (++i == count)
		i = 0;

	ListView_SetItemState(m_hWndListView, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
	
	ListView_SetItemState(m_hWndListView, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	ListView_EnsureVisible(m_hWndListView, i, FALSE);
	ShowLint(i);
}

void OutputDlg::SelectPrevLint()
{
	if (_hSelf == NULL)
		return;

	int count = ListView_GetItemCount(m_hWndListView);
	if (count == 0)
		return;

	int i = ListView_GetNextItem(m_hWndListView, -1, LVNI_FOCUSED | LVNI_SELECTED);
	if (--i == -1)
		i = count - 1;
	
	ListView_SetItemState(m_hWndListView, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);

	ListView_SetItemState(m_hWndListView, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	ListView_EnsureVisible(m_hWndListView, i, FALSE);
	ShowLint(i);
}

void OutputDlg::ShowLint(int i)
{
	const FileLint& fileLint = m_fileLints[i];
	
	int line = fileLint.lint.GetLine();
	int column = fileLint.lint.GetCharacter();
	
	if (!fileLint.strFilePath.empty() && line >= 0 && column >= 0) {
		LRESULT lRes = ::SendMessage(g_nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)fileLint.strFilePath.c_str());
		if (lRes) {
			HWND hWndScintilla = GetCurrentScintillaWindow();
			if (hWndScintilla != NULL) {
				::SendMessage(hWndScintilla, SCI_GOTOLINE, line, 0);
				// since there is no SCI_GOTOCOLUMN, we move to the right until ...
				while (true) {
					::SendMessage(hWndScintilla, SCI_CHARRIGHT, 0, 0);

					int curPos = (int) ::SendMessage(hWndScintilla, SCI_GETCURRENTPOS, 0, 0);

					int curLine = (int) ::SendMessage(hWndScintilla, SCI_LINEFROMPOSITION, curPos, 0);
					if (curLine > line) {
						// ... current line is greater than desired line or ...
						::SendMessage(hWndScintilla, SCI_CHARLEFT, 0, 0);
						break;
					}

					int curCol = (int) ::SendMessage(hWndScintilla, SCI_GETCOLUMN, curPos, 0);
					if (curCol > column) {
						// ... current column is greater than desired column or ...
						::SendMessage(hWndScintilla, SCI_CHARLEFT, 0, 0);
						break;
					}

					if (curCol == column) {
						// ... we reached desired column.
						break;
					}
				}
			}
		}
	}
}

void OutputDlg::CopyToClipboard()
{
	basic_stringstream<TCHAR> stream;

	bool bFirst = true;
	int i = ListView_GetNextItem(m_hWndListView, -1, LVNI_SELECTED);
	while (i != -1) {
		const FileLint& fileLint = m_fileLints[i];

		if (bFirst) {
			bFirst = false;
		} else {
			stream << _T("\r\n");
		}

		stream << _T("Line ") << fileLint.lint.GetLine() 
			<< _T(", column ") << fileLint.lint.GetCharacter()
			<< _T(": ") << fileLint.lint.GetReason().c_str() 
			 << _T("\r\n");

		i = ListView_GetNextItem(m_hWndListView, i, LVNI_SELECTED);
	}

	tstring str = stream.str();
	if (str.empty())
		return;

	if (OpenClipboard(_hSelf)) {
		if (EmptyClipboard()) {
			size_t size = (str.size() + 1) * sizeof(TCHAR);
			HGLOBAL hResult = GlobalAlloc(GMEM_MOVEABLE, size); 
			LPTSTR lpsz = (LPTSTR) GlobalLock(hResult); 
			memcpy(lpsz, str.c_str(), size); 
			GlobalUnlock(hResult); 

			#ifndef _UNICODE
			if (SetClipboardData(CF_TEXT, hResult) == NULL) {
			#else
			if (SetClipboardData(CF_UNICODETEXT, hResult) == NULL) {
			#endif
				GlobalFree(hResult);
				MessageBox(_hSelf, TEXT("Unable to set Clipboard data"), _T("PuppetLint"), MB_OK | MB_ICONERROR);
			}
		} else {
			MessageBox(_hSelf, TEXT("Cannot empty the Clipboard"), _T("PuppetLint"), MB_OK | MB_ICONERROR);
		}
		CloseClipboard();
	} else {
		MessageBox(_hSelf, TEXT("Cannot open the Clipboard"), _T("PuppetLint"), MB_OK | MB_ICONERROR);
	}
}
