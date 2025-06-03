// NodeMapView.cpp
//

#include "stdafx.h"
#include "StViewer.h"
#include "NodeMapView.h"


// CNodeMapView

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CNodeMapView::CNodeMapView() : m_pIStNodeMapDisplayWnd(NULL)
{
	//Create "NodeMapWnd".
	m_pIStNodeMapDisplayWnd.Reset(StApi::CreateIStWnd(StApi::StWindowType_NodeMapDisplay));
	m_pIStNodeMapDisplayWnd->SetVisibleFilter(true);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CNodeMapView::~CNodeMapView()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CNodeMapView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CNodeMapView Message handler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int CNodeMapView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct)  ==  -1)
		return -1;

	try
	{
		if(m_pIStNodeMapDisplayWnd.IsValid())
		{
			//Adjust the size of "NodeMapWnd".
			AdjustLayout();

			//Show "NodeMapWnd".
			m_pIStNodeMapDisplayWnd->Show(GetSafeHwnd(), StApi::StWindowMode_Child);
		}
	}
	catch(const GenICam::GenericException &e)
	{
		OnException(e);
	}
	return 0;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CNodeMapView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	
	//Adjust the size of "NodeMapWnd".
	AdjustLayout();
}

//-----------------------------------------------------------------------------
//Adjust the size of "NodeMapWnd".
//-----------------------------------------------------------------------------
void CNodeMapView::AdjustLayout()
{
	if (GetSafeHwnd()  ==  NULL)
	{
		return;
	}
	
	if(m_pIStNodeMapDisplayWnd.IsValid())
	{
		//Align the size of "NodeMapWnd" to the client window.
		CRect rectClient;
		GetClientRect(rectClient);
		m_pIStNodeMapDisplayWnd->SetPosition(rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height());
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
StApi::RegisteredINodeHandle_t CNodeMapView::RegisterINode(GenApi::INode *pINode, const GenICam::gcstring &strTitle)
{
	const StApi::RegisteredINodeHandle_t hRegistered = m_pIStNodeMapDisplayWnd->RegisterINode(pINode, strTitle);
	return(hRegistered);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CNodeMapView::DeregisterINode(std::vector<StApi::RegisteredINodeHandle_t> &vecRegisteredINodeHandles)
{
	for (std::vector<StApi::RegisteredINodeHandle_t>::const_iterator itr = vecRegisteredINodeHandles.begin(); itr != vecRegisteredINodeHandles.end(); ++itr)
	{
		m_pIStNodeMapDisplayWnd->DeregisterCallback(*itr);
	}
	vecRegisteredINodeHandles.clear();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CNodeMapView::Refresh(bool Collapse)
{
	if (Collapse)
	{
		m_pIStNodeMapDisplayWnd->Collapse();
	}
	m_pIStNodeMapDisplayWnd->Refresh();
}