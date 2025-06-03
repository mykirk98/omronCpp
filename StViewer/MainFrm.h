
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "OutputWnd.h"
class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCToolBar       m_wndToolBarAdvanced;
	CMFCToolBar       m_wndToolBarDrawing;
	CMFCStatusBar     m_wndStatusBar;
	COutputWnd        m_wndOutput;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnAddLog(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnResetToolbar(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()


	/*!
	Update status bar.
	*/
	void UpdateStatusBar();
	
	void InitializeGridLineComboBox(CMFCToolBarComboBoxButton &btnComboBox, LPCTSTR szHeader);

	UINT_PTR m_nTimerID;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
#ifdef ENABLE_INTERFACE_SPECIFIC_NODE_MAP
	afx_msg void OnViewInterfacenode();
	afx_msg void OnViewSystemnode();
	afx_msg LRESULT OnUpdatedInterfaceList(WPARAM wParam, LPARAM lParam);
#endif //ENABLE_INTERFACE_SPECIFIC_NODE_MAP
	afx_msg void OnViewFullscreen();
	afx_msg void OnDestroy();
};


