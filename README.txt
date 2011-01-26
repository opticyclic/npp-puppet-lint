= About =

JSLint Plugin for Notepad++ allows you to run JSLint (The JavaScript Code 
Quality Tool) against your open JavaScript files.

Website: http://jslintnpp.sourceforge.net
JSLint Plugin for Notepad++ Copyright 2010 Martin Vladic.

Based on JSLint by Douglas Crockford (http://www.jslint.com/).
JSLint Copyright 2002 Douglas Crockford.

= Release Notes =

This is beta version. Tested and works with Notepad++ version 5.7+; it also 
may work with older versions.

= Installation =

Copy JSLintNpp.dll (or JSLintNppA.dll if you are using ANSI version) into your
Notepad++ Plugins folder. You will need to re-start Notepad++ afterward.

= Getting Started = 

1. Open your JavaScript file in Notepad++
2. To JSLint your file select menu option 
   "Plugins - JSLint - JSLint Current File" or press shortcut key Ctrl+Shift+F5.
3. All errors (if such exists) that are found in your file will be displayed in
   list in dockable window at the bottom of Notepad++ main window; document is
   automatically scrolled at the position of the first found error.
4. Double-click on errors in the list to view them inside document; or you can
   press Ctrl+Shift+F8 repeatedly to view errors.

You can play with different JSLint options by choosing 
menu option "Plugins - JSLint - Options".

= Files =

README.txt     - This file
JSLintNpp.dll  - Unicode version of plugin
JSLintNppA.dll - ANSI version of plugin

= History =

0.4.0   
    - Bug Fixes
    - Context menu in list control is added
    - Easily add predefined globals (Ticket ID 3159082)

0.3.1   
    - Bug fixes
    - List control now supports multiple selection (use Ctrl+A to select all 
      lints)
    - Added clipboard copy for the selected lints (use Ctrl+C)

0.3.0   
    - New option "Predefined (, separated)" added in Options dialog; this 
      option should be used to specify the names of predefined global variables
    - Added toolbar button that opens Options dialog
    - JSLint script updated to version from 2010-12-23

0.2.1
	- Configuration is now saved in AppData directory (or in whatever
      directory is returned by NPPM_GETPLUGINSCONFIGDIR message)

0.2.0
	- Memory leaks fixed
	- Toolbar added to the JSLint dockable window
	- Visibility state of the JSLint dockable window is now preserved after 
      restart
	- Shortcuts changed (to avoid clashes with existing shortcuts of Notepad++)

0.1.1
	- Bug fixes

0.1.0
	- Initial release

= Todo =

- Display list of global variables (and other useful informations that JSLint
  produces) used in your JS script
  
