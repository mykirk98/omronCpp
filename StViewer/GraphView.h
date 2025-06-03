#pragma once

// CGraphView

class CGraphView : public CDockablePane
{
public:
	explicit CGraphView(StApi::IStImageDisplayWnd *pIStImageDisplayWnd);
	virtual ~CGraphView();

	// Attributes
protected:

	StApi::CIStGraphDataFilterPtr m_pIStGraphDataFilter;
	StApi::CIStGraphDisplayWndPtr m_pIStGraphDisplayWnd;
	StApi::IStImageDisplayWnd *m_pIStImageDisplayWnd;
	UINT_PTR m_nTimerID;
	static UINT_PTR m_nNextTimerID;
	bool m_isRegisteredImageGraphDataSource;
	// Implementation
public:
	/*!
	Adjust the size of "GraphWnd".
	*/
	void AdjustLayout();

	GenApi::INodeMap *GetINodeMapForGraphDataFilter()
	{
		return(m_pIStGraphDataFilter->GetINodeMap());
	}
	GenApi::INodeMap *GetINodeMapForGraphDisplayWnd()
	{
		return(m_pIStGraphDisplayWnd->GetINodeMap());
	}

	bool GetGraphDataSource() const
	{
		return(m_isRegisteredImageGraphDataSource);
	}

	void SetGraphDataSource(bool isRegisteredImage)
	{
		m_isRegisteredImageGraphDataSource = isRegisteredImage;
	}
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
};

