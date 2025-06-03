
// MFCSingleDocViewer.cpp : Defines the class behaviors for the application.
//
#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "MFCSingleDocViewer.h"
#include "MainFrm.h"

#include "MFCSingleDocViewerDoc.h"
#include "MFCSingleDocViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace GenTL;
using namespace StApi;

// CMFCSingleDocViewerApp

BEGIN_MESSAGE_MAP(CMFCSingleDocViewerApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CMFCSingleDocViewerApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	ON_COMMAND(ID_CAMERA_OPEN, &CMFCSingleDocViewerApp::OnCameraOpen)
END_MESSAGE_MAP()


// CMFCSingleDocViewerApp construction

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CMFCSingleDocViewerApp::CMFCSingleDocViewerApp()
{
	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("MFCSingleDocViewer.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CMFCSingleDocViewerApp object
CMFCSingleDocViewerApp theApp;

//-----------------------------------------------------------------------------
// CMFCSingleDocViewerApp initialization
//-----------------------------------------------------------------------------
BOOL CMFCSingleDocViewerApp::InitInstance()
{
	CWinAppEx::InitInstance();
	EnableTaskbarInteraction(FALSE);

	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("SentechSDK\\Sampes\\MFCSingleDocViewer"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	//Create object and initialize "IStSystemPtr" list.
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
		}
		catch (const GenICam::GenericException &e)
		{
			if (eStSystemVendor == StSystemVendor_Default)
			{
				OnException(e);
			}
		}
	}

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMFCSingleDocViewerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMFCSingleDocViewerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}

//-----------------------------------------------------------------------------
// CAboutDlg dialog used for App About
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

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
// App command to run the dialog
void CMFCSingleDocViewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OnException(const GenICam::GenericException &e)
{
	//Get the exception contents.
	GenICam::gcstring strSourceFileName(e.GetSourceFileName());
	const unsigned int iSourceLine = e.GetSourceLine();
	GenICam::gcstring strDescription(e.GetDescription());

	//Make message string.
	CString strMessage;
	strMessage.Append(GCSTRING_2_LPCTSTR(strSourceFileName));
	strMessage.AppendFormat(TEXT("[%u]\r\n"), iSourceLine);
	strMessage.Append(GCSTRING_2_LPCTSTR(strDescription));

	//Show message box.
	AfxMessageBox(strMessage, MB_OK | MB_ICONEXCLAMATION);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerApp::OnCameraOpen()
{

	// We only have one doc template. Get a pointer to it.
	POSITION pos = GetFirstDocTemplatePosition();
	CDocTemplate* pDocTemplate = GetNextDocTemplate(pos);

	CDocument* pDoc = NULL;
	CMFCSingleDocViewerDoc *pDocDevice = NULL;
	if (0 < GetOpenDocumentCount())
	{
		pos = pDocTemplate->GetFirstDocPosition();
		pDoc = pDocTemplate->GetNextDoc(pos);
		pDocDevice = dynamic_cast<CMFCSingleDocViewerDoc*>(pDoc);
		if (pDocDevice)
		{
			pDocDevice->CloseDevice();
		}
	}
	else
	{
		pDoc = pDocTemplate->CreateNewDocument();
		pDocDevice = dynamic_cast<CMFCSingleDocViewerDoc*>(pDoc);
	}
	if (pDocDevice == NULL)
	{
		CString strErrorMsg;
		strErrorMsg.Format(_T("Could not open a camera."));
		AfxMessageBox(strErrorMsg);
	}

	pDocDevice->OpenDevice(CreateIStDevice());

}
//-----------------------------------------------------------------------------
//Select device and get IStDeviceReleasable pointer.
//-----------------------------------------------------------------------------
IStDeviceReleasable *CMFCSingleDocViewerApp::CreateIStDevice()
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
	if (nOffsetX < 0) nOffsetX = 0;
	int nOffsetY = rectMain.top + (rectMain.Height() - nHeight) / 2;
	if (nOffsetY < 0) nOffsetY = 0;
	pIStDeviceSelectionWnd->SetPosition(nOffsetX, nOffsetY, nWidth, nHeight);

	//Specify the "IStSystem" to use.
	pIStDeviceSelectionWnd->RegisterTargetIStSystemList(m_objIStSystemPtrList);

	//Show the "DeviceSelectionWnd".
	pIStDeviceSelectionWnd->Show(this->GetMainWnd()->GetSafeHwnd(), StWindowMode_Modal);

	//Get selected device information.
	StApi::IStInterface *pIStInterface = NULL;
	const StApi::IStDeviceInfo *pIStDeviceInfo = NULL;
	pIStDeviceSelectionWnd->GetSelectedDeviceInfo(&pIStInterface, &pIStDeviceInfo);

	if (pIStDeviceInfo != NULL)
	{
		//Get available DeviceAccessFlag.
		DEVICE_ACCESS_FLAGS eDeviceAccessFlags = DEVICE_ACCESS_CONTROL;
		switch (pIStDeviceInfo->GetAccessStatus())
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