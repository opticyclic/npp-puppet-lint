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

#ifndef OUTPUT_DLG_H
#define OUTPUT_DLG_H

#include "DockingFeature/DockingDlgInterface.h"
#include "DockingFeature/Toolbar.h"
#include "resource.h"
#include "JSLint.h"

class OutputDlg : public DockingDlgInterface
{
public :
	OutputDlg();
	~OutputDlg();

    virtual void display(bool toShow = true) const {
        DockingDlgInterface::display(toShow);
        if (toShow)
            ::SetFocus(::GetDlgItem(_hSelf, IDC_OUTPUT));
    };

	virtual void redraw(void) {
		::RedrawWindow(m_toolbar.getHSelf(), NULL, NULL, TRUE);
	};

	void setParent(HWND parent2set){
		_hParent = parent2set;
	};

	HICON GetTabIcon();

	void ClearAllLints();
	void AddLints(const tstring& strFilePath, const list<JSLintReportItem>& lints);
	void SelectNextLint();
	void SelectPrevLint();

protected :
	virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

	void OnToolbarCmd(UINT message);
	void OnToolbarDropDown(LPNMTOOLBAR lpnmtb);

private:
	ReBar m_rebar;
	ToolBar m_toolbar;

	HWND m_hWndListView;
	HICON m_hTabIcon;

	struct FileLint {
		FileLint(const tstring& strFilePath, const JSLintReportItem& lint)
			: strFilePath(strFilePath), lint(lint) {}
		tstring strFilePath;
		JSLintReportItem lint;
	};

	vector<FileLint> m_fileLints;

	enum {
		COL_NUM    = 0,
		COL_REASON = 1,
		COL_FILE   = 2,
		COL_LINE   = 3,
		COL_COLUMN = 4
	};

	void InitializeToolbar();
	void InitializeListViewCtrl();

	void Resize();

	void GetNameStrFromCmd(UINT resID, LPTSTR tip, UINT count);

	void ShowLint(int i);
};

#endif //OUTPUT_DLG_H
