// AVIFileDlg.cpp
//

#include "stdafx.h"
#include "StViewer.h"
#include "AVIFileDlg.h"
#include "afxdialogex.h"


// CAVIFileDlg

IMPLEMENT_DYNAMIC(CAVIFileDlg, CDialogEx)

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CAVIFileDlg::CAVIFileDlg(StApi::IStVideoFiler *pIStVideoFiler, CWnd* pParent /* = NULL*/)
	: CDialogEx(CAVIFileDlg::IDD, pParent), 
	m_pIStVideoFiler(pIStVideoFiler)
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CAVIFileDlg::~CAVIFileDlg()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CAVIFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAVIFileDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ADD_FILE_NAME, &CAVIFileDlg::OnBnClickedButtonAddFileName)
END_MESSAGE_MAP()


// CAVIFileDlg

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CAVIFileDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	//Calculate the size of "NodeMapWnd".
	CRect rectClient;
	GetClientRect(&rectClient);

	{
		CRect rectFileNameBox;
		GetDlgItem(IDC_LIST_FILE_NAME)->GetWindowRect(&rectFileNameBox);
		ScreenToClient(&rectFileNameBox);
		rectClient.bottom = rectFileNameBox.top - 16;
	}

	try
	{
		// Create a data NodeMapDisplayWnd object to get the IStWndReleasable interface pointer.
		// After the NodeMapDisplayWnd object is no longer needed, call the IStWndReleasable::Release(), please discard the NodeMapDisplayWnd object.
		// In the destructor of CIStNodeMapDisplayWndPtr, IStWndReleasable::Release() is called.
		m_pIStNodeMapDisplayWnd.Reset(StApi::CreateIStWnd(StApi::StWindowType_NodeMapDisplay));

		//Register the Root node of the NodeMap of IStVideoFiler.
		m_pIStNodeMapDisplayWnd->RegisterINode(m_pIStVideoFiler->GetINodeMap()->GetNode("Root"), "");
		
		//Hide unneeded controls.
		m_pIStNodeMapDisplayWnd->SetVisibleAlphabeticMode(false);
		m_pIStNodeMapDisplayWnd->SetVisibleCollapse(false);
		m_pIStNodeMapDisplayWnd->SetVisibleExpand(false);
		m_pIStNodeMapDisplayWnd->SetVisiblePolling(false);
		m_pIStNodeMapDisplayWnd->SetVisibleRefresh(false);
		m_pIStNodeMapDisplayWnd->SetVisibleVisibility(false);
		m_pIStNodeMapDisplayWnd->SetVisibleDescription(false);

		// Sets the position and size of the window.
		m_pIStNodeMapDisplayWnd->SetPosition(0, 0, rectClient.Width(), rectClient.Height());

		//Display the window.
		m_pIStNodeMapDisplayWnd->Show(GetSafeHwnd(), StApi::StWindowMode_Child);
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}


	return TRUE;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CAVIFileDlg::OnBnClickedButtonAddFileName()
{
	GenApi::CNodeMapPtr pINodeMap(m_pIStVideoFiler->GetINodeMap());
	GenApi::CEnumerationPtr pIEnumeration_FileFormat(pINodeMap->GetNode("FileFormat"));
	GenApi::CEnumEntryPtr pIEnumEntry_WebM(pIEnumeration_FileFormat->GetEntryByName("WebM"));
	const bool isWebMSupported = GenApi::IsImplemented(pIEnumEntry_WebM);
	const bool isWebM = isWebMSupported && (pIEnumEntry_WebM->GetValue() == pIEnumeration_FileFormat->GetIntValue());

	CString strFilter;
	if (isWebMSupported)
	{
		if (isWebM)
		{
			strFilter = TEXT("WebM File(*.webm)|*.webm|AVI File(*.avi)|*.avi|All Files(*.*)|*.*||");
		}
		else
		{
			strFilter = TEXT("AVI File(*.avi)|*.avi|WebM File(*.webm)|*.webm|All Files(*.*)|*.*||");
		}
	}
	else
	{
		strFilter = TEXT("AVI File(*.avi)|*.avi|All Files(*.*)|*.*||");
	}
	CString strDefExt = isWebM ? TEXT("webm") : TEXT("avi");
	CFileDialog dlg(FALSE, strDefExt, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
	if(dlg.DoModal() !=  IDOK) return;

	//Add file name to the avi file name list.
	LPCTSTR szFileName = dlg.m_ofn.lpstrFile;
	GenICam::gcstring strFileName(szFileName);
	m_pIStVideoFiler->RegisterFileName(strFileName);

	//Add file name to the ListBox
	CListBox objListBox;
	objListBox.Attach(GetDlgItem(IDC_LIST_FILE_NAME)->GetSafeHwnd());
	objListBox.AddString(szFileName);
	objListBox.Detach();

	//Enable ok button.
	GetDlgItem(IDOK)->EnableWindow(TRUE);
}
