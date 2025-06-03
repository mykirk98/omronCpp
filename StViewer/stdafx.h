
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#ifdef STAPI_DEBUG_BUILD
#ifdef _WIN32
#ifndef  _M_ARM64
#ifdef _DEBUG
#include <vld.h>
#endif //_DEBUG
#endif //_M_ARM64
#endif //_WIN32 
#endif
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type = 'win32' name = 'Microsoft.Windows.Common-Controls' version = '6.0.0.0' processorArchitecture = 'x86' publicKeyToken = '6595b64144ccf1df' language = '*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type = 'win32' name = 'Microsoft.Windows.Common-Controls' version = '6.0.0.0' processorArchitecture = 'amd64' publicKeyToken = '6595b64144ccf1df' language = '*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type = 'win32' name = 'Microsoft.Windows.Common-Controls' version = '6.0.0.0' processorArchitecture = '*' publicKeyToken = '6595b64144ccf1df' language = '*'\"")
#endif
#endif


#include <StApi_TL.h>
#include <StApi_IP.h>
#include <StApi_GUI.h>

#include "Common.h"
#include <afxcontrolbars.h>

#define WM_STVIEWER_START	(WM_APP)
#define WM_DEVICE_LOST		(WM_STVIEWER_START)
#define WM_ADD_LOG			(WM_STVIEWER_START + 1)	//WPARAM:szSource, LPARAM:szLog
#define WM_UPDATED_INTERFACE_LIST (WM_STVIEWER_START + 2)

#define ENABLE_INTERFACE_SPECIFIC_NODE_MAP

typedef enum _EStConfigurationFileTarget_t
{
	StConfigurationFileTarget_First = 0,
	StConfigurationFileTarget_None = 0,
	StConfigurationFileTarget_All,
	StConfigurationFileTarget_SameModel,
	StConfigurationFileTarget_SameID,
	StConfigurationFileTarget_Count
}EStConfigurationFileTarget_t;
typedef enum _EStConfigurationFileType_t
{
	StConfigurationFileType_First = 0,
	StConfigurationFileType_NodeMapWnd = 0,
	StConfigurationFileType_DisplayImageWnd,
	StConfigurationFileType_DefectivePixelDetection,
	StConfigurationFileType_PixelFormatConverter,
	StConfigurationFileType_StViewer,
	StConfigurationFileType_Count
}EStConfigurationFileType_t;
