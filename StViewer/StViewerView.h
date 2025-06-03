
// StViewerView.h : interface of the CStViewerView class
//

#pragma once


class CStViewerView : public CView
{
protected: // create from serialization only
	CStViewerView();
	DECLARE_DYNCREATE(CStViewerView)

// Attributes
public:
	CStViewerDocBase* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CStViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();

protected:
	/*!
	Adjust the size of "DisplayImageWnd".
	*/
	void AdjustLayout();
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	LRESULT OnDeviceLost(WPARAM wParam, LPARAM lParam);
};

#ifndef _DEBUG  // debug version in StViewerView.cpp
inline CStViewerDocBase* CStViewerView::GetDocument() const
   { return reinterpret_cast<CStViewerDocBase*>(m_pDocument); }
#endif

