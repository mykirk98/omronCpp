
#pragma once

/////////////////////////////////////////////////////////////////////////////
// COutputList window



class COutputList : public CListCtrl
{
// Construction
public:
	explicit COutputList(size_t nMaximumItemCount = 100);

// Implementation
public:
	virtual ~COutputList();
	size_t GetMaximumItemCount() const { return(m_nMaximumItemCount); };

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()

	size_t m_nMaximumItemCount;
};

class COutputWnd : public CDockablePane
{
// Construction
public:
	COutputWnd();
	virtual ~COutputWnd();

	void UpdateFonts();
	void AddLog(LPCTSTR szSource, LPCTSTR szLog);

// Attributes
protected:
	CMFCTabCtrl	m_wndTabs;
	COutputList m_wndOutputLog;

// Implementation
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

