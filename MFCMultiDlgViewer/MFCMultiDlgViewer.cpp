
// MFCMultiDlgViewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MFCMultiDlgViewer.h"
#include "MFCMultiDlgViewerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace GenTL;
using namespace StApi;

// CMFCMultiDlgViewerApp
BEGIN_MESSAGE_MAP(CMFCMultiDlgViewerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// CMFCMultiDlgViewerApp construction
//-----------------------------------------------------------------------------
CMFCMultiDlgViewerApp::CMFCMultiDlgViewerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CMFCMultiDlgViewerApp object
CMFCMultiDlgViewerApp theApp;

//-----------------------------------------------------------------------------
// CMFCMultiDlgViewerApp initialization
//-----------------------------------------------------------------------------
BOOL CMFCMultiDlgViewerApp::InitInstance()
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

	CWinApp::InitInstance();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("SentechSDK\\Sampes\\MFCMultiDlgViewer"));


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

	CMFCMultiDlgViewerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
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
//Select device and get IStDeviceReleasable pointer.
//-----------------------------------------------------------------------------
IStDeviceReleasable *CMFCMultiDlgViewerApp::CreateIStDevice()
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