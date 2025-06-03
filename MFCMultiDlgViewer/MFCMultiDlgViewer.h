
// MFCMultiDlgViewer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMFCMultiDlgViewerApp:
// See MFCMultiDlgViewer.cpp for the implementation of this class
//

class CMFCMultiDlgViewerApp : public CWinApp
{
public:
	CMFCMultiDlgViewerApp();
	StApi::IStDeviceReleasable *CreateIStDevice();

protected:
	StApi::CStApiAutoInit	m_objStApiAutoInit;
	StApi::CIStSystemPtrArray m_objIStSystemPtrList;

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMFCMultiDlgViewerApp theApp;

/*!
Show the exception contents with message box.
*/
void OnException(const GenICam::GenericException &e);