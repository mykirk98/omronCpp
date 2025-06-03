
#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "MFCSingleDocViewer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CPropertiesWnd::CPropertiesWnd()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient,rectCombo;
	GetClientRect(rectClient);

	m_pIStNodeMapDisplayWnd->SetPosition(0, 0, rectClient.Width(), rectClient.Height());
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;


	m_pIStNodeMapDisplayWnd = StApi::CreateIStWnd(StApi::StWindowType_NodeMapDisplay);

	AdjustLayout();
	m_pIStNodeMapDisplayWnd->Show(GetSafeHwnd(), StApi::StWindowMode_Child);
	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CPropertiesWnd::InitProperties(StApi::IStDevice *pIStDevice, StApi::IStDataStream *pIStDataStream)
{
	GenApi::CNodeMapPtr pINodeMapLocalDevice(pIStDevice->GetLocalIStPort()->GetINodeMap());
	GenApi::CNodeMapPtr pINodeMapRemoteDevice(pIStDevice->GetRemoteIStPort()->GetINodeMap());

	try
	{
		const char *pszNames[] =
		{
			"PixelFormat",
			"ExposureMode",
			"ExposureTime",
			"GainSelector",
			"Gain",
			"BalanceRatioSelector",
			"BalanceRatio",
			"BalanceWhiteAuto",
			"TriggerSelector",
			"TriggerMode",
			"TriggerSource",
			"TriggerSoftware",
		};
		for (size_t i = 0; i < _countof(pszNames); ++i)
		{
			GenApi::CNodePtr pINode(pINodeMapRemoteDevice->GetNode(pszNames[i]));
			if (pINode.IsValid())
			{
				m_pIStNodeMapDisplayWnd->RegisterINode(pINode, "Remote Device");
			}
		}
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}

}

