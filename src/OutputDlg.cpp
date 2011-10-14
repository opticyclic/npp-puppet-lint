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
#include "JSLintOptions.h"

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
#define	IDM_TB_JSLINT_OPTIONS      (IDM_TOOLBAR + 5)

////////////////////////////////////////////////////////////////////////////////

OutputDlg::TabDefinition OutputDlg::m_tabs[] = {
    {
        TEXT("Errors"),
        IDC_ERROR_LIST,
        true
    },
    {
        TEXT("Unused"),
        IDC_UNUSED_LIST,
        false
    }
};

////////////////////////////////////////////////////////////////////////////////

OutputDlg::OutputDlg()
	: DockingDlgInterface(IDD_OUTPUT)
{
    for (int i = 0; i < NUM_LIST_VIEWS; ++i) {
        m_hWndListViews[i] = 0;
    }
}

OutputDlg::~OutputDlg()
{
	DestroyIcon(m_hTabIcon);
}

BOOL CALLBACK OutputDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    int i;

	switch (message) 
	{
		case WM_INITDIALOG:
			InitializeToolbar();
            InitializeTab();
            for (i = 0; i < NUM_LIST_VIEWS; ++i) {
			    InitializeListView(i);
            }
            OnTabSelChanged();
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
                        int iTab = TabCtrl_GetCurSel(m_hWndTab);
                        int iLint = ListView_GetNextItem(m_hWndListViews[iTab], -1, LVIS_FOCUSED | LVIS_SELECTED);
						if (iLint != -1) {
							ShowLint(iLint);
						}
						return TRUE;
					} else if (LOWORD(wParam) == ID_ADD_PREDEFINED) {
                        int iTab = TabCtrl_GetCurSel(m_hWndTab);
                        int iLint = ListView_GetNextItem(m_hWndListViews[iTab], -1, LVIS_FOCUSED | LVIS_SELECTED);
						if (iLint != -1) {
							const FileLint& fileLint = m_fileLints[iTab][iLint];
							tstring var = fileLint.lint.GetUndefVar();
							if (!var.empty()) {
                                JSLintOptions::GetInstance().AppendOption(IDC_PREDEFINED, var);
							}
						}
						return TRUE;
					} else if (LOWORD(wParam) == ID_SELECT_ALL) {
                        ListView_SetItemState(m_hWndListViews[TabCtrl_GetCurSel(m_hWndTab)], -1, LVIS_SELECTED, LVIS_SELECTED);
						return TRUE;
					}
				}
			}
			break;

		case WM_NOTIFY: {
				LPNMHDR pNMHDR = (LPNMHDR) lParam;
                if (pNMHDR->idFrom == m_tabs[TabCtrl_GetCurSel(m_hWndTab)].m_listViewID && pNMHDR->code == LVN_KEYDOWN) {
					LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
					if (pnkd->wVKey == 'A' && (::GetKeyState(VK_CONTROL) >> 15 & 1)) {
						ListView_SetItemState(m_hWndListViews[TabCtrl_GetCurSel(m_hWndTab)], -1, LVIS_SELECTED, LVIS_SELECTED);
					} else if (pnkd->wVKey == 'C' && (::GetKeyState(VK_CONTROL) >> 15 & 1)) {
						CopyToClipboard();
					}
				} else if (pNMHDR->idFrom == m_tabs[TabCtrl_GetCurSel(m_hWndTab)].m_listViewID && pNMHDR->code == NM_DBLCLK) {
					LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
                    int iFocused = lpnmitem->iItem;
					if (iFocused != -1) {
						ShowLint(iFocused);
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
                } else if (pNMHDR->idFrom == IDC_TAB && pNMHDR->code == TCN_SELCHANGE) {
                    OnTabSelChanged();
                    return TRUE;
                }
				DockingDlgInterface::run_dlgProc(message, wParam, lParam);
				return FALSE;
			}
			break;

		case WM_CONTEXTMENU: {
				// build context menu
				HMENU menu = ::CreatePopupMenu();

				int numSelected = ListView_GetSelectedCount(m_hWndListViews[TabCtrl_GetCurSel(m_hWndTab)]);

                int iTab = TabCtrl_GetCurSel(m_hWndTab);

				int iFocused = -1;
				if (numSelected > 0) {
					iFocused = ListView_GetNextItem(m_hWndListViews[iTab], -1, LVIS_FOCUSED | LVIS_SELECTED);
				}

				bool reasonIsUndefVar = false;
				if (iFocused != -1) {
					const FileLint& fileLint = m_fileLints[iTab][iFocused];
					reasonIsUndefVar = fileLint.lint.IsReasonUndefVar();
				}

				if (iFocused != -1) {
					AppendMenu(menu, MF_ENABLED, ID_SHOW_LINT, TEXT("Show"));
				}

				if (reasonIsUndefVar) {
					AppendMenu(menu, MF_ENABLED, ID_ADD_PREDEFINED, TEXT("Add to the Predefined List"));
				}

				if (GetMenuItemCount(menu) > 0)
					AppendMenu(menu, MF_SEPARATOR, 0, NULL);

				if (numSelected > 0) {
					AppendMenu(menu, MF_ENABLED, ID_COPY_LINTS, TEXT("Copy"));
				}

				AppendMenu(menu, MF_ENABLED, ID_SELECT_ALL, TEXT("Select All"));

				// determine context menu position
				POINT point;
				point.x = LOWORD(lParam);
				point.y = HIWORD(lParam);
				if (point.x == 65535 || point.y == 65535) {
					point.x = 0;
					point.y = 0;
					ClientToScreen(m_hWndListViews[TabCtrl_GetCurSel(m_hWndTab)], &point);
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
			jsLintCurrentFile();
			break;
		case IDM_TB_JSLINT_ALL_FILES:
			jsLintAllFiles();
			break;
		case IDM_TB_NEXT_LINT:
			gotoNextLint();
			break;
		case IDM_TB_PREV_LINT:
			gotoPrevLint();
			break;
		case IDM_TB_JSLINT_OPTIONS:
			showJSLintOptionsDlg();
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
		{IDM_TB_JSLINT_OPTIONS,      IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDB_TB_LINT_OPTIONS, 0},
	};

	m_toolbar.init(_hInst, _hSelf, TB_STANDARD, toolBarIcons, sizeof(toolBarIcons) / sizeof(ToolBarButtonUnit));
	m_rebar.init(_hInst, _hSelf);
	m_toolbar.addToRebar(&m_rebar);
	m_rebar.setIDVisible(REBAR_BAR_TOOLBAR, true);
}

void OutputDlg::InitializeTab()
{
    m_hWndTab = ::GetDlgItem(_hSelf, IDC_TAB);

    TCITEM tie;

    tie.mask = TCIF_TEXT | TCIF_IMAGE; 
    tie.iImage = -1; 

    for (int i = 0; i < NUM_LIST_VIEWS; ++i) {
        tie.pszText = (LPTSTR)m_tabs[i].m_strTabName; 
        TabCtrl_InsertItem(m_hWndTab, i, &tie); 
    }
}

void OutputDlg::InitializeListView(int i)
{
    m_hWndListViews[i] = ::GetDlgItem(_hSelf, m_tabs[i].m_listViewID);

	ListView_SetExtendedListViewStyle(m_hWndListViews[i], LVS_EX_FULLROWSELECT);

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    int iCol = 0;

	lvc.iSubItem = iCol;
	lvc.pszText = TEXT("");
	lvc.cx = 28;
	lvc.fmt = LVCFMT_RIGHT;
	ListView_InsertColumn(m_hWndListViews[i], iCol++, &lvc);

    if (m_tabs[i].m_errorList) {
	    lvc.iSubItem = iCol;
	    lvc.pszText = TEXT("Reason");
	    lvc.cx = 500;
	    lvc.fmt = LVCFMT_LEFT;
	    ListView_InsertColumn(m_hWndListViews[i], iCol++, &lvc);
    } else {
	    lvc.iSubItem = iCol;
	    lvc.pszText = TEXT("Variable");
	    lvc.cx = 250;
	    lvc.fmt = LVCFMT_LEFT;
	    ListView_InsertColumn(m_hWndListViews[i], iCol++, &lvc);

	    lvc.iSubItem = iCol;
	    lvc.pszText = TEXT("Function");
	    lvc.cx = 250;
	    lvc.fmt = LVCFMT_LEFT;
	    ListView_InsertColumn(m_hWndListViews[i], iCol++, &lvc);
    }

    lvc.iSubItem = iCol;
	lvc.pszText = TEXT("File");
	lvc.cx = 200;
	lvc.fmt = LVCFMT_LEFT;
	ListView_InsertColumn(m_hWndListViews[i], iCol++, &lvc);

	lvc.iSubItem = iCol;
	lvc.pszText = TEXT("Line");
	lvc.cx = 50;
	lvc.fmt = LVCFMT_RIGHT;
	ListView_InsertColumn(m_hWndListViews[i], iCol++, &lvc);

	lvc.iSubItem = iCol;
	lvc.pszText = TEXT("Column");
	lvc.cx = 50;
	lvc.fmt = LVCFMT_RIGHT;
	ListView_InsertColumn(m_hWndListViews[i], iCol++, &lvc);
}

void OutputDlg::Resize()
{
	RECT rc;
	getClientRect(rc);

	m_toolbar.reSizeTo(rc);
	m_rebar.reSizeTo(rc);

    RECT rcToolbar;
    GetWindowRect(m_toolbar.getHSelf(), &rcToolbar);

	getClientRect(rc);
    rc.top += rcToolbar.bottom - rcToolbar.top;

    //InflateRect(&rc, -4, -4);
    ::MoveWindow(m_hWndTab, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

    TabCtrl_AdjustRect(m_hWndTab, FALSE, &rc);
    //InflateRect(&rc, -4, -4);
    for (int i = 0; i < NUM_LIST_VIEWS; ++i) {
        ::SetWindowPos(m_hWndListViews[i], m_hWndTab, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0);
    }
}

void OutputDlg::OnTabSelChanged()
{
    int iSel = TabCtrl_GetCurSel(m_hWndTab);
    for (int i = 0; i < NUM_LIST_VIEWS; ++i) {
        ShowWindow(m_hWndListViews[i], iSel == i ? SW_SHOW : SW_HIDE);
    }
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
		TEXT("JSLint Current File"),
		TEXT("JSLint All Files"),
		TEXT("Go To Previous Lint"),
		TEXT("Go To Next Lint"),
		TEXT("JSLint Options"),
	};

	_tcscpy(tip, szToolTip[resID - IDM_TB_JSLINT_CURRENT_FILE]);
}

void OutputDlg::ClearAllLints()
{
    for (int i = 0; i < NUM_LIST_VIEWS; ++i) {
    	m_fileLints[i].clear();
	    ListView_DeleteAllItems(m_hWndListViews[i]);
    }
}

void OutputDlg::AddLints(const tstring& strFilePath, const list<JSLintReportItem>& lints)
{
	basic_stringstream<TCHAR> stream;

	LVITEM lvI;
	lvI.mask = LVIF_TEXT | LVIF_STATE;

	for (list<JSLintReportItem>::const_iterator it = lints.begin(); it != lints.end(); ++it) {
		const JSLintReportItem& lint = *it;

        HWND hWndListView = m_hWndListViews[lint.GetType()];

		lvI.iSubItem = 0;
		lvI.iItem = ListView_GetItemCount(hWndListView);
		lvI.state = 0;
		lvI.stateMask = 0;

		stream.str(TEXT(""));
		stream << lvI.iItem + 1;
		tstring strNum = stream.str();

		lvI.pszText = (LPTSTR)strNum.c_str();
		
		ListView_InsertItem(hWndListView, &lvI);

        int iCol = 1;

        if (m_tabs[lint.GetType()].m_errorList) {
		    tstring strReason = lint.GetReason();
		    ListView_SetItemText(hWndListView, lvI.iItem, iCol++, (LPTSTR)strReason.c_str());
        } else {
		    tstring strVariable = lint.GetReason();
		    ListView_SetItemText(hWndListView, lvI.iItem, iCol++, (LPTSTR)strVariable.c_str());
		    tstring strFunction = lint.GetEvidence();
		    ListView_SetItemText(hWndListView, lvI.iItem, iCol++, (LPTSTR)strFunction.c_str());
        }

		tstring strFile = Path::GetFileName(strFilePath);
		ListView_SetItemText(hWndListView, lvI.iItem, iCol++, (LPTSTR)strFile.c_str());

		stream.str(TEXT(""));
		stream << lint.GetLine() + 1;
		tstring strLine = stream.str();
		ListView_SetItemText(hWndListView, lvI.iItem, iCol++, (LPTSTR)strLine.c_str());
		
		stream.str(TEXT(""));
		stream << lint.GetCharacter() + 1;
		tstring strColumn = stream.str();
		ListView_SetItemText(hWndListView, lvI.iItem, iCol++, (LPTSTR)strColumn.c_str());

        m_fileLints[lint.GetType()].push_back(FileLint(strFilePath, lint));
	}

    for (int i = 0; i < NUM_LIST_VIEWS; ++i) {
        tstring strTabName;
        int count = ListView_GetItemCount(m_hWndListViews[i]);
        if (count > 0) {
            stream.str(TEXT(""));
            stream << m_tabs[i].m_strTabName << TEXT(" (") << count << TEXT(")");
    		strTabName = stream.str();
        } else {
            strTabName = m_tabs[i].m_strTabName;
        }
        TCITEM tie;
        tie.mask = TCIF_TEXT; 
        tie.pszText = (LPTSTR)strTabName.c_str(); 
        TabCtrl_SetItem(m_hWndTab, i, &tie);
    }

    InvalidateRect(getHSelf(), NULL, TRUE);
}

void OutputDlg::SelectNextLint()
{
	if (_hSelf == NULL)
		return;

    int iTab = TabCtrl_GetCurSel(m_hWndTab);
    HWND hWndListView = m_hWndListViews[iTab];

	int count = ListView_GetItemCount(hWndListView);
	if (count == 0)
		return;

	int i = ListView_GetNextItem(hWndListView, -1, LVNI_FOCUSED | LVNI_SELECTED);
	if (++i == count)
		i = 0;

	ListView_SetItemState(hWndListView, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
	
	ListView_SetItemState(hWndListView, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	ListView_EnsureVisible(hWndListView, i, FALSE);
	ShowLint(i);
}

void OutputDlg::SelectPrevLint()
{
	if (_hSelf == NULL)
		return;

    int iTab = TabCtrl_GetCurSel(m_hWndTab);
    HWND hWndListView = m_hWndListViews[iTab];

    int count = ListView_GetItemCount(hWndListView);
	if (count == 0)
		return;

	int i = ListView_GetNextItem(hWndListView, -1, LVNI_FOCUSED | LVNI_SELECTED);
	if (--i == -1)
		i = count - 1;
	
	ListView_SetItemState(hWndListView, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);

	ListView_SetItemState(hWndListView, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	ListView_EnsureVisible(hWndListView, i, FALSE);
	ShowLint(i);
}

void OutputDlg::ShowLint(int i)
{
    int iTab = TabCtrl_GetCurSel(m_hWndTab);
	const FileLint& fileLint = m_fileLints[iTab][i];
	
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

    InvalidateRect(getHSelf(), NULL, TRUE);
}

void OutputDlg::CopyToClipboard()
{
	basic_stringstream<TCHAR> stream;

    int iTab = TabCtrl_GetCurSel(m_hWndTab);

	bool bFirst = true;
	int i = ListView_GetNextItem(m_hWndListViews[iTab], -1, LVNI_SELECTED);
	while (i != -1) {
		const FileLint& fileLint = m_fileLints[iTab][i];

		if (bFirst) {
			bFirst = false;
		} else {
			stream << TEXT("\r\n");
		}

		stream << TEXT("Line ") << fileLint.lint.GetLine() + 1
			<< TEXT(", column ") << fileLint.lint.GetCharacter() + 1
			<< TEXT(": ") << fileLint.lint.GetReason().c_str() 
			<< TEXT("\r\n\t") << fileLint.lint.GetEvidence().c_str() << TEXT("\r\n");

		i = ListView_GetNextItem(m_hWndListViews[TabCtrl_GetCurSel(m_hWndTab)], i, LVNI_SELECTED);
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
				MessageBox(_hSelf, TEXT("Unable to set Clipboard data"), TEXT("JSLint"), MB_OK | MB_ICONERROR);
			}
		} else {
			MessageBox(_hSelf, TEXT("Cannot empty the Clipboard"), TEXT("JSLint"), MB_OK | MB_ICONERROR);
		}
		CloseClipboard();
	} else {
		MessageBox(_hSelf, TEXT("Cannot open the Clipboard"), TEXT("JSLint"), MB_OK | MB_ICONERROR);
	}
}
