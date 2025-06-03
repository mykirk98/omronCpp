
// StViewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "StViewer.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "StViewerDoc.h"
#include "StStillImageDoc.h"
#include "MyDocManager.h"
#include "StViewerView.h"
#include "ConfigurationFileSettingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace GenTL;
using namespace StApi;

const TCHAR * const CStViewerApp::m_szConfigFileSectionName = TEXT("ConfigurationFile");
const TCHAR * const CStViewerApp::m_pszConfigFileEntryName[StConfigurationFileType_Count] = 
{
	TEXT("DeviceNodeMapWindow"),
	TEXT("DisplayImageWindow"),
	TEXT("DefectivePixelDetection"),
	TEXT("PixelFormatConverter"),
	TEXT("StViewer")
};

const TCHAR * const CStViewerApp::m_pszConfigurationFileName[StConfigurationFileType_Count] =
{
	TEXT("DeviceNodeMapWnd.cfg"),
	TEXT("DisplayImageWnd.cfg"),
	TEXT("DefectivePixelDetection.cfg"),
	TEXT("PixelFormatConverter.cfg"),
	TEXT("StViewer.cfg")
};


// CStViewerApp

BEGIN_MESSAGE_MAP(CStViewerApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CStViewerApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CStViewerApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CStViewerApp::OnFileOpen)
	ON_COMMAND(ID_FILE_CONFIGURATIONFILESETTING, &CStViewerApp::OnFileConfigurationfilesetting)
END_MESSAGE_MAP()


// CStViewerApp construction

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStViewerApp::CStViewerApp() try : 
	// Before using the library, perform the initialization. 
	// StApiInitialize () is called by the constructor of CStApiAutoInit, StApiTerminate () is called by the destructor.
	m_objStApiAutoInit()
{
	m_bHiColorIcons = TRUE;

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("StViewer.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}
catch (const GenICam::GenericException &e)
{
	OnException(e);
}

// The one and only CStViewerApp object

CStViewerApp theApp;


// CStViewerApp initialization
CStViewerApp::~CStViewerApp()
{
	StRegisteredInterfaceInfo_t::iterator itr = m_StRegisteredInterfaceInfo.begin();
	while(itr != m_StRegisteredInterfaceInfo.end())
	{
		m_objIStNodeMapDisplayWndForInterfaceSpecificNodeMap->DeregisterINode(itr->second.first);
		itr->second.second->Release();
		itr = m_StRegisteredInterfaceInfo.erase(itr);
	}

}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CStViewerApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction();

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("StApi"));
	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);


	//Create object and initialize "IStSystemPtr" list.
#if 1
	const uint32_t nCount = StSystemVendor_Count;
	//const uint32_t nCount = 1;	//Default only
	//const uint32_t nCount = 2;	//Default + Euresys
	for (uint32_t i = StSystemVendor_Default; i < nCount; ++i)
	{
		EStSystemVendor_t eStSystemVendor = (EStSystemVendor_t)i;
		try
		{
			// Create a system object, to get the IStSystemReleasable interface pointer.
			// After the system object is no longer needed, call the IStSystemReleasable::Release(), please discard the system object.
			IStSystemReleasable *pIStSystemReleasable = CreateIStSystem(eStSystemVendor, StInterfaceType_All);
			m_objIStSystemPtrList.Register(pIStSystemReleasable);
			RegisterCallback(pIStSystemReleasable, *this, &CStViewerApp::OnStCallback_IStSystem, (void*)NULL);
			pIStSystemReleasable->StartEventAcquisitionThread();
		}
		catch (const GenICam::GenericException &e)
		{
			if (eStSystemVendor == StSystemVendor_Default)
			{
				OnException(e);
			}
		}
	}
#else
	EStSystemVendor_t peSystemVendorArray[] = { StSystemVendor_Default, StSystemVendor_Euresys, StSystemVendor_Kaya };
	for (size_t i = 0; i < sizeof(peSystemVendorArray) / sizeof(EStSystemVendor_t); i++)
	{
		try
		{
			// Create a system object, to get the IStSystemReleasable interface pointer.
			// After the system object is no longer needed, call the IStSystemReleasable::Release(), please discard the system object.
			m_objIStSystemPtrList.Register(CreateIStSystem(peSystemVendorArray[i], StInterfaceType_All));
		}
		catch (const GenICam::GenericException &e)
		{
			if (peSystemVendorArray[i] == StSystemVendor_Default)
			{
				OnException(e);
			}
		}
	}
#endif

#ifdef ENABLE_INTERFACE_SPECIFIC_NODE_MAP
	m_objIStNodeMapDisplayWndForSystemSpecificNodeMap = StApi::CreateIStWnd(StWindowType_NodeMapDisplay);
	m_objIStNodeMapDisplayWndForInterfaceSpecificNodeMap = StApi::CreateIStWnd(StWindowType_NodeMapDisplay);
	for (size_t i = 0; i < m_objIStSystemPtrList.GetSize(); ++i)
	{
		IStSystem *pIStSystem = m_objIStSystemPtrList[i];
		const GenICam::gcstring strSystemName(pIStSystem->GetIStSystemInfo()->GetDisplayName());
		m_objIStNodeMapDisplayWndForSystemSpecificNodeMap->RegisterINode(pIStSystem->GetIStPort()->GetINodeMap()->GetNode("Root"), strSystemName);
	}
	UpdateInterfaceNodeMapDisplayWnd();

	m_objIStNodeMapDisplayWndForSystemSpecificNodeMap->Collapse();
	m_objIStNodeMapDisplayWndForInterfaceSpecificNodeMap->Collapse();
#endif //ENABLE_INTERFACE_SPECIFIC_NODE_MAP

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	this->m_pDocManager = new CMyDocManager();
	CMultiDocTemplate* pDocTemplate;

	//Template for devices.
	pDocTemplate = new CMultiDocTemplate(IDR_CAMERA_CTRL,
		RUNTIME_CLASS(CStViewerDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CStViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	//Template for still images.
	pDocTemplate = new CMultiDocTemplate(IDI_FILE_CTRL,
		RUNTIME_CLASS(CStStillImageDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CStViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CMultiDocTemplate(IDI_FILE_BMP_CTRL,
		RUNTIME_CLASS(CStStillImageDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CStViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CMultiDocTemplate(IDI_FILE_PNG_CTRL,
		RUNTIME_CLASS(CStStillImageDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CStViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CMultiDocTemplate(IDI_FILE_TIF_CTRL,
		RUNTIME_CLASS(CStStillImageDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CStViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CMultiDocTemplate(IDI_FILE_TIFF_CTRL,
		RUNTIME_CLASS(CStStillImageDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CStViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CMultiDocTemplate(IDI_FILE_JPG_CTRL,
		RUNTIME_CLASS(CStStillImageDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CStViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CMultiDocTemplate(IDI_FILE_JPEG_CTRL,
		RUNTIME_CLASS(CStStillImageDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CStViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	
	return TRUE;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerApp::UpdateInterfaceNodeMapDisplayWnd()
{
	StRegisteredInterfaceInfo_t mapOldStRegisteredInterfaceInfo = m_StRegisteredInterfaceInfo;
	m_StRegisteredInterfaceInfo.clear();

	bool isUpdated = false;

	for (size_t i = 0; i < m_objIStSystemPtrList.GetSize(); ++i)
	{
		IStSystem *pIStSystem = m_objIStSystemPtrList[i];
		for (size_t j = 0; j < pIStSystem->GetInterfaceCount(); ++j)
		{
			try
			{
				IStInterface *pIStInterface = pIStSystem->GetIStInterface(j);
				StRegisteredInterfaceInfo_t::iterator itr = mapOldStRegisteredInterfaceInfo.find(pIStInterface);
				if (itr != mapOldStRegisteredInterfaceInfo.end())
				{
					m_StRegisteredInterfaceInfo.insert(*itr);
					mapOldStRegisteredInterfaceInfo.erase(itr);
				}
				else
				{
					pIStInterface->StopEventAcquisitionThread();

					GenApi::CNodeMapPtr pINodeMap(pIStInterface->GetIStPort()->GetINodeMap());

					const GenICam::gcstring strInterfaceName(pIStInterface->GetIStInterfaceInfo()->GetDisplayName());
					StApi::RegisteredINodeHandle_t hRegistered = m_objIStNodeMapDisplayWndForInterfaceSpecificNodeMap->RegisterINode(pINodeMap->GetNode("Root"), strInterfaceName);
					
					//Register callback function to be notified of interface list changes.
					StApi::IStRegisteredCallbackReleasable* pIStRegisteredCallbackReleasable = NULL;
					pIStRegisteredCallbackReleasable = RegisterCallback(pIStInterface, *this, &CStViewerApp::OnStCallback_IStInterface, (void*)NULL);

					m_StRegisteredInterfaceInfo.insert(std::make_pair(pIStInterface, std::make_pair(hRegistered, pIStRegisteredCallbackReleasable)));
					pIStInterface->StartEventAcquisitionThread();

					isUpdated = true;
				}
			}
			catch (const GenICam::GenericException &e)
			{
				// If the interface XML file is bad, the exception is caught here.
				OnException(e);
			}
		}
	}

	if (mapOldStRegisteredInterfaceInfo.size())
	{
		
		for (StRegisteredInterfaceInfo_t::iterator itr = mapOldStRegisteredInterfaceInfo.begin(); itr != mapOldStRegisteredInterfaceInfo.end(); ++itr)
		{
			m_objIStNodeMapDisplayWndForInterfaceSpecificNodeMap->DeregisterINode(itr->second.first);
			itr->second.second->Release();
		}

		isUpdated = true;
	}
	if (isUpdated)
	{
		m_objIStNodeMapDisplayWndForInterfaceSpecificNodeMap->Refresh();
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int CStViewerApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

#ifdef ENABLE_INTERFACE_SPECIFIC_NODE_MAP
	for (size_t i = 0; i < m_objIStSystemPtrList.GetSize(); ++i)
	{
		IStSystem *pIStSystem = m_objIStSystemPtrList[i];
		for (size_t j = 0; j < pIStSystem->GetInterfaceCount(); ++j)
		{
			IStInterface *pIStInterface = pIStSystem->GetIStInterface(j);
			pIStInterface->StopEventAcquisitionThread();
		}
	}
#endif //ENABLE_INTERFACE_SPECIFIC_NODE_MAP

	return CWinAppEx::ExitInstance();
}

// CStViewerApp message handlers


// CAboutDlg dialog used for App About

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString strText;
	strText.Format(TEXT("StViewer, Version %s"), GCSTRING_2_LPCTSTR(GetStApiVersionText()));
	SetDlgItemText(IDC_STATIC_VERSION, strText);

	return TRUE;  // return TRUE unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CStViewerApp::OnFileConfigurationfilesetting()
{
	CConfigurationFileSettingDlg dlg;
	dlg.DoModal();
}

// CStViewerApp customization load/save methods

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerApp::PreLoadState()
{
	for (EStConfigurationFileType_t eType = StConfigurationFileType_First; eType < StConfigurationFileType_Count; eType = (EStConfigurationFileType_t)(eType + 1))
	{
		m_peConfigurationFileTarget[eType] = StConfigurationFileTarget_SameModel;
	}

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerApp::LoadCustomState()
{
	for (EStConfigurationFileType_t eType = StConfigurationFileType_First; eType < StConfigurationFileType_Count; eType = (EStConfigurationFileType_t)(eType + 1))
	{
		m_peConfigurationFileTarget[eType] = (EStConfigurationFileTarget_t)GetSectionInt(m_szConfigFileSectionName, m_pszConfigFileEntryName[eType], m_peConfigurationFileTarget[eType]);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerApp::SaveCustomState()
{
	for (EStConfigurationFileType_t eType = StConfigurationFileType_First; eType < StConfigurationFileType_Count; eType = (EStConfigurationFileType_t)(eType + 1))
	{
		WriteSectionInt(m_szConfigFileSectionName, m_pszConfigFileEntryName[eType], m_peConfigurationFileTarget[eType]);
	}
}

// CStViewerApp message handlers
//-----------------------------------------------------------------------------
//Select device and get IStDeviceReleasable pointer.
//-----------------------------------------------------------------------------
IStDeviceReleasable *CStViewerApp::CreateIStDevice()
{
	IStDeviceReleasable *pIStDeviceReleasable = NULL;

	//Get the location and size of the main window.
	CRect rectMain;
	GetMainWnd()->GetWindowRect(&rectMain);

	//Create "DeviceSelectionWnd".
	CIStDeviceSelectionWndPtr pIStDeviceSelectionWnd(CreateIStWnd(StWindowType_DeviceSelection));

	//Move the "DeviceSelectionWnd" to the center of the main window.
	int nWidth = 1280;
	int nHeight = 720;
	int nOffsetX = rectMain.left + (rectMain.Width() - nWidth) / 2;
	if(nOffsetX < 0) nOffsetX = 0;
	int nOffsetY = rectMain.top + (rectMain.Height() - nHeight) / 2;
	if(nOffsetY < 0) nOffsetY = 0;
	pIStDeviceSelectionWnd->SetPosition(nOffsetX, nOffsetY, nWidth, nHeight);

	//Specify the "IStSystem" to use.
	pIStDeviceSelectionWnd->RegisterTargetIStSystemList(m_objIStSystemPtrList);
		
	//Show the "DeviceSelectionWnd".
	pIStDeviceSelectionWnd->Show(this->GetMainWnd()->GetSafeHwnd(), StWindowMode_Modal);

	//Get selected device information.
	StApi::IStInterface *pIStInterface = NULL;
	const StApi::IStDeviceInfo *pIStDeviceInfo = NULL;
	pIStDeviceSelectionWnd->GetSelectedDeviceInfo(&pIStInterface, &pIStDeviceInfo);
		
	if(pIStDeviceInfo !=  NULL)
	{
		//Get available DeviceAccessFlag.
		DEVICE_ACCESS_FLAGS eDeviceAccessFlags = DEVICE_ACCESS_CONTROL;
		switch(pIStDeviceInfo->GetAccessStatus())
		{
		case(DEVICE_ACCESS_STATUS_READONLY):
			eDeviceAccessFlags = DEVICE_ACCESS_READONLY;
			break;
		case(DEVICE_ACCESS_STATUS_READWRITE):
			eDeviceAccessFlags = DEVICE_ACCESS_CONTROL;
			break;
		}

		//For GigE Vision switchover function
		if (eDeviceAccessFlags == DEVICE_ACCESS_CONTROL)
		{
			const uint32_t nDevCount = pIStInterface->GetDeviceCount();
			for (uint32_t nDevIndex = 0; nDevIndex < nDevCount; ++nDevIndex)
			{
				if (pIStInterface->GetIStDeviceInfo(nDevIndex) == pIStDeviceInfo)
				{
					GenApi::CIntegerPtr pInteger_DeviceSelector(pIStInterface->GetIStPort()->GetINodeMap()->GetNode("DeviceSelector"));
					if (GenApi::IsWritable(pInteger_DeviceSelector))
					{
						pInteger_DeviceSelector->SetValue(nDevIndex);

						GenApi::CIntegerPtr pInteger_SwitchoverKey(pIStInterface->GetIStPort()->GetINodeMap()->GetNode("GevApplicationSwitchoverKey"));
						if (GenApi::IsWritable(pInteger_SwitchoverKey))
						{
							//If you use switchover function for GigE Vision, please set a switchover key here.
							const uint16_t nSwitchoverKey = 0;
							pInteger_SwitchoverKey->SetValue(nSwitchoverKey);
						}
					}
					break;
				}
			}
		}

		//Create object and get IStDeviceReleasable pointer.
		GenICam::gcstring strDeviceID = pIStDeviceInfo->GetID();
		pIStDeviceReleasable = pIStInterface->CreateIStDevice(strDeviceID, eDeviceAccessFlags);
	}


	return(pIStDeviceReleasable);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerApp::LoadSaveNodeMapSettingFile(GenApi::INodeMap *pINodeMap, bool isLoad, EStConfigurationFileType_t eType, const StApi::IStDevice *pIStDevice)
{
	EStConfigurationFileTarget_t eTarget = GetConfigurationFileTarget(eType);
	TCHAR szFileName[MAX_PATH] = { TEXT('\0') };
	try
	{

		switch (eTarget)
		{
			case(StConfigurationFileTarget_All) :
				_tcscat_s(szFileName, _countof(szFileName), TEXT("All\\"));
				break;

			case(StConfigurationFileTarget_SameModel) :
			case(StConfigurationFileTarget_SameID) :
				if (pIStDevice == NULL) return;
				const StApi::IStDeviceInfo *pIStDeviceInfo = pIStDevice->GetIStDeviceInfo();
				if (pIStDeviceInfo == NULL) return;
				_tcscat_s(szFileName, _countof(szFileName), GCSTRING_2_LPCTSTR(pIStDeviceInfo->GetModel()));
				_tcscat_s(szFileName, _countof(szFileName), TEXT("\\"));
				if (eTarget == StConfigurationFileTarget_SameID)
				{
					_tcscat_s(szFileName, _countof(szFileName), GCSTRING_2_LPCTSTR(pIStDeviceInfo->GetID()));
					_tcscat_s(szFileName, _countof(szFileName), TEXT("\\"));
				}
				break;
		}
		_tcscat_s(szFileName, _countof(szFileName), GetConfigurationFileName(eType));

	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}

	//Swap ":" to "-".
	for (size_t i = 0; i < _countof(szFileName); ++i)
	{
		if (szFileName[i] == TEXT('\0'))
		{
			break;
		}
		if (szFileName[i] == TEXT(':'))
		{
			szFileName[i] = TEXT('-');
		}
	}

	mLoadSaveNodeMapSettingFile(pINodeMap, isLoad, eTarget, szFileName);

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerApp::LoadSaveNodeMapSettingFile(GenApi::INodeMap *pINodeMap, bool isLoad, EStConfigurationFileType_t eType, LPCTSTR szDeviceName)
{
	EStConfigurationFileTarget_t eTarget = GetConfigurationFileTarget(eType);
	TCHAR szFileName[MAX_PATH] = { TEXT('\0') };
	try
	{

		switch (eTarget)
		{
		case(StConfigurationFileTarget_All):
			_tcscat_s(szFileName, _countof(szFileName), TEXT("All\\"));
			break;

		case(StConfigurationFileTarget_SameModel):
		case(StConfigurationFileTarget_SameID):
			_tcscat_s(szFileName, _countof(szFileName), TEXT("StillImage\\"));
			if (eTarget == StConfigurationFileTarget_SameID)
			{
				_tcscat_s(szFileName, _countof(szFileName), szDeviceName);
				_tcscat_s(szFileName, _countof(szFileName), TEXT("\\"));
			}
			break;
		}
		_tcscat_s(szFileName, _countof(szFileName), GetConfigurationFileName(eType));

	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}

	//Swap ":" to "-".
	for (size_t i = 0; i < _countof(szFileName); ++i)
	{
		if (szFileName[i] == TEXT('\0'))
		{
			break;
		}
		if (szFileName[i] == TEXT(':'))
		{
			szFileName[i] = TEXT('-');
		}
	}

	mLoadSaveNodeMapSettingFile(pINodeMap, isLoad, eTarget, szFileName);

}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerApp::mLoadSaveNodeMapSettingFile(GenApi::INodeMap *pINodeMap, bool isLoad, EStConfigurationFileTarget_t eTarget, LPCTSTR szFileName)
{
	if ((eTarget == StConfigurationFileTarget_None) || (StConfigurationFileTarget_Count <= eTarget))
	{
		return;
	}

	// Get path of "My Document" for further usage.
	TCHAR szFullPath[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szFullPath);
	_tcscat_s(szFullPath, _countof(szFullPath), TEXT("\\StViewer\\"));
	_tcscat_s(szFullPath, _countof(szFullPath), szFileName);

	try
	{

		// Set up file name with gcstring.
		GenICam::gcstring strFileName(szFullPath);

		// Create a FeatureBag object for acquiring/saving camera settings.
		StApi::CIStFeatureBagPtr pIStFeatureBagPtr(StApi::CreateIStFeatureBag());

		if (isLoad)
		{
			if (PathFileExists(szFullPath))
			{
				// Load the settings from file to the FeatureBag.
				// *Note: we load from the one we just created above.
				pIStFeatureBagPtr->StoreFileToBag(strFileName);

				// Load the settings from the FeatureBag to the camera
				pIStFeatureBagPtr->Load(pINodeMap, true);
			}
		}
		else
		{
			// Acquire and save all current settings from INodeMap object to FeatureBag.
			pIStFeatureBagPtr->StoreNodeMapToBag(pINodeMap);


			// Save the settings in the FeatureBag to file
			pIStFeatureBagPtr->SaveToFile(strFileName);
		}
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerApp::OnStCallback_IStInterface(StApi::IStCallbackParamBase *pIStCallbackParamBase, void * /*pvContext*/)
{
	try
	{
		StApi::EStCallbackType_t eStCallbackType = pIStCallbackParamBase->GetCallbackType();
		if (eStCallbackType == StApi::StCallbackType_StApiTLEvent_InterfaceClosing)
		{
			StApi::IStCallbackParamStApiTLInterfaceClosing *pIStCallbackParamStApiTLInterfaceClosing = dynamic_cast<StApi::IStCallbackParamStApiTLInterfaceClosing*>(pIStCallbackParamBase);
			StRegisteredInterfaceInfo_t::iterator itr = m_StRegisteredInterfaceInfo.find(pIStCallbackParamStApiTLInterfaceClosing->GetIStInterface());
			if (itr != m_StRegisteredInterfaceInfo.end())
			{
				m_objIStNodeMapDisplayWndForInterfaceSpecificNodeMap->DeregisterINode(itr->second.first);
				m_StRegisteredInterfaceInfo.erase(itr);
				m_pMainWnd->SendMessageW(WM_UPDATED_INTERFACE_LIST);

			}
		}
		else if (eStCallbackType == StCallbackType_StApiTLEvent_DeviceListUpdated)
		{
			//OutputDebugString(TEXT("Device list changed.\n"));
		}
	}
	catch (...)
	{
		//TODO:
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerApp::OnStCallback_IStSystem(StApi::IStCallbackParamBase *pIStCallbackParamBase, void * /*pvContext*/)
{
	try
	{
		StApi::EStCallbackType_t eStCallbackType = pIStCallbackParamBase->GetCallbackType();
		if (eStCallbackType == StApi::StCallbackType_StApiTLEvent_InterfaceListUpdated)
		{
			m_pMainWnd->PostMessageW(WM_UPDATED_INTERFACE_LIST);
		}
	}
	catch (...)
	{
		//TODO:
	}
}