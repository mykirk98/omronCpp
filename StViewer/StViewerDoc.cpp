
// StViewerDoc.cpp : implementation of the CStViewerDoc class
//

#include "stdafx.h"
#include "StViewer.h"
#include "StViewerDoc.h"
#include "resource.h"
#include <propkey.h>

#include "CameraSideFFC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace GenTL;

// CStViewerDoc

IMPLEMENT_DYNCREATE(CStViewerDoc, CStViewerDocBase)

BEGIN_MESSAGE_MAP(CStViewerDoc, CStViewerDocBase)
	ON_UPDATE_COMMAND_UI_RANGE(ID_ACQ_START, ID_FILE_SAVE_CAMERA_DESCRIPTION_FILE, &CStViewerDoc::OnUpdateCommandHandler)
	ON_COMMAND_RANGE(ID_ACQ_START, ID_FILE_SAVE_CAMERA_DESCRIPTION_FILE, &CStViewerDoc::OnCommandHandler)

	ON_UPDATE_COMMAND_UI_RANGE(ID_DRAWING_NONE, ID_DRAWING_CROSS, &CStViewerDoc::OnUpdateDrawingCommandHandler)
	ON_UPDATE_COMMAND_UI_RANGE(ID_H_GRID_COUNT, ID_V_GRID_COUNT, &CStViewerDoc::OnUpdateDrawingCommandHandler)
	ON_COMMAND_RANGE(ID_DRAWING_NONE, ID_DRAWING_CROSS, &CStViewerDoc::OnDrawingCommandHandler)
	ON_CBN_SELENDOK(ID_H_GRID_COUNT, OnSelectHGridCount)
	ON_CBN_SELENDOK(ID_V_GRID_COUNT, OnSelectVGridCount)
	ON_COMMAND(ID_FILE_START_FFC, &CStViewerDoc::OnFileStartFfc)
	ON_UPDATE_COMMAND_UI(ID_FILE_START_FFC, &CStViewerDoc::OnUpdateFileStartFfc)
	ON_COMMAND_RANGE(ID_GRAPHDATASOURCE_REGISTEREDIMAGE, ID_GRAPHDATASOURCE_CONVERTEDIMAGE, &CStViewerDoc::OnGraphDataSource)
	ON_UPDATE_COMMAND_UI_RANGE(ID_GRAPHDATASOURCE_REGISTEREDIMAGE, ID_GRAPHDATASOURCE_CONVERTEDIMAGE, &CStViewerDoc::OnGraphDataSourceCommandHandler)
END_MESSAGE_MAP()


// CStViewerDoc construction/destruction
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStViewerDoc::CStViewerDoc() : 
m_pIStDevice(NULL), m_pIStDataStream(NULL), m_pCDefectivePixelDetectionPane(NULL),
m_pCSaveMultipleImagesFileBase(NULL), m_IsAcquisitionRunning(false), m_nReceivedImageCount(0), m_nDroppedIDCount(0), m_IsDeviceLost(false)
{
	// TODO: add one-time construction code here
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStViewerDoc::~CStViewerDoc()
{
	DeregisterNodesFormView();
	if(m_pCSaveMultipleImagesFileBase)
	{
		delete m_pCSaveMultipleImagesFileBase;
		m_pCSaveMultipleImagesFileBase = NULL;
	}

}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CStViewerDoc::OnNewDocument()
{

	if (!CStViewerDocBase::OnNewDocument())
		return FALSE;
	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	BOOL bReval = TRUE;

	HCURSOR hCursor = NULL;
	try
	{
		//Open the device.
		CStViewerApp *pCStViewerApp = dynamic_cast<CStViewerApp*>(AfxGetApp());
		m_pIStDevice.Reset(pCStViewerApp->CreateIStDevice());
		if (!m_pIStDevice.IsValid())
		{
			bReval = FALSE;
		}
		else
		{
			hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

			GenApi::CNodeMapPtr pINodeMapRemoteDevice(m_pIStDevice->GetRemoteIStPort()->GetINodeMap());

			//Register device lost event.
			GenApi::CNodeMapPtr pINodeMapLocalDevice(m_pIStDevice->GetLocalIStPort()->GetINodeMap());
			if (pINodeMapLocalDevice)
			{
				GenApi::CNodePtr pINodeEventDeviceLost(pINodeMapLocalDevice->GetNode("EventDeviceLost"));
				if (pINodeEventDeviceLost.IsValid())
				{
					m_pIStRegisteredCallbackDeviceLost.Reset(StApi::RegisterCallback(pINodeEventDeviceLost, *this, &CStViewerDoc::OnDeviceLost, (void*)NULL, GenApi::cbPostOutsideLock));
				}
			}

			//Start Event Acquisition Thread
			m_pIStDevice->StartEventAcquisitionThread();

			//Set title.
			const GenICam::gcstring strDisplayName = m_pIStDevice->GetIStDeviceInfo()->GetDisplayName();
			m_strDeviceName = GCSTRING_2_LPCTSTR(strDisplayName);
			GenICam::gcstring strUserName;
			try
			{
				strUserName = m_pIStDevice->GetIStDeviceInfo()->GetUserDefinedName();
			}
			catch (...)
			{
				strUserName = "";
			}
			CString strTitle;
			if ((0 < strUserName.length()) && (m_strDeviceName.Compare(GCSTRING_2_LPCTSTR(strUserName)) != 0))
			{
				strTitle.Format(TEXT("%s[%s]"), m_strDeviceName.GetBuffer(), GCSTRING_2_LPCTSTR(strUserName));
			}
			else
			{
				strTitle = m_strDeviceName;
			}
			SetTitle(strTitle);
			pCStViewerApp->LoadSaveNodeMapSettingFile(m_pIStImageDisplayWnd->GetINodeMap(), true, StConfigurationFileType_DisplayImageWnd, m_pIStDevice);
			pCStViewerApp->LoadSaveNodeMapSettingFile(m_pIStImageDisplayWnd->GetIStPixelFormatConverter()->GetINodeMap(), true, StConfigurationFileType_PixelFormatConverter, m_pIStDevice);

			//Create object and get IStDataStream pointer.
			m_pIStDataStream.Reset(m_pIStDevice->CreateIStDataStream(0));
			if (m_pIStDataStream.IsValid())
			{
				//Register callback function to receive images.
				RegisterCallback(m_pIStDataStream, *this, &CStViewerDoc::OnStCallback, (void*)NULL);


				CreateStViewerNodeMap();
				if (m_pStViewerNodeMap.IsValid())
				{
					GenApi::CIntegerPtr pIInteger_Min(m_pStViewerNodeMap->GetNode("MinStreamBufferCount"));
					GenApi::CIntegerPtr pIInteger_Count(m_pStViewerNodeMap->GetNode("StreamBufferCount"));
					if (pIInteger_Min.IsValid() && pIInteger_Count.IsValid())
					{
						try
						{
							int64_t nMin = m_pIStDataStream->GetIStDataStreamInfo()->GetBufAnnounceMin();
							if (pIInteger_Min->GetValue() < nMin)
							{
								pIInteger_Min->SetValue(nMin);
								if (pIInteger_Count->GetValue() < nMin)
								{
									pIInteger_Count->SetValue(nMin, false);
								}
							}
						}
						catch (...)
						{
						}

					}
					pCStViewerApp->LoadSaveNodeMapSettingFile(m_pStViewerNodeMap, true, StConfigurationFileType_StViewer, m_pIStDevice);
				}
			}

			//Create a node map window.
			CMDIFrameWndEx *pCMDIFrameWndEx = dynamic_cast<CMDIFrameWndEx*>(pCStViewerApp->GetMainWnd());
			if (pCMDIFrameWndEx)
			{
				UINT nID = GetUnusedID(pCMDIFrameWndEx);
				if (nID != 0)
				{
					m_pCNodeMapView = new CNodeMapView();
					pCStViewerApp->LoadSaveNodeMapSettingFile(m_pCNodeMapView->GetINodeMap(), true, StConfigurationFileType_NodeMapWnd, m_pIStDevice);
					CString strNodeMapTitle;
					strNodeMapTitle.Format(TEXT("NodeMap-%s"), strTitle.GetBuffer());
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

					//Log
					AddDeviceLog(IDS_DEVICE_OPENED);
				}

				//Graph
				nID = GetUnusedID(pCMDIFrameWndEx);
				if (nID != 0)
				{
					m_pCGraphView = new CGraphView(m_pIStImageDisplayWnd);
					CString strGraphTitle;
					strGraphTitle.Format(TEXT("Graph-%s"), strTitle.GetBuffer());
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

						GenApi::CNodeMapPtr pINodeMapGraphDataFilter(m_pCGraphView->GetINodeMapForGraphDataFilter());
						GenApi::CIntegerPtr pIIntegerSensorWidth(pINodeMapRemoteDevice->GetNode("SensorWidth"));
						GenApi::CIntegerPtr pIIntegerSensorHeight(pINodeMapRemoteDevice->GetNode("SensorHeight"));
						GenApi::CIntegerPtr pIIntegerWidthMax(pINodeMapGraphDataFilter->GetNode("WidthMax"));
						GenApi::CIntegerPtr pIIntegerHeightMax(pINodeMapGraphDataFilter->GetNode("HeightMax"));

						if (GenApi::IsReadable(pIIntegerSensorWidth) && GenApi::IsWritable(pIIntegerWidthMax))
						{
							pIIntegerWidthMax->SetValue(pIIntegerSensorWidth->GetValue());
						}
						if (GenApi::IsReadable(pIIntegerSensorHeight) && GenApi::IsWritable(pIIntegerHeightMax))
						{
							pIIntegerHeightMax->SetValue(pIIntegerSensorHeight->GetValue());
						}
					}
				}
				
				//Defective pixel
				if ((m_pIStDevice->GetIStDeviceInfo()->GetAccessStatus() == DEVICE_ACCESS_STATUS_OPEN_READWRITE) && CDefectivePixelManager::IsSupported(pINodeMapRemoteDevice))
				{
					nID = GetUnusedID(pCMDIFrameWndEx);
					if (nID != 0)
					{
						m_pCDefectivePixelDetectionPane = new CDefectivePixelDetectionPane(pINodeMapRemoteDevice, this, m_pIStImageDisplayWnd);
						CString strDefectivePixelDetectionPaneTitle;
						strDefectivePixelDetectionPaneTitle.Format(TEXT("Defective Pixel Detection-%s"), strTitle.GetBuffer());
						bReval = m_pCDefectivePixelDetectionPane->Create(strDefectivePixelDetectionPaneTitle, pCMDIFrameWndEx, CSize(300, 500), TRUE, nID, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI);
						if (!bReval)
						{
							delete m_pCDefectivePixelDetectionPane;
							m_pCDefectivePixelDetectionPane = NULL;
						}
						else
						{
							m_pCDefectivePixelDetectionPane->EnableDocking(CBRS_ALIGN_ANY);
							pCMDIFrameWndEx->DockPane(m_pCDefectivePixelDetectionPane);
							BOOL bHiColorIcons = TRUE;
							HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
							m_pCDefectivePixelDetectionPane->SetIcon(hPropertiesBarIcon, FALSE);

							//Create object and get IStDefectivePixelDetectionFilter pointer.
							GenApi::CNodeMapPtr pINodeMap(m_pCDefectivePixelDetectionPane->GetINodeMapForDefectivePixelDetectionFilter());
							pCStViewerApp->LoadSaveNodeMapSettingFile(pINodeMap, true, StConfigurationFileType_DefectivePixelDetection, m_pIStDevice);
							m_objCIStImageCallbackList.Add(m_pCDefectivePixelDetectionPane);
						}
					}
				}

				RegisterNodesToView();
			}

			SetCursor(hCursor);
		}
				
	}
	catch (const GenICam::GenericException &e)
	{
		m_pIStRegisteredCallbackDeviceLost.Reset(NULL);
		m_pIStDataStream.Reset(NULL);
		m_pIStDevice.Reset(NULL);
		if (hCursor)
		{
			SetCursor(hCursor);
		}
		OnException(e);
		bReval = FALSE;
	}

	if (!bReval)
	{
		delete this;
	}
	return bReval;
}




// CStViewerDoc commands


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::OnCloseDocument()
{
	if (!m_pIStDevice.IsValid()) return;
	StopImageAcquisition();

	//Stop event acquisition thread before closing "NodeMapView".
	m_pIStDevice->StopEventAcquisitionThread();

	CStViewerApp *pCStViewerApp = dynamic_cast<CStViewerApp*>(AfxGetApp());
	//Close "NodeMapView".
	if(m_pCNodeMapView)
	{
		pCStViewerApp->LoadSaveNodeMapSettingFile(m_pCNodeMapView->GetINodeMap(), false, StConfigurationFileType_NodeMapWnd, m_pIStDevice);

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
	//Close "DefectivePixelDetectionPane".
	if (m_pCDefectivePixelDetectionPane)
	{
		pCStViewerApp->LoadSaveNodeMapSettingFile(m_pCDefectivePixelDetectionPane->GetINodeMapForDefectivePixelDetectionFilter(), false, StConfigurationFileType_DefectivePixelDetection, m_pIStDevice);

		m_pCDefectivePixelDetectionPane->UndockPane();
		m_pCDefectivePixelDetectionPane->DestroyWindow();
		delete m_pCDefectivePixelDetectionPane;
		m_pCDefectivePixelDetectionPane = NULL;
	}

	pCStViewerApp->LoadSaveNodeMapSettingFile(m_pIStImageDisplayWnd->GetINodeMap(), false, StConfigurationFileType_DisplayImageWnd, m_pIStDevice);
	pCStViewerApp->LoadSaveNodeMapSettingFile(m_pIStImageDisplayWnd->GetIStPixelFormatConverter()->GetINodeMap(), false, StConfigurationFileType_PixelFormatConverter, m_pIStDevice);

	if (m_pStViewerNodeMap.IsValid())
	{
		pCStViewerApp->LoadSaveNodeMapSettingFile(m_pStViewerNodeMap, false, StConfigurationFileType_StViewer, m_pIStDevice);
	}
	m_pIStImageDisplayWnd->Close();
	m_pIStDataStream.Reset(NULL);
	m_pIStRegisteredCallbackDeviceLost.Reset(NULL);
	m_pIStDevice.Reset(NULL);


	//Log
	AddDeviceLog(IDS_DEVICE_CLOSED);

	CStViewerDocBase::OnCloseDocument();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::StartImageAcquisition()
{
	StopImageAcquisition();
	
	try
	{
		//Streaming Buffer Count
		bool isAutoBufferCount = true;
		size_t nBufferCount = 8;	//Default frame buffer count
		if (m_pStViewerNodeMap.IsValid())
		{
			GenApi::CIntegerPtr pIInteger_TLParamsLocked(m_pStViewerNodeMap->GetNode("TLParamsLocked"));
			if (pIInteger_TLParamsLocked.IsValid())
			{
				pIInteger_TLParamsLocked->SetValue(1);
			}

			GenApi::CEnumerationPtr pIEnumeration_StreamBufferCountMode(m_pStViewerNodeMap->GetNode("StreamBufferCountMode"));
			if (pIEnumeration_StreamBufferCountMode.IsValid())
			{
				isAutoBufferCount = (pIEnumeration_StreamBufferCountMode->GetCurrentEntry()->GetSymbolic().compare("Automatic") == 0);
			}

			if (!isAutoBufferCount)
			{
				GenApi::CIntegerPtr pIInteger_StreamBufferCount(m_pStViewerNodeMap->GetNode("StreamBufferCount"));
				if (pIInteger_StreamBufferCount.IsValid())
				{
					nBufferCount = pIInteger_StreamBufferCount->GetValue(0);
				}
			}
		}
		if (isAutoBufferCount)
		{
			GenApi::CFloatPtr pIFloat_AcquisitionFrameRate(m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("AcquisitionFrameRate"));
			if (GenApi::IsReadable(pIFloat_AcquisitionFrameRate))
			{
				const double dblFrameRate = pIFloat_AcquisitionFrameRate->GetValue();
				const size_t nCandidateBufferCount = (size_t)(dblFrameRate / 100);	//10ms
				if (nBufferCount < nCandidateBufferCount)
				{
					nBufferCount = nCandidateBufferCount;
				}
				if (m_pStViewerNodeMap.IsValid())
				{
					GenApi::CIntegerPtr pIInteger_StreamBufferCount(m_pStViewerNodeMap->GetNode("StreamBufferCount"));
					pIInteger_StreamBufferCount->SetValue(nBufferCount, false);
				}
			}
		}
		m_pIStDataStream->SetStreamBufferCount(nBufferCount);


		//Clear received image count.
		m_nReceivedImageCount = 0;
		m_nDroppedIDCount = 0;
		m_nLastID = 0;
		m_bFirstFrame = true;

		// Start the image acquisition of the host side.
		m_pIStDataStream->StartAcquisition();
		m_IsAcquisitionRunning = true;
		
		if (m_pIStDevice->GetRemoteIStPort()->GetIStPortInfo()->IsAccessWrite())
		{
			// Start the image acquisition of the camera side.
			m_pIStDevice->AcquisitionStart();
		}


		//Log
		AddDeviceLog(IDS_STREAMING_STARTED);
	}
	catch(const GenICam::GenericException &e)
	{
		OnException(e);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::StopImageAcquisition()
{
	if(!IsAcquisitionRunning()) return;
	
	if(!m_pIStDevice.IsValid()) return;
	try
	{
		if (!m_IsDeviceLost)
		{
				if (m_pIStDevice->GetRemoteIStPort()->GetIStPortInfo()->IsAccessWrite())
				{
					// Stop the image acquisition of the camera side.
					m_pIStDevice->AcquisitionStop();
				}
				//Log
				AddDeviceLog(IDS_STREAMING_STOPPED);
		}
		// Stop the image acquisition of the host side.
		m_pIStDataStream->StopAcquisition();

		if (m_pStViewerNodeMap.IsValid())
		{
			GenApi::CIntegerPtr pIInteger_TLParamsLocked(m_pStViewerNodeMap->GetNode("TLParamsLocked"));
			if (pIInteger_TLParamsLocked.IsValid())
			{
				pIInteger_TLParamsLocked->SetValue(0);
			}
		}
		m_IsAcquisitionRunning = false;

#ifdef USE_DISPLAY_RELEASABLE_IMAGE_BUFFER
		m_pIStImageDisplayWnd->ReleaseRegisteredIStStreamBuffer();
#endif
	}
	catch(const GenICam::GenericException &e)
	{
		OnException(e);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::OnStCallback(StApi::IStCallbackParamBase *pIStCallbackParamBase, void * /*pvContext*/)
{
	try
	{
		StApi::EStCallbackType_t eStCallbackType = pIStCallbackParamBase->GetCallbackType();
		if(eStCallbackType  ==  StApi::StCallbackType_GenTLEvent_DataStreamNewBuffer)
		{

			StApi::IStCallbackParamGenTLEventNewBuffer *pIStCallbackParamGenTLEventNewBuffer = dynamic_cast<StApi::IStCallbackParamGenTLEventNewBuffer*>(pIStCallbackParamBase);
			StApi::IStDataStream *pIStDataStream = pIStCallbackParamGenTLEventNewBuffer->GetIStDataStream();

			// Wait until the data is acquired.
			// If the data has been received, IStStreamBufferReleasable interface pointer is retrieved.
			// When the received data is no longer needed, immediately call the IStStreamBufferReleasable::Release(), please return the buffer to the streaming queue.
			// In the destructor of CIStStreamBufferPtr, IStStreamBufferReleasable::Release() is called.
			StApi::CIStStreamBufferPtr pIStStreamBuffer(pIStDataStream->RetrieveBuffer(0));
			
			if(m_pIStImageDisplayWnd.IsValid())
			{

				const StApi::IStStreamBufferInfo *pIStStreamBufferInfo = pIStStreamBuffer->GetIStStreamBufferInfo();
				const uint64_t nFrameID = pIStStreamBufferInfo->GetFrameID();

				if (!m_bFirstFrame)
				{
					if (m_nLastID < nFrameID)
					{
						m_nDroppedIDCount += (nFrameID - m_nLastID - 1);
					}
				}
				m_nLastID = nFrameID;
				m_bFirstFrame = false;

				if (pIStStreamBufferInfo->IsIncomplete())
				{
					++m_nDroppedIDCount;
				}
				else
				{
					++m_nReceivedImageCount;
				}
				if (pIStStreamBufferInfo->IsImagePresent())
				{
					try
					{
						// Get the IStImage interface pointer to the acquired image data.
						StApi::IStImage *pIStImage = pIStStreamBuffer->GetIStImage();

						m_objCIStImageCallbackList.OnIStImage(pIStImage);

						if(m_pCSaveMultipleImagesFileBase)
						{
							GenApi::AutoLock autoLock(m_CLockForAVI);
							if (m_pCSaveMultipleImagesFileBase)
							{
								if (m_pCSaveMultipleImagesFileBase->RegisterIStStreamBuffer(pIStStreamBuffer))
								{
									StopRecording();
								}
							}
						}

						// Register the image to be displayed.
#ifdef USE_DISPLAY_RELEASABLE_IMAGE_BUFFER
							// Registered streaming buffer is to be released automatically.
						m_pIStImageDisplayWnd->RegisterIStStreamBuffer(pIStStreamBuffer.Move());
#else
							// Registered image is to be copied, the original data is no longer needed.
						m_pIStImageDisplayWnd->RegisterIStImage(pIStImage);
#endif
					}
					catch (...)
					{
					}
				}
			}

		}
		else if(eStCallbackType  ==  StApi::StCallbackType_GenTLEvent_DataStreamError)
		{
			StApi::IStCallbackParamGenTLEventErrorDS *pIStCallbackParamGenTLEventErrorDS = dynamic_cast<StApi::IStCallbackParamGenTLEventErrorDS*>(pIStCallbackParamBase);
			OutputDebugString(GCSTRING_2_LPCTSTR(pIStCallbackParamGenTLEventErrorDS->GetDescription()));
		}
	}
	catch(...)
	{
		//TODO:
	}
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::StartRecording()
{
	if(m_pCSaveMultipleImagesFileBase ==  NULL)
	{
		CSaveMultipleImagesFileBase *pCSaveMultipleImagesFileBase = NULL;
		if (IDYES == AfxMessageBox(L"Generate multiple still image files?", MB_ICONQUESTION | MB_YESNO))
		{
			pCSaveMultipleImagesFileBase = new CStillImageFiles();
		}
		else
		{
			pCSaveMultipleImagesFileBase = new CAVIFile();
		}
		try
		{
			if (!pCSaveMultipleImagesFileBase->Open(m_pIStDevice, m_pIStImageDisplayWnd))
			{
				delete pCSaveMultipleImagesFileBase;
			}
			else
			{
				GenApi::AutoLock autoLock(m_CLockForAVI);
				m_pCSaveMultipleImagesFileBase = pCSaveMultipleImagesFileBase;
			}
		}
		catch (const GenICam::GenericException &e)
		{
			delete pCSaveMultipleImagesFileBase;
			OnException(e);
		}
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::StopRecording()
{
	GenApi::AutoLock autoLock(m_CLockForAVI);
	if(m_pCSaveMultipleImagesFileBase)
	{
		delete m_pCSaveMultipleImagesFileBase;
		m_pCSaveMultipleImagesFileBase = NULL;
	}
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::CameraConfigFile(BOOL isOpenMode)
{
	try
	{
		CFileDialog dlgLoad(isOpenMode,	TEXT(".cfg"), NULL,	OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,	TEXT("CFG File(*.cfg)|*.cfg|AllFiles(*.*)|*.*||"));
		dlgLoad.m_ofn.lpstrTitle = TEXT("Camera Config File");
		if (dlgLoad.DoModal() == IDOK)
		{
			LPCTSTR szFileName = dlgLoad.m_ofn.lpstrFile;
			const GenICam::gcstring strFileName(szFileName);
			StApi::CIStFeatureBagPtr pIStFeatureBag(StApi::CreateIStFeatureBag());

			// Get the INodeMap interface pointer for the camera settings.
			GenApi::CNodeMapPtr pINodeMapRemote(m_pIStDevice->GetRemoteIStPort()->GetINodeMap());

			if (isOpenMode)
			{
				pIStFeatureBag->StoreFileToBag(strFileName);
				GenICam::gcstring_vector strErrorList;
				pIStFeatureBag->Load(pINodeMapRemote, true, &strErrorList);
				if (0 < strErrorList.size())
				{
					const size_t nMaxDispCount = 5;
					CString strMsg;
					strMsg = TEXT("Failed to load following nodes.");
					for (size_t i = 0; i < min(strErrorList.size(), nMaxDispCount); ++i)
					{
						strMsg.Append(TEXT("\n"));
						strMsg.Append(GCSTRING_2_LPCTSTR(strErrorList[i]));
					}
					if (nMaxDispCount < strErrorList.size())
					{
						strMsg.Append(TEXT("\n..."));
					}
					AfxMessageBox(strMsg, MB_ICONERROR);
				}
			}
			else
			{
				pIStFeatureBag->StoreNodeMapToBag(pINodeMapRemote);
				pIStFeatureBag->SaveToFile(strFileName);
			}
		}
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::SaveCameraDescriptionFile()
{
	try
	{
		// Get the IStPort interface pointer for the remote camera.
		StApi::IStPort *pIStPort = m_pIStDevice->GetRemoteIStPort();
		const GenICam::gcstring strFileName(pIStPort->GetXMLFileName());
		

		CFileDialog dlgSave(FALSE, NULL, strFileName.w_str().c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, TEXT("AllFiles(*.*)|*.*||"));
		dlgSave.m_ofn.lpstrTitle = TEXT("Camera description file");
		if (dlgSave.DoModal() == IDOK)
		{
			GenICam::gcstring strFullPath(dlgSave.m_ofn.lpstrFile);
			pIStPort->SaveXMLFile(strFullPath);
		}
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::OnUpdateCommandHandler(CCmdUI *pCmdUI)
{
	BOOL bEnable = FALSE;
	switch (pCmdUI->m_nID)
	{
	case(ID_ACQ_START) :			bEnable = !IsAcquisitionRunning();				break;
	case(ID_ACQ_STOP) :				bEnable = IsAcquisitionRunning();				break;
	case(ID_SAVE_STILL_IMAGE) :		bEnable = m_pIStImageDisplayWnd->HasImage();	break;
	case(ID_START_RECORDING) :		bEnable = !IsRecording();						break;
	case(ID_STOP_RECORDING) :		bEnable = IsRecording();						break;
	case(ID_FILE_SAVE_CAMERA_CONFIG_FILE):	bEnable = TRUE; 	break;
	case(ID_FILE_LOAD_CAMERA_CONFIG_FILE):	bEnable = TRUE; 	break;
	case(ID_FILE_SAVE_CAMERA_DESCRIPTION_FILE):	bEnable = TRUE;  break;
	}
	pCmdUI->Enable(bEnable);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::OnCommandHandler(UINT nID)
{
	switch (nID)
	{
	case(ID_ACQ_START) : StartImageAcquisition();		break;
	case(ID_ACQ_STOP) : StopImageAcquisition();		break;
	case(ID_SAVE_STILL_IMAGE) :		SaveStillImage();		break;
	case(ID_START_RECORDING) : StartRecording();		break;
	case(ID_STOP_RECORDING) : StopRecording();		break;
	case(ID_FILE_SAVE_CAMERA_CONFIG_FILE) : CameraConfigFile(FALSE);		break;
	case(ID_FILE_LOAD_CAMERA_CONFIG_FILE) : CameraConfigFile(TRUE);		break;
	case(ID_FILE_SAVE_CAMERA_DESCRIPTION_FILE) : SaveCameraDescriptionFile(); break;
	}

}

//-----------------------------------------------------------------------------
//Get the current FPS value.
//-----------------------------------------------------------------------------
void CStViewerDoc::GetFPSString(CString &strText)
{
	if (m_pIStDataStream)
	{
		//Get the current FPS value.
		double dblFPS = m_pIStDataStream->GetCurrentFPS();

		//Get the current BPS value.
		double dblBPS = m_pIStDataStream->GetCurrentBPS();

#if 1
		LPCTSTR pszUnit[] = { TEXT("bps"), TEXT("kbps"), TEXT("Mbps"), TEXT("Gbps"), TEXT("Tbps") };
		const double dblUnit = 1000.0;
		dblBPS *= 8;
#elif 1
		LPCTSTR pszUnit[] = { TEXT("Bps"), TEXT("kBps"), TEXT("MBps"), TEXT("GBps"), TEXT("TBps") };
		const double dblUnit = 1000.0;
#else
		LPCTSTR pszUnit[] = { TEXT("Bps"), TEXT("KiBps"), TEXT("MiBps"), TEXT("GiBps"), TEXT("TiBps") };
		const double dblUnit = 1024.0;
#endif
		const double dblThresh = dblUnit * 10;

		size_t iUnitIndex = 0;
		for (;;)
		{
			if ((dblBPS <= dblThresh) || (_countof(pszUnit) + 1 <= iUnitIndex))
			{
				break;
			}

			dblBPS /= dblUnit;
			++iUnitIndex;
		}

		//Convert to a string.
		strText.Format(TEXT("%.2lf[fps] / %.2lf[%s]"), dblFPS, dblBPS, pszUnit[iUnitIndex]);
	}
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::OnDeviceLost(GenApi::INode *pINode, void*)
{

	if (GenApi::IsAvailable(pINode))
	{
		if (m_pIStDevice->IsDeviceLost())
		{
			m_IsDeviceLost = true;
			POSITION pos = GetFirstViewPosition();
			GetNextView(pos)->PostMessage(WM_DEVICE_LOST, (WPARAM)this, NULL);
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::OnFileStartFfc()
{
	bool bReval = false;
	
	if (AfxMessageBox(IDS_STRING_FFC_START, MB_ICONQUESTION | MB_OKCANCEL) != IDOK)
	{
		return;
	}

	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	try
	{
		const bool isRunning = IsAcquisitionRunning();

		if (isRunning)
		{
			StopImageAcquisition();
		}

		{
			CCameraSideFFC objCCameraSideFFC(m_pIStDevice);
			m_objCIStImageCallbackList.Add(&objCCameraSideFFC);
			try
			{
				StartImageAcquisition();
				objCCameraSideFFC.Wait(30000);
				StopImageAcquisition();

				objCCameraSideFFC.CalculateAndSend();
				bReval = true;
			}
			catch (const GenICam::GenericException &e)
			{
				OnException(e);
			}
			m_objCIStImageCallbackList.Remove(&objCCameraSideFFC);
		}

		if (isRunning)
		{
			StartImageAcquisition();
		}
		SetCursor(hCursor);
	}
	catch (const GenICam::GenericException &e)
	{
		SetCursor(hCursor);
		OnException(e);
	}

	AfxMessageBox(bReval ? IDS_STRING_FFC_END_SUCCEEDED : IDS_STRING_FFC_END_FAILED, MB_ICONINFORMATION);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::OnUpdateFileStartFfc(CCmdUI *pCmdUI)
{
	if (!CCameraSideFFC::IsSupported(m_pIStDevice->GetRemoteIStPort()->GetINodeMap()))
	{
		if (pCmdUI->m_pMenu != NULL)
		{
			pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nID, MF_BYCOMMAND);
		}
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::OnGraphDataSourceCommandHandler(CCmdUI *pCmdUI)
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
void CStViewerDoc::OnGraphDataSource(UINT nID)
{
	const bool isRegisteredImage = (nID == ID_GRAPHDATASOURCE_REGISTEREDIMAGE);
	m_pCGraphView->SetGraphDataSource(isRegisteredImage);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::GetStatusBarText(size_t nIndex, CString &strText)
{
	if (nIndex == 0)
	{
		//Received Image Count
		const uint64_t nRcvCount = GetReceivedImageCount();
		const uint64_t nDroppedCount = GetDroppedIDFrameCount();
		strText.Format(TEXT("Received=%I64u[Dropped=%I64u]"), nRcvCount, nDroppedCount);
	}
	else if(nIndex == 1)
	{
		//FPS
		GetFPSString(strText);
	}
	else if (nIndex == 2)
	{
		GetPixelInfoText(m_pIStImageDisplayWnd, strText);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CStViewerDoc::OnOpenDocument(LPCTSTR)
{
	return FALSE;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::RegisterNodesToView()
{
	//Get the NodeMap of the device port.
	if (m_pIStDevice)
	{
#ifdef ENABLE_INTERFACE_SPECIFIC_NODE_MAP
		//For ActionCommand
		if (m_pIStDevice->GetIStInterface()->GetIStInterfaceInfo()->GetTLType().compare(TLTypeGEVName) == 0)
		{
			GenApi::CNodeMapPtr pINodeMapForInterface(m_pIStDevice->GetIStInterface()->GetIStPort()->GetINodeMap());
			GenICam::gcstring strTitle("Interface");
			m_vecRegisteredINodeHandles.push_back(m_pCNodeMapView->RegisterINode(pINodeMapForInterface->GetNode("ActionControl"), strTitle));
			m_vecRegisteredINodeHandles.push_back(m_pCNodeMapView->RegisterINode(pINodeMapForInterface->GetNode("EventControl"), strTitle));
		}
#endif //ENABLE_INTERFACE_SPECIFIC_NODE_MAP
		//Get the NodeMap of the device local port.
		StApi::IStPort *pIPortLocal = m_pIStDevice->GetLocalIStPort();
		if (pIPortLocal)
		{
			GenApi::CNodeMapPtr pINodeMap(pIPortLocal->GetINodeMap());
			GenICam::gcstring strTitle("Local Device");
			m_vecRegisteredINodeHandles.push_back(m_pCNodeMapView->RegisterINode(pINodeMap->GetNode("Root"), strTitle));
		}

		//Get the NodeMap of the device remote port.
		StApi::IStPort *pIPortRemote = m_pIStDevice->GetRemoteIStPort();
		if (pIPortRemote)
		{
			GenApi::CNodeMapPtr pINodeMap(pIPortRemote->GetINodeMap());
			GenICam::gcstring strTitle("Remote Device");
			m_vecRegisteredINodeHandles.push_back(m_pCNodeMapView->RegisterINode(pINodeMap->GetNode("Root"), strTitle));
		}
	}

	//Get the NodeMap of the data stream port.
	if (m_pIStDataStream)
	{
		StApi::IStPort *pIPortDataStream = m_pIStDataStream->GetIStPort();
		if (pIPortDataStream)
		{
			try
			{
				GenApi::CNodeMapPtr pINodeMap(pIPortDataStream->GetINodeMap());
				GenICam::gcstring strTitle("Data Stream");
				m_vecRegisteredINodeHandles.push_back((m_pCNodeMapView->RegisterINode(pINodeMap->GetNode("Root"), strTitle)));


				if (m_pStViewerNodeMap.IsValid())
				{
					m_vecRegisteredINodeHandles.push_back((m_pCNodeMapView->RegisterINode(m_pStViewerNodeMap->GetNode("DataStream"), strTitle)));
				}
			}
			catch (const GenICam::GenericException &)
			{

			}
			catch (...)
			{

			}
		}
	}

	CStViewerDocSingleDocBase::RegisterNodesToView();

	if (m_pCDefectivePixelDetectionPane)
	{
		GenApi::CNodeMapPtr pINodeMap(m_pCDefectivePixelDetectionPane->GetINodeMapForDefectivePixelDetectionFilter());
		m_pCNodeMapView->RegisterINode(pINodeMap->GetNode("Root"), "Defective Pixel Detection Filter");
	}

	m_pCNodeMapView->Refresh(true);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStViewerDoc::CreateStViewerNodeMap()
{
	const HMODULE hModule = AfxGetApp()->m_hInstance;
	HRSRC hRSrc = FindResource(hModule, MAKEINTRESOURCE(IDR_XML_STVIEWER), TEXT("XML"));
	if (hRSrc == NULL) return;
	
	HGLOBAL	hXMLResource = LoadResource(hModule, hRSrc);
	if (hXMLResource == NULL) return;

	
	const size_t	nResourceSize = SizeofResource(hModule, hRSrc);
	try
	{
		std::vector<char> vecXMLData;
		vecXMLData.resize(nResourceSize + 1);
		vecXMLData[vecXMLData.size() - 1] = '\0';

		LPVOID pvRes = LockResource(hXMLResource);
		memcpy_s(&vecXMLData[0], vecXMLData.size(), pvRes, nResourceSize);

		m_pStViewerNodeMapRef._LoadXMLFromString(&vecXMLData[0]);
		m_pStViewerNodeMap = m_pStViewerNodeMapRef._Ptr;
	}
	catch (std::bad_alloc &)
	{
	}
	catch (const GenICam::GenericException &)
	{

	}
	catch (...)
	{

	}
	FreeResource(hXMLResource);
}