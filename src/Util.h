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

tstring TrimSpaces(const tstring& str);

BOOL CenterWindow(HWND hWnd, HWND hParentWnd, BOOL bRepaint = FALSE);

////////////////////////////////////////////////////////////////////////////////

class Win32Handle
{
	Win32Handle(const Win32Handle& rhs);
	Win32Handle& operator=(const Win32Handle& rhs);

public:
	Win32Handle() : m_handle(NULL) {}
	Win32Handle(HANDLE handle) : m_handle(handle) {}

	~Win32Handle() { 
		if (m_handle != NULL) {
			CloseHandle(m_handle); 
		}
	}

	Win32Handle& operator=(HANDLE handle) { 
		if (m_handle != NULL) {
			CloseHandle(m_handle);
		}
		m_handle = handle; 
		return *this; 
	}

	operator HANDLE() { return m_handle; }
	HANDLE* operator &() { return &m_handle; }

private:
	HANDLE m_handle;
};

////////////////////////////////////////////////////////////////////////////////

class IOException : public exception
{
};

////////////////////////////////////////////////////////////////////////////////

class Path
{
public:
	static tstring GetPathRoot(const tstring& strPath);
	static tstring GetDirectoryName(const tstring& strPath);
	static tstring GetFileName(const tstring& strPath);
	static tstring GetFileNameWithoutExtension(const tstring& strPath);
	static tstring GetPathNameWithoutExtension(const tstring& strPath);
	static tstring GetExtension(const tstring& strPath);

	static tstring GetFullPath(const tstring& strPath);
	static tstring GetFullPath(const tstring& strPath, const tstring& strBaseDir);

	static bool IsRelative(const tstring& strPath);
	static bool IsDir(const tstring& strPath);
	static bool IsFileExists(const tstring& strPath);

	static tstring GetTempFileName();
	static tstring GetModuleFileName(HMODULE hModule);
};

////////////////////////////////////////////////////////////////////////////////

class TempFile
{
public:
	~TempFile() {
		// delete temp file
		if (!m_strFileName.empty())
			DeleteFile(m_strFileName.c_str());
	}

	// create temp file
	void Create() {
		m_strFileName = Path::GetTempFileName();
		if (m_strFileName.empty()) {
			throw IOException();
		}
	}

	// test if temp file is created
	operator bool() const { return !m_strFileName.empty(); }

	// return temp file name as std string
	const tstring& GetFileName() const { return m_strFileName; }

	// return temp file name as Win32 string
	operator LPCTSTR() const { return m_strFileName.c_str(); }

private:
	tstring m_strFileName;
};

////////////////////////////////////////////////////////////////////////////////

class TextConversion
{
public:
	static string UTF8_To_S(const string& str) {
		int wsize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		wchar_t* wbuffer = new wchar_t[wsize];
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wbuffer, wsize);
		int size = WideCharToMultiByte(CP_ACP, 0, wbuffer, wsize, NULL, 0, NULL, NULL);
		char* buffer = new char[size];
		WideCharToMultiByte(CP_ACP, 0, wbuffer, wsize, buffer, size, NULL, NULL);
		return buffer;
	}

	static wstring UTF8_To_W(const string& str) {
		int wsize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		wchar_t* wbuffer = new wchar_t[wsize];
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wbuffer, wsize);
		return wbuffer;
	}

	static tstring UTF8_To_T(const string& str) {
	#if defined(UNICODE) || defined(_UNICODE)
		return UTF8_To_W(str);
	#else
		return UTF8_To_S(str);
	#endif
	}

	static string S_To_UTF8(const string& str) {
		int wsize = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
		wchar_t* wbuffer = new wchar_t[wsize];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wbuffer, wsize);

		int size = WideCharToMultiByte(CP_UTF8, 0, wbuffer, wsize, NULL, 0, NULL, NULL);
		char* buffer = new char[size];
		WideCharToMultiByte(CP_UTF8, 0, wbuffer, wsize, buffer, size, NULL, NULL);
		return buffer;
	}

	static string W_To_UTF8(const wstring& wstr) {
		int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
		char* buffer = new char[size];
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, buffer, size, NULL, NULL);
		return buffer;
	}

	static string T_To_UTF8(const tstring& tstr) {
	#if defined(UNICODE) || defined(_UNICODE)
		return W_To_UTF8(tstr);
	#else
		return S_To_UTF8(tstr);
	#endif
	}
};
