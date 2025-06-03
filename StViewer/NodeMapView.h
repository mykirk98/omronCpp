#pragma once
#include <vector>
// CNodeMapView
class CNodeMapView : public CDockablePane
{
public:
	CNodeMapView();
	virtual ~CNodeMapView();

	// Attributes
protected:
	StApi::CIStNodeMapDisplayWndPtr m_pIStNodeMapDisplayWnd;
	// Implementation
public:
	/*!
	Adjust the size of "NodeMapWnd".
	*/
	void AdjustLayout();

	GenApi::INodeMap *GetINodeMap() { return(m_pIStNodeMapDisplayWnd->GetINodeMap()); };
	StApi::RegisteredINodeHandle_t RegisterINode(GenApi::INode *, const GenICam::gcstring &);
	void DeregisterINode(std::vector<StApi::RegisteredINodeHandle_t> &);
	void Refresh(bool Collapse = false);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};


