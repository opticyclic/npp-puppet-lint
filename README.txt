= About =

JSLint Plugin for Notepad++ allows you to run JSLint against your open 
JavaScript files.

Website: http://jslintnpp.sourceforge.net
JSLint Plugin for Notepad++ Copyright 2010 Martin Vladic.

Based on JSLint by Douglas Crockford (http://www.jslint.com/).
JSLint Copyright 2002 Douglas Crockford.

= Release Notes =

This is first version. Consider it beta. Tested and works with Notepad++
version 5.7+; it also may work with older versions.

= Installation =

Copy JSLintNpp.dll (or JSLintNppA.dll if you are using ANSI version) into your
Notepad++ Plugins folder. You will need to re-start Notepad++ afterward.

= Getting Started = 

1. Open your JavaScript file in Notepad++
2. To JSLint your file select menu option 
   "Plugins - JSLint - JSLint Current File" or press shortcut key Ctrl+Shift+L.
3. All errors (if such exists) that are found in your file will be displayed in
   list in dockable window at the bottom of Notepad++ main window; document is
   automatically scrolled at the position of the first found error.
4. Double-click on errors in the list to view them inside document; or you can
   press Ctrl+Shift+N repeatedly to view errors.

You can play with different JSLint options by choosing 
menu option "Plugins - JSLint - Options".

= Files =

README.txt     - This file
JSLintNpp.dll  - Unicode version of plugin
JsLintNppA.dll - ANSI version of plugin

= History =

0.1.0.100    Initial release

= Todo =

- Display list of Global variables (and other useful informations that JSLint
  produces) used in your JS script
  
