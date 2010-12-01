#pragma once

INT_PTR CALLBACK OptionsDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);

class JSLintOptions
{
public:
	JSLintOptions();

	void ReadOptions();
	void SaveOptions();

	tstring GetOptionsString() const;

	tstring GetOptionName(UINT id) const;
	UINT GetOptionID(const tstring& name) const;

	void CheckOption(UINT id);
	void UncheckOption(UINT id);

	bool IsOptionChecked(const tstring& name);

	void SetOption(UINT id, const tstring& value);
	void ResetOption(UINT id);

	void ClearAllOptions();
	void SetGoodParts();

	void UpdateDialog(HWND hDlg);

private:
	enum OptionType {
		OPTION_TYPE_UNKNOW,
		OPTION_TYPE_BOOL,
		OPTION_TYPE_INT
	};

	struct Option {
		Option() : type(OPTION_TYPE_UNKNOW) {}

		Option(const tstring& name) 
			: type(OPTION_TYPE_BOOL)
			, name(name)
			, value(TEXT("false"))
			, defaultValue(TEXT("false")) {}

		Option(const tstring& name, const tstring& value) 
			: type(OPTION_TYPE_INT)
			, name(name)
			, value(value)
			, defaultValue(value) {}

		OptionType type;
		tstring name;
		tstring value;
		tstring defaultValue;
	};

	std::map<UINT, Option> m_options;

	static tstring GetConfigFileName();
};

extern JSLintOptions g_jsLintOptions;