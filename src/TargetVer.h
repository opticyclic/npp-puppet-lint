// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
// your application.  The macros work by enabling all features available on platform versions up to and 
// including the version specified.

#ifndef WINVER                  // Specifies that the minimum required platform is Windows XP.
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows XP.
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE               // Specifies that the minimum required platform is Internet Explorer 6.0.
#define _WIN32_IE 0x0600
#endif
