
// StViewerView.cpp : implementation of the CStViewerView class
//

#include "stdafx.h"

#include "StViewer.h"


#include "StViewerDocBase.h"
#include "StViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CStViewerView

IMPLEMENT_DYNCREATE(CStViewerView, CView)

BEGIN_MESSAGE_MAP(CStViewerView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()

	ON_MESSAGE(WM_DEVICE_LOST, OnDeviceLost)
END_MESSAGE_MAP()

// CStViewerView construction/destruction

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStViewerView::CStViewerView()
{
	// TODO: add construction code here
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStViewerView::~CStViewerView()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CStViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CStViewerView drawing

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerView::OnDraw(CDC* /*pDC*/)
{
	CStViewerDocBase* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}



// CStViewerView diagnostics

#ifdef _DEBUG
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerView::AssertValid() const
{
	CView::AssertValid();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStViewerDocBase* CStViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CStViewerDocBase)));
	return (CStViewerDocBase*)m_pDocument;
}
#endif //_DEBUG


// CStViewerView message handlers

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	CRect rectClient;
	GetClientRect(&rectClient);

	//Adjust the size of "DisplayImageWnd".
	AdjustLayout();

	//Show "DisplayImageWnd".
	
	GetDocument()->OnInitDisplayImageWnd(GetSafeHwnd());
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	//Adjust the size of "DisplayImageWnd".
	AdjustLayout();
}


//-----------------------------------------------------------------------------
//Adjust the size of "DisplayImageWnd".
//-----------------------------------------------------------------------------
void CStViewerView::AdjustLayout()
{
	CRect rectClient;
	GetClientRect(&rectClient);
	GetDocument()->OnDisplayImageWndResize(GetSafeHwnd(), rectClient);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
LRESULT CStViewerView::OnDeviceLost(WPARAM wParam, LPARAM /*lParam*/)
{
	CStViewerDocBase *pCStViewerDocBase = (CStViewerDocBase*)wParam;
	pCStViewerDocBase->AddDeviceLog(IDS_DEVICE_LOST);
	AfxMessageBox(TEXT("Device lost."));
	GetParent()->PostMessageW(WM_CLOSE);
	return(0);
}

