
// StStillImageDoc.cpp : implementation of the CStStillImageDoc class
//

#include "stdafx.h"
#include "StViewer.h"
#include "StStillImageDoc.h"
#include "resource.h"
#include <propkey.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CStStillImageDoc

IMPLEMENT_DYNCREATE(CStStillImageDoc, CStViewerDocBase)

BEGIN_MESSAGE_MAP(CStStillImageDoc, CStViewerDocBase)
	ON_UPDATE_COMMAND_UI_RANGE(ID_ACQ_START, ID_FILE_SAVE_CAMERA_DESCRIPTION_FILE, &CStStillImageDoc::OnUpdateCommandHandler)
	ON_COMMAND_RANGE(ID_ACQ_START, ID_FILE_SAVE_CAMERA_DESCRIPTION_FILE, &CStStillImageDoc::OnCommandHandler)

	ON_UPDATE_COMMAND_UI_RANGE(ID_DRAWING_NONE, ID_DRAWING_CROSS, &CStStillImageDoc::OnUpdateDrawingCommandHandler)
	ON_UPDATE_COMMAND_UI_RANGE(ID_H_GRID_COUNT, ID_V_GRID_COUNT, &CStStillImageDoc::OnUpdateDrawingCommandHandler)
	ON_COMMAND_RANGE(ID_DRAWING_NONE, ID_DRAWING_CROSS, &CStStillImageDoc::OnDrawingCommandHandler)
	ON_CBN_SELENDOK(ID_H_GRID_COUNT, OnSelectHGridCount)
	ON_CBN_SELENDOK(ID_V_GRID_COUNT, OnSelectVGridCount)
	ON_COMMAND_RANGE(ID_GRAPHDATASOURCE_REGISTEREDIMAGE, ID_GRAPHDATASOURCE_CONVERTEDIMAGE, &CStStillImageDoc::OnGraphDataSource)
	ON_UPDATE_COMMAND_UI_RANGE(ID_GRAPHDATASOURCE_REGISTEREDIMAGE, ID_GRAPHDATASOURCE_CONVERTEDIMAGE, &CStStillImageDoc::OnGraphDataSourceCommandHandler)
END_MESSAGE_MAP()

// CStStillImageDoc construction/destruction
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStStillImageDoc::CStStillImageDoc()
{
	// TODO: add one-time construction code here
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStStillImageDoc::~CStStillImageDoc()
{
	DeregisterNodesFormView();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CStStillImageDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	m_strFileName = lpszPathName;

	BOOL bReval = TRUE;
	try
	{
		CStViewerApp *pCStViewerApp = dynamic_cast<CStViewerApp*>(AfxGetApp());
		//Open a file.
		GenICam::gcstring strgcFileName(m_strFileName);

		StApi::CIStImageBufferPtr pIStImageBuffer;
		pIStImageBuffer.Reset(StApi::CreateIStImageBuffer(NULL));

		StApi::CIStStillImageFilerPtr pIStStillImageFiler(StApi::CreateIStFiler(StApi::StFilerType_StillImage));
		pIStStillImageFiler->Load(pIStImageBuffer, strgcFileName);
		m_pIStImageDisplayWnd->RegisterIStImage(pIStImageBuffer->GetIStImage());


		CFileFind find;
		if (find.FindFile(m_strFileName))
		{
			find.FindNextFile();
			find.GetLastWriteTime(m_UpdateTime);
			find.Close();
		}

		if (bReval)
		{
			//Get file name from full path.
			int nStart = 0;
			for (;;)
			{
				const int nTmp = m_strFileName.Find(TEXT("\\"), nStart);
				if (nTmp < 0) break;
				nStart = nTmp + 1;
			}
			CString strOnlyFileName(m_strFileName.Mid(nStart));

			//Set title.
			SetTitle(strOnlyFileName);

			//Log
			AddDeviceLog(IDS_FILE_OPENED);

			//Load setting file.
			pCStViewerApp->LoadSaveNodeMapSettingFile(m_pIStImageDisplayWnd->GetINodeMap(), true, StConfigurationFileType_DisplayImageWnd, strOnlyFileName);
			pCStViewerApp->LoadSaveNodeMapSettingFile(m_pIStImageDisplayWnd->GetIStPixelFormatConverter()->GetINodeMap(), true, StConfigurationFileType_PixelFormatConverter, strOnlyFileName);

			//Create a node map window.
			CMDIFrameWndEx *pCMDIFrameWndEx = dynamic_cast<CMDIFrameWndEx*>(pCStViewerApp->GetMainWnd());
			if (pCMDIFrameWndEx)
			{
				UINT nID = GetUnusedID(pCMDIFrameWndEx);
				if (nID != 0)
				{
					m_pCNodeMapView = new CNodeMapView();
					pCStViewerApp->LoadSaveNodeMapSettingFile(m_pCNodeMapView->GetINodeMap(), true, StConfigurationFileType_NodeMapWnd, strOnlyFileName);
					CString strNodeMapTitle(TEXT("NodeMap-"));
					strNodeMapTitle.Append(strOnlyFileName);
					bReval = m_pCNodeMapView->Create(strNodeMapTitle, pCMDIFrameWndEx, CSize(300, 500), TRUE, nID, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI);
					if (!bReval)
					{
						delete m_pCNodeMapView;
						m_pCNodeMapView = NULL;
					}
					else
					{
						m_pCNodeMapView->EnableDocking(CBRS_ALIGN_ANY);
						pCMDIFrameWndEx->DockPane(m_pCNodeMapView);
						BOOL bHiColorIcons = TRUE;
						HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
						m_pCNodeMapView->SetIcon(hPropertiesBarIcon, FALSE);
					}

				}

				//Graph
				nID = GetUnusedID(pCMDIFrameWndEx);
				if (nID != 0)
				{
					m_pCGraphView = new CGraphView(m_pIStImageDisplayWnd);
					CString strGraphTitle(TEXT("Graph-"));
					strGraphTitle.Append(strOnlyFileName);
					bReval = m_pCGraphView->Create(strGraphTitle, pCMDIFrameWndEx, CSize(300, 500), TRUE, nID, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI);
					if (!bReval)
					{
						delete m_pCGraphView;
						m_pCGraphView = NULL;
					}
					else
					{
						m_pCGraphView->EnableDocking(CBRS_ALIGN_ANY);
						pCMDIFrameWndEx->DockPane(m_pCGraphView);
						BOOL bHiColorIcons = TRUE;
						HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
						m_pCGraphView->SetIcon(hPropertiesBarIcon, FALSE);

						StApi::IStImage *pIStImage = m_pIStImageDisplayWnd->GetRegisteredIStImage();

						GenApi::CNodeMapPtr pINodeMapGraphDataFilter(m_pCGraphView->GetINodeMapForGraphDataFilter());
						GenApi::CIntegerPtr pIIntegerWidthMax(pINodeMapGraphDataFilter->GetNode("WidthMax"));
						GenApi::CIntegerPtr pIIntegerHeightMax(pINodeMapGraphDataFilter->GetNode("HeightMax"));

						if (GenApi::IsWritable(pIIntegerWidthMax))
						{
							pIIntegerWidthMax->SetValue(pIStImage->GetImageWidth());
						}
						if (GenApi::IsWritable(pIIntegerHeightMax))
						{
							pIIntegerHeightMax->SetValue(pIStImage->GetImageHeight());
						}

					}
				}
				RegisterNodesToView();
			}
		}
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
		bReval = FALSE;
	}
	if (!bReval)
	{
		delete this;
	}
	return(bReval);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStStillImageDoc::OnCloseDocument()
{
	CStViewerApp *pCStViewerApp = dynamic_cast<CStViewerApp*>(AfxGetApp());
	//Close "NodeMapView".
	if(m_pCNodeMapView)
	{
		pCStViewerApp->LoadSaveNodeMapSettingFile(m_pCNodeMapView->GetINodeMap(), false, StConfigurationFileType_NodeMapWnd, GetTitle());
		m_pCNodeMapView->UndockPane();
		m_pCNodeMapView->DestroyWindow();
		delete m_pCNodeMapView;
		m_pCNodeMapView = NULL;
	}
	//Close "GraphView".
	if (m_pCGraphView)
	{
		m_pCGraphView->UndockPane();
		m_pCGraphView->DestroyWindow();
		delete m_pCGraphView;
		m_pCGraphView = NULL;
	}

	pCStViewerApp->LoadSaveNodeMapSettingFile(m_pIStImageDisplayWnd->GetINodeMap(), false, StConfigurationFileType_DisplayImageWnd, GetTitle());
	pCStViewerApp->LoadSaveNodeMapSettingFile(m_pIStImageDisplayWnd->GetIStPixelFormatConverter()->GetINodeMap(), false, StConfigurationFileType_PixelFormatConverter, GetTitle());

	m_pIStImageDisplayWnd->Close();


	//Log
	AddDeviceLog(IDS_FILE_CLOSED);

	CStViewerDocBase::OnCloseDocument();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStStillImageDoc::OnUpdateCommandHandler(CCmdUI *pCmdUI)
{
	BOOL bEnable = FALSE;
	switch (pCmdUI->m_nID)
	{
	case(ID_ACQ_START) :			bEnable = FALSE;				break;
	case(ID_ACQ_STOP) :				bEnable = FALSE;				break;
	case(ID_SAVE_STILL_IMAGE) :		bEnable = m_pIStImageDisplayWnd->HasImage();	break;
	case(ID_START_RECORDING) :		bEnable = FALSE;						break;
	case(ID_STOP_RECORDING) :		bEnable = FALSE;						break;
	case(ID_FILE_SAVE_CAMERA_CONFIG_FILE):	bEnable = FALSE; 	break;
	case(ID_FILE_LOAD_CAMERA_CONFIG_FILE):	bEnable = FALSE; 	break;
	case(ID_FILE_SAVE_CAMERA_DESCRIPTION_FILE):	bEnable = FALSE;  break;
	}
	pCmdUI->Enable(bEnable);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStStillImageDoc::OnCommandHandler(UINT nID)
{
	switch (nID)
	{
	case(ID_SAVE_STILL_IMAGE) :		SaveStillImage();		break;
	}

}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStStillImageDoc::OnGraphDataSourceCommandHandler(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
	const bool isRegisteredImage = m_pCGraphView->GetGraphDataSource();
	if (
		((pCmdUI->m_nID == ID_GRAPHDATASOURCE_REGISTEREDIMAGE) && isRegisteredImage) ||
		((pCmdUI->m_nID == ID_GRAPHDATASOURCE_CONVERTEDIMAGE) && !isRegisteredImage)
		)
	{
		pCmdUI->SetCheck();
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStStillImageDoc::OnGraphDataSource(UINT nID)
{
	const bool isRegisteredImage = (nID == ID_GRAPHDATASOURCE_REGISTEREDIMAGE);
	m_pCGraphView->SetGraphDataSource(isRegisteredImage);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStStillImageDoc::GetStatusBarText(size_t nIndex, CString &strText)
{

	if (nIndex == 0)
	{
		//File Name
		strText = GetTitle();
	}
	else if (nIndex == 1)
	{
		//FPS
		CString strFormat;
		strFormat.LoadStringW(IDS_STRING_FILE_TIME);
		strText = TEXT("Update time:");
		strText.Append(m_UpdateTime.Format(strFormat));
	}
	else if (nIndex == 2)
	{
		GetPixelInfoText(m_pIStImageDisplayWnd, strText);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStStillImageDoc::RegisterNodesToView()
{
	CStViewerDocSingleDocBase::RegisterNodesToView();
	m_pCNodeMapView->Refresh(true);
}
