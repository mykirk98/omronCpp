
// MFCSingleDocViewerView.cpp : implementation of the CMFCSingleDocViewerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFCSingleDocViewer.h"
#endif

#include "MFCSingleDocViewerDoc.h"
#include "MFCSingleDocViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCSingleDocViewerView

IMPLEMENT_DYNCREATE(CMFCSingleDocViewerView, CScrollView)

BEGIN_MESSAGE_MAP(CMFCSingleDocViewerView, CScrollView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_MESSAGE(WM_DEVICE_LOST, OnDeviceLost)
	ON_MESSAGE(WM_NEW_IMAGE, OnNewImage)

	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_FILE_SAVE_AS, &CMFCSingleDocViewerView::OnFileSaveAs)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//CMFCSingleDocViewerView construction
//-----------------------------------------------------------------------------
CMFCSingleDocViewerView::CMFCSingleDocViewerView() : m_isNewImage(false), m_nTimerID(0)
{
	// TODO: add construction code here

	CSize sizeImage(1, 1);
	SetScrollSizes(MM_TEXT, sizeImage);
}

//-----------------------------------------------------------------------------
//CMFCSingleDocViewerView destruction
//-----------------------------------------------------------------------------
CMFCSingleDocViewerView::~CMFCSingleDocViewerView()
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerView::OnDraw(CDC* pDC)
{
	CMFCSingleDocViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
	if (m_objCBitmap.m_hObject)
	{
		CDC dcBitmap;
		dcBitmap.CreateCompatibleDC(pDC);

		HGDIOBJ hOldBitmap = dcBitmap.SelectObject(m_objCBitmap.GetSafeHandle());
		CSize sizeSrc = m_objCBitmap.GetBitmapDimension();
		pDC->BitBlt(0, 0, sizeSrc.cx, sizeSrc.cy, &dcBitmap, 0, 0, SRCCOPY);
		//pDC->StretchBlt(0, 0, sizeSrc.cx, sizeSrc.cy, &dcBitmap, 0, 0, sizeSrc.cx, sizeSrc.cy, SRCCOPY);
		dcBitmap.SelectObject(hOldBitmap);
	}

	FillOutsideRect(pDC, &m_objBackgroundBrush);
}

// CMFCSingleDocViewerView diagnostics

#ifdef _DEBUG
void CMFCSingleDocViewerView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CMFCSingleDocViewerView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CMFCSingleDocViewerDoc* CMFCSingleDocViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCSingleDocViewerDoc)));
	return (CMFCSingleDocViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CMFCSingleDocViewerView message handlers
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
LRESULT CMFCSingleDocViewerView::OnDeviceLost(WPARAM wParam, LPARAM /*lParam*/)
{
	AfxMessageBox(TEXT("Device lost."));
	GetDocument()->CloseDevice();
	Invalidate();
	return(0);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
LRESULT CMFCSingleDocViewerView::OnNewImage(WPARAM wParam, LPARAM lParam)
{
	m_isNewImage = true;
	return(0);
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerView::OnTimer(UINT_PTR nIDEvent)
{
	if (m_isNewImage)
	{
		CMFCSingleDocViewerDoc* pDoc = GetDocument();
		ASSERT(pDoc != NULL);

		if (m_objCBitmap.m_hObject)
		{
			m_objCBitmap.DeleteObject();
		}
		if (pDoc->GetBitmapImage(GetDC(), m_objCBitmap))
		{
			m_isNewImage = false;

			CSize sizeImage(m_objCBitmap.GetBitmapDimension());
			if ((sizeImage.cx == 0) || (sizeImage.cy == 0))
			{
				sizeImage.cx = 1;
				sizeImage.cy = 1;
			}
			SetScrollSizes(MM_TEXT, sizeImage);

			Invalidate();
		}
	}
	CScrollView::OnTimer(nIDEvent);
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int CMFCSingleDocViewerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_nTimerID = SetTimer(m_nTimerID, 33, NULL);

	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerView::OnDestroy()
{
	CScrollView::OnDestroy();

	if (m_nTimerID)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CMFCSingleDocViewerView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	if (m_objBackgroundBrush.m_hObject == NULL)
	{
		m_objBackgroundBrush.CreateHatchBrush(HS_DIAGCROSS, RGB(192, 192, 192));
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerView::OnFileSaveAs()
{
	StApi::CIStImageBufferPtr pIStImageBuffer(StApi::CreateIStImageBuffer());
	if (!GetDocument()->GetLatestImage(pIStImageBuffer)) return;


	typedef struct
	{
		StApi::EStStillImageFileFormat_t eFileType;
		LPCTSTR szExt;
		LPCTSTR szFilter;
	} SFILE_TYPE;

	const SFILE_TYPE psFileTypeList[] = {
		{ StApi::StStillImageFileFormat_Bitmap, TEXT(".bmp"), TEXT("Bitmap File(*.bmp)|*.bmp") },
		{ StApi::StStillImageFileFormat_JPEG, TEXT(".jpg"), TEXT("Jpeg File(*.jpg)|*.jpg") },
		{ StApi::StStillImageFileFormat_TIFF, TEXT(".tif"), TEXT("Tiff File(*.tif)|*.tif") },
		{ StApi::StStillImageFileFormat_PNG, TEXT(".png"), TEXT("PNG File(*.png)|*.png") },
		{ StApi::StStillImageFileFormat_CSV, TEXT(".csv"), TEXT("CSV File(*.csv)|*.csv") },
		{ StApi::StStillImageFileFormat_StApiRaw, TEXT(".straw"), TEXT("StRaw File(*.straw)|*.straw") },
	};

	{
		StApi::IStImage *pIStImage(pIStImageBuffer->GetIStImage());
		const StApi::EStPixelFormatNamingConvention_t ePixelFormat = pIStImage->GetImagePixelFormat();

		StApi::CIStStillImageFilerPtr pIStStillImageFiler(StApi::CreateIStFiler(StApi::StFilerType_StillImage));
		CString strDefExt;
		CString strFilter;
		std::vector <StApi::EStStillImageFileFormat_t> vecFormatList;
		for (size_t i = 0; i < _countof(psFileTypeList); ++i)
		{
			const SFILE_TYPE *pFileType = &psFileTypeList[i];
			if (pIStStillImageFiler->IsSaveSupported(ePixelFormat, pFileType->eFileType))
			{
				if (strDefExt.IsEmpty())
				{
					strDefExt = pFileType->szExt;
				}
				strFilter.Append(pFileType->szFilter);
				strFilter.AppendChar(TEXT('|'));
				vecFormatList.push_back(pFileType->eFileType);
			}
		}
		strFilter.AppendChar(TEXT('|'));

		CFileDialog dlg(FALSE, strDefExt, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
		dlg.m_ofn.lpstrTitle = TEXT("Save Still Image File");
		if (dlg.DoModal() == IDOK)
		{
			CString strFileName(dlg.m_ofn.lpstrFile);
			CString strExt(PathFindExtension(dlg.m_ofn.lpstrFile));
			StApi::EStStillImageFileFormat_t nFileType = vecFormatList[dlg.m_ofn.nFilterIndex - 1];
			for (size_t i = 0; i < _countof(psFileTypeList); ++i)
			{
				const SFILE_TYPE *pFileType = &psFileTypeList[i];
				if (strExt.CompareNoCase(pFileType->szExt) == 0)
				{
					nFileType = pFileType->eFileType;
					break;
				}
			}

			GenICam::gcstring strgcFileName(strFileName);
			pIStStillImageFiler->Save(pIStImage, nFileType, strgcFileName);

		}
	}

}
