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

INT_PTR CALLBACK OptionsDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);

class JSLintOptions
{
public:
	JSLintOptions();

	void ReadOptions();
	void SaveOptions();

	tstring GetOptionsCommentString() const;
	tstring GetOptionsJSONString() const;

	tstring GetOptionName(UINT id) const;
	UINT GetOptionID(const tstring& name) const;

	void CheckOption(UINT id);
	void UncheckOption(UINT id);

	bool IsOptionChecked(const tstring& name) const;

	void SetOption(UINT id, const tstring& value);
	void AppendOption(UINT id, const tstring& value);
	void ResetOption(UINT id);

	void ClearAllOptions();
	void SetGoodParts();

	void UpdateDialog(HWND hDlg);

	int GetTabWidth();

private:
	enum OptionType {
		OPTION_TYPE_UNKNOW,
		OPTION_TYPE_BOOL,
		OPTION_TYPE_INT,
		OPTION_TYPE_ARR_STRING
	};

	struct Option {
		Option() : type(OPTION_TYPE_UNKNOW) {}

		Option(const tstring& name) 
			: type(OPTION_TYPE_BOOL)
			, name(name)
			, value(TEXT("false"))
			, defaultValue(TEXT("false")) {}

		Option(OptionType type, const tstring& name, const tstring& value) 
			: type(type)
			, name(name)
			, value(value)
			, defaultValue(value) {}

		OptionType type;
		tstring name;
		tstring value;
		tstring defaultValue;
	};

	std::map<UINT, Option> m_options;

    bool IsOptionIncluded(const Option& option) const;
};

extern JSLintOptions g_jsLintOptions;