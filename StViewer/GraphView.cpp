#include "stdafx.h"
#include "GraphView.h"

UINT_PTR CGraphView::m_nNextTimerID = 10;
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CGraphView::CGraphView(StApi::IStImageDisplayWnd *pIStImageDisplayWnd) : m_pIStGraphDataFilter(StApi::CreateIStFilter(StApi::StFilterType_GraphData)), m_pIStImageDisplayWnd(pIStImageDisplayWnd), m_nTimerID(0), m_isRegisteredImageGraphDataSource(false)
{
	//Create "GraphWnd".
	m_pIStGraphDisplayWnd.Reset(StApi::CreateIStWnd(StApi::StWindowType_GraphDisplay));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CGraphView::~CGraphView()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CGraphView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CGraphView Message handler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int CGraphView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	try
	{
		if (m_pIStGraphDisplayWnd.IsValid())
		{
			//Adjust the size of "GraphWnd".
			AdjustLayout();

			//Show "GraphWnd".
			m_pIStGraphDisplayWnd->Show(GetSafeHwnd(), StApi::StWindowMode_Child);

			m_nTimerID = SetTimer(m_nNextTimerID++, 200, NULL);
		}
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}
	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CGraphView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	//Adjust the size of "GraphWnd".
	AdjustLayout();
}

//-----------------------------------------------------------------------------
//Adjust the size of "GraphWnd".
//-----------------------------------------------------------------------------
void CGraphView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	if (m_pIStGraphDisplayWnd.IsValid())
	{
		//Align the size of "GraphWnd" to the client window.
		CRect rectClient;
		GetClientRect(rectClient);
		m_pIStGraphDisplayWnd->SetPosition(rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height());
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CGraphView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == m_nTimerID)
	{
		if (IsVisible())
		{
			try
			{
				if (m_pIStImageDisplayWnd->HasImage())
				{
					StApi::IStImage *pIStImage = m_isRegisteredImageGraphDataSource ? m_pIStImageDisplayWnd->GetRegisteredIStImage() : m_pIStImageDisplayWnd->GetConvertedIStImage();
					m_pIStGraphDataFilter->Filter(pIStImage);
					m_pIStGraphDisplayWnd->RegisterIStGraphDataBufferList(m_pIStGraphDataFilter->GetIStGraphDataBufferList());
				}
			}
			catch (...)
			{

			}
		}
	}
	CDockablePane::OnTimer(nIDEvent);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CGraphView::OnDestroy()
{
	CDockablePane::OnDestroy();

	KillTimer(m_nTimerID);
}
