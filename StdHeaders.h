#include <Windows.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <Shlwapi.h>

#pragma warning(disable:4996)

#include <string>
#include <tchar.h>
#include <list>
#include <map>
#include <sstream>
#include <vector>

using namespace std;

#if defined(UNICODE) || defined(_UNICODE)
typedef wstring tstring;
#else
typedef string tstring;
#endif
