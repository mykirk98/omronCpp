// CStillImageFilesDlg.cpp
//

#include "stdafx.h"
#include "StViewer.h"
#include "CStillImageFilesDlg.h"
#include "afxdialogex.h"


// CStillImageFilesDlg

IMPLEMENT_DYNAMIC(CStillImageFilesDlg, CDialogEx)

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStillImageFilesDlg::CStillImageFilesDlg(StApi::IStStillImageFiler *pIStStillImageFiler, StApi::IStPixelFormatConverter *pIStPixelFormatConverter, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_STILL_IMAGE_FILES, pParent)
	, m_pIStStillImageFiler(pIStStillImageFiler)
	, m_pIStPixelFormatConverter(pIStPixelFormatConverter)
	, m_strStillImageFilesPath(_T(""))
	, m_strStillImageFilesPattern(_T(""))
	, m_nMaximumImageFileCount(100)
	, m_nSaveAImagePer(1)
	, m_nFileType(0)
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStillImageFilesDlg::~CStillImageFilesDlg()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStillImageFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_STILL_IMAGE_FILES_PATH, m_strStillImageFilesPath);
	DDX_Text(pDX, IDC_EDIT_STILL_IMAGE_FILES_NAME_PATTERN, m_strStillImageFilesPattern);
	DDX_Text(pDX, IDC_EDIT_SAVE_IMAGE_FILE_COUNT, m_nMaximumImageFileCount);
	DDX_Text(pDX, IDC_EDIT_SAVE_A_IMAGE_PER, m_nSaveAImagePer);
	DDX_CBIndex(pDX, IDC_COMBO_FILE_TYPE, m_nFileType);
}


BEGIN_MESSAGE_MAP(CStillImageFilesDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_STILL_IMAGE_FILES_PATH, &CStillImageFilesDlg::OnBnClickedButtonStillImageFilesPath)
END_MESSAGE_MAP()


// CStillImageFilesDlg

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM /*lParam*/, LPARAM lpData)
{
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		SendMessage(hWnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
		break;
	case BFFM_SELCHANGED:
		break;
	}
	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStillImageFilesDlg::OnBnClickedButtonStillImageFilesPath()
{
	LPMALLOC pMalloc;
	if (SUCCEEDED(SHGetMalloc(&pMalloc)))
	{
		TCHAR szPath[MAX_PATH] = { TEXT('\0') };
		BROWSEINFO browsInfo;
		UpdateData(TRUE);
		memset(&browsInfo, NULL, sizeof(browsInfo));

		browsInfo.hwndOwner = GetSafeHwnd();
		browsInfo.pidlRoot = NULL;
		browsInfo.pszDisplayName = szPath;
		browsInfo.lpszTitle = L"Still image files path";
		browsInfo.ulFlags = BIF_RETURNONLYFSDIRS;
		browsInfo.lpfn = &BrowseCallbackProc;
		browsInfo.lParam = (LPARAM)(LPCTSTR)m_strStillImageFilesPath;
		browsInfo.iImage = (int)NULL;

		LPITEMIDLIST pIDlist = SHBrowseForFolder(&browsInfo);
		if (NULL != pIDlist)
		{
			//Get full path name
			SHGetPathFromIDList(pIDlist, szPath);
			pMalloc->Free(pIDlist);
			m_strStillImageFilesPath = szPath;
			UpdateData(FALSE);
		}
		pMalloc->Release();
	}
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CStillImageFilesDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	//Calculate the size of "NodeMapWnd".
	CRect rectClient;
	GetClientRect(&rectClient);

	{
		CRect rectButton;
		GetDlgItem(IDC_COMBO_FILE_TYPE)->GetWindowRect(&rectButton);
		ScreenToClient(&rectButton);
		rectClient.bottom = rectButton.top -16;
	}

	try
	{
		// Create a data NodeMapDisplayWnd object to get the IStWndReleasable interface pointer.
		// After the NodeMapDisplayWnd object is no longer needed, call the IStWndReleasable::Release(), please discard the NodeMapDisplayWnd object.
		// In the destructor of CIStNodeMapDisplayWndPtr, IStWndReleasable::Release() is called.
		m_pIStNodeMapDisplayWnd.Reset(StApi::CreateIStWnd(StApi::StWindowType_NodeMapDisplay));

		//Register the Root node of the NodeMap of IStVideoFiler.
		GenApi::CNodeMapPtr pINodeMap_PixelFormatConverter(m_pIStPixelFormatConverter->GetINodeMap());
		m_pIStNodeMapDisplayWnd->RegisterINode(pINodeMap_PixelFormatConverter->GetNode("DestinationPixelFormat"), "Pixel Format Converter");
		m_pIStNodeMapDisplayWnd->RegisterINode(pINodeMap_PixelFormatConverter->GetNode("BayerInterpolationMethod"), "Pixel Format Converter");
		GenApi::CNodeMapPtr pINodeMap_StillImageFiler(m_pIStStillImageFiler->GetINodeMap());
		m_pIStNodeMapDisplayWnd->RegisterINode(pINodeMap_StillImageFiler->GetNode("Quality"), "JPEG");
		GenApi::CIntegerPtr pIInteger_CompressionLevel(pINodeMap_StillImageFiler->GetNode("CompressionLevel"));
		if (pIInteger_CompressionLevel.IsValid())
		{
			m_pIStNodeMapDisplayWnd->RegisterINode(pIInteger_CompressionLevel->GetNode(), "PNG");
		}

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

	return TRUE;  // return TRUE unless you set the focus to a control
}
