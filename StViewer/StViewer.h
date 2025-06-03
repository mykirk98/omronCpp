
// StViewer.h : main header file for the StViewer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include <vector>
#include <map>

// CStViewerApp:
// See StViewer.cpp for the implementation of this class
//

class CStViewerApp : public CWinAppEx
{
public:
	CStViewerApp();
	~CStViewerApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

public:
	EStConfigurationFileTarget_t GetConfigurationFileTarget(EStConfigurationFileType_t eType) const
	{
		return(m_peConfigurationFileTarget[eType]);
	}
	void SetConfigurationFileTarget(EStConfigurationFileType_t eType, EStConfigurationFileTarget_t eTarget)
	{
		m_peConfigurationFileTarget[eType] = eTarget;
	}
	LPCTSTR GetConfigurationFileName(EStConfigurationFileType_t eType) const
	{
		return(m_pszConfigurationFileName[eType]);
	}
	void LoadSaveNodeMapSettingFile(GenApi::INodeMap *pINodeMap, bool isLoad, EStConfigurationFileType_t eType, const StApi::IStDevice *pIStDevice);
	void LoadSaveNodeMapSettingFile(GenApi::INodeMap *pINodeMap, bool isLoad, EStConfigurationFileType_t eType, LPCTSTR szDeviceName);

#ifdef ENABLE_INTERFACE_SPECIFIC_NODE_MAP
	void UpdateInterfaceNodeMapDisplayWnd();
#endif //ENABLE_INTERFACE_SPECIFIC_NODE_MAP

protected:
	StApi::CStApiAutoInit	m_objStApiAutoInit;
	StApi::CIStSystemPtrArray m_objIStSystemPtrList;

	static const TCHAR * const m_szConfigFileSectionName;
	static const TCHAR * const m_pszConfigFileEntryName[StConfigurationFileType_Count];
	static const TCHAR * const m_pszConfigurationFileName[StConfigurationFileType_Count];
	EStConfigurationFileTarget_t m_peConfigurationFileTarget[StConfigurationFileType_Count];
	void mLoadSaveNodeMapSettingFile(GenApi::INodeMap *pINodeMap, bool isLoad, EStConfigurationFileTarget_t eTarget, LPCTSTR szFileName);

#ifdef ENABLE_INTERFACE_SPECIFIC_NODE_MAP
	StApi::CIStNodeMapDisplayWndPtr m_objIStNodeMapDisplayWndForInterfaceSpecificNodeMap;
	StApi::CIStNodeMapDisplayWndPtr m_objIStNodeMapDisplayWndForSystemSpecificNodeMap;
	typedef std::map<StApi::IStInterface *, std::pair<StApi::RegisteredINodeHandle_t, StApi::IStRegisteredCallbackReleasable*>> StRegisteredInterfaceInfo_t;
	StRegisteredInterfaceInfo_t m_StRegisteredInterfaceInfo;
#endif //ENABLE_INTERFACE_SPECIFIC_NODE_MAP

public:
	/*!
	Select device and get IStDeviceReleasable pointer.
	*/
	StApi::IStDeviceReleasable *CreateIStDevice();
#ifdef ENABLE_INTERFACE_SPECIFIC_NODE_MAP
	void ShowNodeMapForInterfaces(HWND hParentWnd)
	{
		m_objIStNodeMapDisplayWndForInterfaceSpecificNodeMap->Show(hParentWnd, StApi::StWindowMode_Modaless);
	};
	void ShowNodeMapForSystems(HWND hParentWnd)
	{
		m_objIStNodeMapDisplayWndForSystemSpecificNodeMap->Show(hParentWnd, StApi::StWindowMode_Modaless);
	};
	void OnStCallback_IStInterface(StApi::IStCallbackParamBase *, void *pvContext);
	void OnStCallback_IStSystem(StApi::IStCallbackParamBase *, void *pvContext);
#endif //ENABLE_INTERFACE_SPECIFIC_NODE_MAP
	afx_msg void OnFileConfigurationfilesetting();
};

extern CStViewerApp theApp;
