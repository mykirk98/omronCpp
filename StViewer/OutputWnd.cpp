
#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define COLUMN_TIME		0
#define COLUMN_SOURCE	1
#define COLUMN_LOG		2
#define COLUMN_COUNT	3

/////////////////////////////////////////////////////////////////////////////
// COutputBar

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
COutputWnd::COutputWnd()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct)  ==  -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create tabs window:
	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
	{
		TRACE0("Failed to create output tab window\n");
		return -1;      // fail to create
	}

	// Create output panes:
	const DWORD dwStyle = LVS_REPORT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;


	if (!m_wndOutputLog.Create(dwStyle, rectDummy, &m_wndTabs, 2))
	{
		TRACE0("Failed to create output windows\n");
		return -1;      // fail to create
	}
	DWORD dwExStyle = ListView_GetExtendedListViewStyle(m_wndOutputLog.m_hWnd);
	dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	ListView_SetExtendedListViewStyle(m_wndOutputLog.m_hWnd, dwExStyle);

	UpdateFonts();

	CString strTabName;
	BOOL bNameValid;

	// Attach list windows to tab:
	bNameValid = strTabName.LoadString(IDS_LOG_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputLog, strTabName, (UINT)0);

	//Set list titles.
	m_wndOutputLog.InsertColumn(COLUMN_TIME, TEXT("Time"), LVCFMT_LEFT, 150);
	m_wndOutputLog.InsertColumn(COLUMN_SOURCE, TEXT("Source"), LVCFMT_LEFT, 150);
	m_wndOutputLog.InsertColumn(COLUMN_LOG, TEXT("Log"), LVCFMT_LEFT, 500);

	AddLog(AfxGetApp()->m_pszAppName, TEXT("Started"));

	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void COutputWnd::AddLog(LPCTSTR szSource, LPCTSTR szLog)
{
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);

	CString strTime;
	strTime.Format(TEXT("%04u/%02u/%02u %02u:%02u:%02u.%03u"), sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond, sTime.wMilliseconds);

	int nItem = m_wndOutputLog.InsertItem(m_wndOutputLog.GetItemCount(), strTime);
	m_wndOutputLog.SetItemText(nItem, COLUMN_SOURCE, szSource);
	m_wndOutputLog.SetItemText(nItem, COLUMN_LOG, szLog);

	const size_t nMaximumItemCount = m_wndOutputLog.GetMaximumItemCount();
	for (;;)
	{
		const int nItemCount = m_wndOutputLog.GetItemCount();
		if (nItemCount <= 0) break;
		if ((size_t)nItemCount <= nMaximumItemCount) break;
		m_wndOutputLog.DeleteItem(0);
	}

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// Tab control should cover the whole client area:
	m_wndTabs.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void COutputWnd::UpdateFonts()
{
	m_wndOutputLog.SetFont(&afxGlobalData.fontRegular);
}

/////////////////////////////////////////////////////////////////////////////
// COutputList1

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
COutputList::COutputList(size_t nMaximumItemCount) : m_nMaximumItemCount(nMaximumItemCount)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
COutputList::~COutputList()
{
}

BEGIN_MESSAGE_MAP(COutputList, CListCtrl)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList message handlers


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_OUTPUT_POPUP);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		reinterpret_cast<CMDIFrameWndEx*>(AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void COutputList::OnEditCopy()
{

	CString strText;
	for (int nItem = 0; nItem < GetItemCount(); ++nItem)
	{
		for (int nSubItem = 0; nSubItem <= COLUMN_COUNT; ++nSubItem)
		{
			if (0 < nSubItem)
			{
				strText.AppendChar(TEXT('\t'));
			}
			strText.Append(GetItemText(nItem, nSubItem));
		}
		strText.AppendChar(TEXT('\n'));
	}

	if (!OpenClipboard())
	{
		AfxMessageBox(_T("Cannot open the Clipboard"));
		return;
	}
	// Remove the current Clipboard contents
	if (!EmptyClipboard())
	{
		CloseClipboard();
		AfxMessageBox(_T("Cannot empty the Clipboard"));
		return;
	}
	// Get the currently selected data
	const size_t nSize = strText.GetAllocLength();
	HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, nSize * sizeof(TCHAR));
	if (hGlob == NULL)
	{
		CloseClipboard();
		return;
	}
	_tcscpy_s(reinterpret_cast<LPTSTR>(hGlob), nSize, strText);
	// For the appropriate data formats...
#if _UNICODE
	const UINT nFormat = CF_UNICODETEXT;
#else
	const UINT nFormat = CF_TEXT;
#endif
	if (::SetClipboardData(nFormat, hGlob) == NULL)
	{
		CString msg;
		msg.Format(_T("Unable to set Clipboard data, error: %lu"), GetLastError());
		AfxMessageBox(msg);
		CloseClipboard();
		GlobalFree(hGlob);
		return;
	}
	CloseClipboard();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void COutputList::OnEditClear()
{
	DeleteAllItems();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void COutputList::OnViewOutput()
{
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	if (pMainFrame !=  NULL && pParentBar !=  NULL)
	{
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();

	}
}
