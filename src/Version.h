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

#define VERSION_MAJOR    0
#define VERSION_MINOR    8
#define VERSION_REVISION 0
#define VERSION_BUILD    116

#define _STR(s) TEXT(#s)
#define STR(s) _STR(s)

#define MY_PRODUCT_VERSION STR(VERSION_MAJOR) TEXT(".") STR(VERSION_MINOR) TEXT(".") STR(VERSION_REVISION) TEXT(".") STR(VERSION_BUILD)
#define MY_PRODUCT_VERSION_NUM VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION,VERSION_BUILD
