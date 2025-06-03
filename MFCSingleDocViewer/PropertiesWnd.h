
#pragma once

class CPropertiesWnd : public CDockablePane
{
// Construction
public:
	CPropertiesWnd();

	void AdjustLayout();

// Attributes

protected:
	StApi::CIStNodeMapDisplayWndPtr m_pIStNodeMapDisplayWnd;
// Implementation
public:
	virtual ~CPropertiesWnd();

	void InitProperties(StApi::IStDevice *, StApi::IStDataStream *);
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

