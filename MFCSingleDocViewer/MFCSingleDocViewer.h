
// MFCSingleDocViewer.h : main header file for the MFCSingleDocViewer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CMFCSingleDocViewerApp:
// See MFCSingleDocViewer.cpp for the implementation of this class
//

class CMFCSingleDocViewerApp : public CWinAppEx
{
public:
	CMFCSingleDocViewerApp();
	StApi::IStDeviceReleasable *CreateIStDevice();

protected:
	StApi::CStApiAutoInit	m_objStApiAutoInit;
	StApi::CIStSystemPtrArray m_objIStSystemPtrList;
// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCameraOpen();
};

extern CMFCSingleDocViewerApp theApp;

/*!
Show the exception contents with message box.
*/
void OnException(const GenICam::GenericException &e);