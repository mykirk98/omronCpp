
// MFCSingleDocViewerView.h : interface of the CMFCSingleDocViewerView class
//

#pragma once


class CMFCSingleDocViewerView : public CScrollView
{
protected: // create from serialization only
	CMFCSingleDocViewerView();
	DECLARE_DYNCREATE(CMFCSingleDocViewerView)

// Attributes
public:
	CMFCSingleDocViewerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
protected:

// Implementation
public:
	virtual ~CMFCSingleDocViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CBitmap m_objCBitmap;
	bool m_isNewImage;
	UINT_PTR m_nTimerID;
	CBrush m_objBackgroundBrush;
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	LRESULT OnDeviceLost(WPARAM wParam, LPARAM lParam);
	LRESULT OnNewImage(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void OnInitialUpdate();
	afx_msg void OnFileSaveAs();
};

#ifndef _DEBUG  // debug version in MFCSingleDocViewerView.cpp
inline CMFCSingleDocViewerDoc* CMFCSingleDocViewerView::GetDocument() const
   { return reinterpret_cast<CMFCSingleDocViewerDoc*>(m_pDocument); }
#endif

