
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "StViewer.h"

#include "MainFrm.h"

#include "StViewerDocBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_WM_SETTINGCHANGE()
	ON_WM_TIMER()
	ON_MESSAGE(WM_ADD_LOG, OnAddLog)
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE(AFX_WM_RESETTOOLBAR, OnResetToolbar)
#ifdef ENABLE_INTERFACE_SPECIFIC_NODE_MAP
	ON_COMMAND(ID_VIEW_INTERFACENODE, &CMainFrame::OnViewInterfacenode)
	ON_COMMAND(ID_VIEW_SYSTEMNODE, &CMainFrame::OnViewSystemnode)
	ON_MESSAGE(WM_UPDATED_INTERFACE_LIST, &CMainFrame::OnUpdatedInterfaceList)
#endif //ENABLE_INTERFACE_SPECIFIC_NODE_MAP
	ON_COMMAND(ID_WINDOW_FULLSCREEN, &CMainFrame::OnViewFullscreen)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

static UINT indicators[]  = 
{
	ID_SEPARATOR,           // status line indicator
	IDS_STATUS_PIXEL_INFO, 
	IDS_STATUS_RECEIVED_IMAGE_COUNT,
	IDS_STATUS_FPS,
};

//-----------------------------------------------------------------------------
// CMainFrame construction/destruction
//-----------------------------------------------------------------------------
CMainFrame::CMainFrame()
	: m_nTimerID(0)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CMainFrame::~CMainFrame()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct)  ==  -1)
		return -1;

	BOOL bNameValid;

	//Title
	CString strText;
	strText.Format(TEXT("StViewer (Version %s)"), GCSTRING_2_LPCTSTR(StApi::GetStApiVersionText()));
	SetTitle(strText);

	// MDI tabs
	EnableMDITabs(TRUE, FALSE, CMFCBaseTabCtrl::LOCATION_TOP, TRUE, CMFCTabCtrl::STYLE_3D_SCROLLED, FALSE, TRUE);
	GetMDITabs().EnableAutoColor(TRUE);

	// Menu bar
	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	// Tool bar
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, IDS_TOOLBAR_CUSTOMIZE);
	
	//Advanced toolbar
	if (!m_wndToolBarAdvanced.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | /*WS_VISIBLE |*/ CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(1,1,1,1), IDR_MAINFRAME_ADVANCED) ||
		!m_wndToolBarAdvanced.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_ADVANCED256 : IDR_MAINFRAME_ADVANCED))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	CString strToolBarAdvancedName;
	bNameValid = strToolBarAdvancedName.LoadString(IDS_TOOLBAR_ADVANCED);
	ASSERT(bNameValid);
	m_wndToolBarAdvanced.SetWindowText(strToolBarAdvancedName);
	m_wndToolBarAdvanced.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, IDS_TOOLBAR_CUSTOMIZE);

	//Drawing toolbar
	if (!m_wndToolBarDrawing.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | /*WS_VISIBLE |*/ CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(1, 1, 1, 1), IDR_TOOLBAR_DRAWING) || !m_wndToolBarDrawing.LoadToolBar(theApp.m_bHiColorIcons ? IDR_TOOLBAR_DRAWING_256 : IDR_TOOLBAR_DRAWING))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	CString strToolBarDrawingName;
	bNameValid = strToolBarDrawingName.LoadString(IDS_TOOLBAR_DRAWING);
	ASSERT(bNameValid);
	m_wndToolBarDrawing.SetWindowText(strToolBarDrawingName);
	m_wndToolBarDrawing.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, IDS_TOOLBAR_CUSTOMIZE);




	// Allow user-defined toolbars operations:
	//HKEY_CURRENT_USER\Software\Local AppWizard-Generated Applications\StViewer
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	// Status bar
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarAdvanced.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarDrawing.EnableDocking(CBRS_ALIGN_ANY);
	

	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	DockPane(&m_wndToolBarAdvanced);
	DockPane(&m_wndToolBarDrawing);


	// set the visual manager and style based on persisted value
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Output window
	CString strOutputWnd;
	bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
	ASSERT(bNameValid);
	if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Output window\n");
		return FALSE; // failed to create
	}

	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(theApp.m_bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	UpdateMDITabbedBarsIcons();

	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutput);


	// Enable enhanced windows management dialog
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);

	// Enable toolbar and docking window menu replacement
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	EnableFullScreenMode(ID_WINDOW_FULLSCREEN);

	// Switch the order of document name and application name on the window title bar. This
	// improves the usability of the taskbar because the document name is visible with the thumbnail.
	ModifyStyle(0, FWS_PREFIXTITLE);
	
	//Start timer that updates status bar.
	m_nTimerID = SetTimer(1, 500, NULL);

	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.dwExStyle |= WS_EX_ACCEPTFILES;

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();


	{
		CMFCToolBarComboBoxButton btnComboBox(ID_H_GRID_COUNT, GetCmdMgr()->GetCmdImage(ID_H_GRID_COUNT), CBS_DROPDOWNLIST);
		InitializeGridLineComboBox(btnComboBox, TEXT("H Line:"));
		pDlgCust->ReplaceButton(ID_H_GRID_COUNT, btnComboBox);
	}
	{
		CMFCToolBarComboBoxButton btnComboBox(ID_V_GRID_COUNT, GetCmdMgr()->GetCmdImage(ID_V_GRID_COUNT), CBS_DROPDOWNLIST);
		InitializeGridLineComboBox(btnComboBox, TEXT("V Line:"));
		pDlgCust->ReplaceButton(ID_V_GRID_COUNT, btnComboBox);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres  ==  0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, IDS_TOOLBAR_CUSTOMIZE);
	return lres;
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}
	

	// enable customization button for all user toolbars
	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar !=  NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, IDS_TOOLBAR_CUSTOMIZE);
		}
	}

	return TRUE;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CMDIFrameWndEx::OnSettingChange(uFlags, lpszSection);
	m_wndOutput.UpdateFonts();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMainFrame::UpdateStatusBar()
{
	if (!m_wndStatusBar.IsVisible())
	{
		return;
	}
	try
	{
		CMDIChildWnd *pCMDIChildWnd = this->MDIGetActive();
		if (pCMDIChildWnd)
		{
			CStViewerDocBase *pCStViewerDocBase = dynamic_cast<CStViewerDocBase*>(pCMDIChildWnd->GetActiveDocument());
			if (pCStViewerDocBase)
			{
				const UINT pnID[] = {
					IDS_STATUS_RECEIVED_IMAGE_COUNT,	//Received Image Count
					IDS_STATUS_FPS,						//FPS
					IDS_STATUS_PIXEL_INFO				//Pixel Information
				};
				for (size_t nIndex = 0; nIndex < _countof(pnID); ++nIndex)
				{
					const int nCtrlIndex = this->m_wndStatusBar.CommandToIndex(pnID[nIndex]);
					if (0 <= nCtrlIndex)
					{
						CString strText;
						pCStViewerDocBase->GetStatusBarText(nIndex, strText);
						m_wndStatusBar.SetPaneText(nCtrlIndex, strText);
						m_wndStatusBar.SetTipText(nCtrlIndex, strText);
					}
				}
			}
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
void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	if(m_nTimerID  ==  nIDEvent)
	{
		//Update status bar.
		UpdateStatusBar();

		//Dealing with the phenomenon of buttons being held down under high load.
		CMFCToolBar * const pCMFCToolBars[] = { &m_wndToolBar , &m_wndToolBarDrawing };
		for (size_t i = 0; i < _countof(pCMFCToolBars); ++i)
		{
			if (pCMFCToolBars[i]->IsVisible())
			{
				const UINT nCount = pCMFCToolBars[i]->GetCount();
				for (UINT j = 0; j < nCount; ++j)
				{
					pCMFCToolBars[i]->UpdateButton(j);
				}
			}
		}
	}
	else
	{
		CMDIFrameWndEx::OnTimer(nIDEvent);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
LRESULT CMainFrame::OnAddLog(WPARAM wp, LPARAM lp)
{
	LPCTSTR szSource = (LPCTSTR)wp;
	LPCTSTR szLog = (LPCTSTR)lp;

	m_wndOutput.AddLog(szSource, szLog);

	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIFrameWndEx::OnSize(nType, cx, cy);


	if (m_wndStatusBar)
	{

		LPCTSTR szSampleText = TEXT("Received = 00000000[Dropped = 00000000]");
		SIZE sDipsText;
		GetTextExtentPoint32(m_wndStatusBar.GetDC()->GetSafeHdc(), szSampleText, static_cast<int>(_tcslen(szSampleText)), &sDipsText);
		const int nMaxSmallWidth = sDipsText.cx;
		
		int nSmallWidth = cx / 4;
		if (nMaxSmallWidth < nSmallWidth)
		{
			nSmallWidth = nMaxSmallWidth;
		}
		const int nWideWidth = (cx - nSmallWidth * 2) / 2;
		
		const UINT pnID[] = { 0 };
		m_wndStatusBar.SetIndicators(pnID, 1);
		m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));

		for (int i = 0; i < 4; ++i)
		{
			int nWidth = nSmallWidth;
			switch (indicators[i])
			{
			case(IDS_STATUS_PIXEL_INFO):
			case(ID_SEPARATOR):
				nWidth = nWideWidth;
				break;
			}
			m_wndStatusBar.SetPaneWidth(i, nWidth);
			m_wndStatusBar.SetPaneStyle(i, m_wndStatusBar.GetPaneStyle(i) | CBRS_TOOLTIPS);
		}
		//Update status bar.
		UpdateStatusBar();
	}
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
LRESULT CMainFrame::OnResetToolbar(WPARAM wp, LPARAM /*lp*/)
{
	if ((wp == IDR_TOOLBAR_DRAWING) || (wp == IDR_TOOLBAR_DRAWING_256))
	{
		{
			CMFCToolBarComboBoxButton btnComboBox(ID_H_GRID_COUNT, GetCmdMgr()->GetCmdImage(ID_H_GRID_COUNT), CBS_DROPDOWNLIST);
			InitializeGridLineComboBox(btnComboBox, TEXT("H Line:"));
			m_wndToolBarDrawing.ReplaceButton(ID_H_GRID_COUNT, btnComboBox);
		}
		{
			CMFCToolBarComboBoxButton btnComboBox(ID_V_GRID_COUNT, GetCmdMgr()->GetCmdImage(ID_V_GRID_COUNT), CBS_DROPDOWNLIST);
			InitializeGridLineComboBox(btnComboBox, TEXT("V Line:"));
			m_wndToolBarDrawing.ReplaceButton(ID_V_GRID_COUNT, btnComboBox);
		}
	}
	return(0);
}
#ifdef ENABLE_INTERFACE_SPECIFIC_NODE_MAP
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMainFrame::OnViewInterfacenode()
{

	CStViewerApp *pCStViewerApp = dynamic_cast<CStViewerApp*>(AfxGetApp());
	pCStViewerApp->ShowNodeMapForInterfaces(GetSafeHwnd());
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMainFrame::OnViewSystemnode()
{

	CStViewerApp *pCStViewerApp = dynamic_cast<CStViewerApp*>(AfxGetApp());
	pCStViewerApp->ShowNodeMapForSystems(GetSafeHwnd());
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
LRESULT CMainFrame::OnUpdatedInterfaceList(WPARAM, LPARAM)
{
	CStViewerApp *pCStViewerApp = dynamic_cast<CStViewerApp*>(AfxGetApp());
	pCStViewerApp->UpdateInterfaceNodeMapDisplayWnd();
	return(0);
}
#endif //ENABLE_INTERFACE_SPECIFIC_NODE_MAP


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMainFrame::OnViewFullscreen()
{
	ShowFullScreen();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMainFrame::InitializeGridLineComboBox(CMFCToolBarComboBoxButton &btnComboBox, LPCTSTR szHeader)
{
	const size_t nMaxLineCount = 32;
	for (size_t i = 0; i <= nMaxLineCount; ++i)
	{
		CString strText;
#if _MSC_VER < 1900
#if _WIN64
		strText.Format(TEXT("%s%I64u"), szHeader, i);
#else
		strText.Format(TEXT("%s%u"), szHeader, i);
#endif
#else
		strText.Format(TEXT("%s%zu"), szHeader, i);
#endif
		btnComboBox.AddItem(strText);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMainFrame::OnDestroy()
{
	CMDIFrameWndEx::OnDestroy();

	KillTimer(m_nTimerID);
}
