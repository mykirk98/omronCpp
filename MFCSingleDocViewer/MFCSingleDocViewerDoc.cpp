
// MFCSingleDocViewerDoc.cpp : implementation of the CMFCSingleDocViewerDoc class
//

#include "stdafx.h"
#include "MFCSingleDocViewer.h"
#include "MFCSingleDocViewerDoc.h"
#include "PropertiesWnd.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace GenTL;
using namespace StApi;

// CMFCSingleDocViewerDoc

IMPLEMENT_DYNCREATE(CMFCSingleDocViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CMFCSingleDocViewerDoc, CDocument)
	ON_COMMAND(ID_CAMERA_ACQ_START, &CMFCSingleDocViewerDoc::OnCameraAcqStart)
	ON_COMMAND(ID_CAMERA_ACQ_STOP, &CMFCSingleDocViewerDoc::OnCameraAcqStop)
	ON_COMMAND(ID_CAMERA_SNAP, &CMFCSingleDocViewerDoc::OnCameraSnap)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_ACQ_STOP, &CMFCSingleDocViewerDoc::OnUpdateCameraAcqStop)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_ACQ_START, &CMFCSingleDocViewerDoc::OnUpdateCameraAcqStart)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &CMFCSingleDocViewerDoc::OnUpdateFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_SNAP, &CMFCSingleDocViewerDoc::OnUpdateCameraSnap)
	ON_COMMAND(ID_PROPERTIES_WND, &CMFCSingleDocViewerDoc::OnPropertiesWnd)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES_WND, &CMFCSingleDocViewerDoc::OnUpdatePropertiesWnd)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// CMFCSingleDocViewerDoc construction
//-----------------------------------------------------------------------------
CMFCSingleDocViewerDoc::CMFCSingleDocViewerDoc() : 
	m_pIStPixelFormatConverter(CreateIStConverter(StConverterType_PixelFormat)), 
	m_hWnd(NULL), m_isStreamingStarted(false), m_isDeviceLostDetected(false)
{
	m_vecBitmapInfo.resize(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256);
	memset(&m_vecBitmapInfo[0], 0, m_vecBitmapInfo.size());
	BITMAPINFO *psBitmapInfo = (BITMAPINFO*)&m_vecBitmapInfo[0];
	psBitmapInfo->bmiHeader.biPlanes = 1;
	psBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	uint32_t dwColor = 0;
	uint32_t *pdwPallete = (uint32_t*)psBitmapInfo->bmiColors;
	for (size_t i = 0; i < 256; ++i)
	{
		*pdwPallete++ = dwColor;
		dwColor += 0x010101;
	}

	psBitmapInfo->bmiHeader.biCompression = BI_RGB;
	psBitmapInfo->bmiHeader.biBitCount = 8;
	psBitmapInfo->bmiHeader.biClrImportant = psBitmapInfo->bmiHeader.biClrUsed = 256;
	psBitmapInfo->bmiHeader.biHeight = 1;
	psBitmapInfo->bmiHeader.biSizeImage = 1;
	psBitmapInfo->bmiHeader.biWidth = 1;
}

//-----------------------------------------------------------------------------
// CMFCSingleDocViewerDoc destruction
//-----------------------------------------------------------------------------
CMFCSingleDocViewerDoc::~CMFCSingleDocViewerDoc()
{
	CloseDevice();
}


#ifdef SHARED_HANDLERS

// Support for thumbnails
void CMFCSingleDocViewerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CMFCSingleDocViewerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CMFCSingleDocViewerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CMFCSingleDocViewerDoc diagnostics

#ifdef _DEBUG
void CMFCSingleDocViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMFCSingleDocViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMFCSingleDocViewerDoc commands

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CMFCSingleDocViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;


	return(TRUE);
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::CloseDevice()
{
	if (m_pIStDevice.IsValid())
	{
		m_wndProperties.DestroyWindow();
		CFrameWndEx *pCFrameWndEx = dynamic_cast<CFrameWndEx*>(AfxGetApp()->GetMainWnd());
		if (pCFrameWndEx != NULL)
		{
			pCFrameWndEx->AdjustDockingLayout();
		}

		if (m_pIStDataStream.IsValid())
		{
			StopStreaming();
			m_pIStDataStream.Reset(NULL);
		}
		m_pIStRegisteredCallbackDeviceLost.Reset(NULL);
		m_pIStDevice.Reset(NULL);
		m_isStreamingStarted = false;
		m_isDeviceLostDetected = false;

		SetTitle(TEXT(""));
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::OpenDevice(StApi::IStDeviceReleasable *pIStDeviceReleasable)
{
	m_pIStDevice = pIStDeviceReleasable;
	if (!m_pIStDevice.IsValid())
	{
		return;
	}

	try
	{
		GenApi::CNodeMapPtr pINodeMapRemoteDevice(m_pIStDevice->GetRemoteIStPort()->GetINodeMap());

		//Register device lost event.
		GenApi::CNodeMapPtr pINodeMapLocalDevice(m_pIStDevice->GetLocalIStPort()->GetINodeMap());
		if (pINodeMapLocalDevice)
		{
			GenApi::CNodePtr pINodeEventDeviceLost(pINodeMapLocalDevice->GetNode("EventDeviceLost"));
			if (pINodeEventDeviceLost.IsValid())
			{
				m_pIStRegisteredCallbackDeviceLost.Reset(StApi::RegisterCallback(pINodeEventDeviceLost, *this, &CMFCSingleDocViewerDoc::OnDeviceLost, (void*)NULL, GenApi::cbPostOutsideLock));
			}
		}

		//Start Event Acquisition Thread
		m_pIStDevice->StartEventAcquisitionThread();

		//Set title.
		const GenICam::gcstring strDisplayName = m_pIStDevice->GetIStDeviceInfo()->GetDisplayName();
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
		if (0 < strUserName.length())
		{
			strTitle.Format(TEXT("%s[%s]"), GCSTRING_2_LPCTSTR(strDisplayName), GCSTRING_2_LPCTSTR(strUserName));
		}
		else
		{
			strTitle = GCSTRING_2_LPCTSTR(strDisplayName);
		}
		SetTitle(strTitle);

		//Create object and get IStDataStream pointer.
		m_pIStDataStream.Reset(m_pIStDevice->CreateIStDataStream(0));
		if (m_pIStDataStream.IsValid())
		{
			//Register callback function to receive images.
			RegisterCallback(m_pIStDataStream, *this, &CMFCSingleDocViewerDoc::OnStCallback, (void*)NULL);
		}

		BOOL bNameValid;
		// Create properties window
		CString strPropertiesWnd;
		bNameValid = strPropertiesWnd.LoadString(ID_PROPERTIES_WND);
		ASSERT(bNameValid);
		CFrameWndEx *pCFrameWndEx = dynamic_cast<CFrameWndEx*>(AfxGetApp()->GetMainWnd());
		if (!m_wndProperties.Create(strPropertiesWnd, pCFrameWndEx, CRect(0, 0, 200, 600), TRUE, ID_PROPERTIES_WND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
		{
			TRACE0("Failed to create Properties window\n");
			return;
		}
		HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_PROPERTIES_WND_HC), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
		m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);
		m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
		pCFrameWndEx->DockPane(&m_wndProperties);
		m_wndProperties.InitProperties(m_pIStDevice, m_pIStDataStream);
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::OnStCallback(StApi::IStCallbackParamBase *pIStCallbackParamBase, void * /*pvContext*/)
{
	try
	{
		StApi::EStCallbackType_t eStCallbackType = pIStCallbackParamBase->GetCallbackType();
		if (eStCallbackType == StApi::StCallbackType_GenTLEvent_DataStreamNewBuffer)
		{

			StApi::IStCallbackParamGenTLEventNewBuffer *pIStCallbackParamGenTLEventNewBuffer = dynamic_cast<StApi::IStCallbackParamGenTLEventNewBuffer*>(pIStCallbackParamBase);
			StApi::IStDataStream *pIStDataStream = pIStCallbackParamGenTLEventNewBuffer->GetIStDataStream();

			// Wait until the data is acquired.
			// If the data has been received, IStStreamBufferReleasable interface pointer is retrieved.

			GenApi::AutoLock objAutoLock(m_objLockForStreamingBuffer);
			m_pIStStreamBuffer.Reset(pIStDataStream->RetrieveBuffer(0));

			PostMessage(GetWndHandle(), WM_NEW_IMAGE, (WPARAM)this, NULL);
		}
		else if (eStCallbackType == StApi::StCallbackType_GenTLEvent_DataStreamError)
		{
			StApi::IStCallbackParamGenTLEventErrorDS *pIStCallbackParamGenTLEventErrorDS = dynamic_cast<StApi::IStCallbackParamGenTLEventErrorDS*>(pIStCallbackParamBase);
			OutputDebugString(GCSTRING_2_LPCTSTR(pIStCallbackParamGenTLEventErrorDS->GetDescription()));
		}
	}
	catch (...)
	{
		//TODO:
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::OnDeviceLost(GenApi::INode *pINode, void*)
{

	if (GenApi::IsAvailable(pINode))
	{
		if (m_pIStDevice->IsDeviceLost())
		{
			m_isDeviceLostDetected = true;
			PostMessage(GetWndHandle(), WM_DEVICE_LOST, (WPARAM)this, NULL);
		}
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::OnCameraAcqStart()
{
	StopStreaming();
	StartStreaming();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::OnCameraAcqStop()
{
	StopStreaming();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::OnCameraSnap()
{
	StopStreaming();
	StartStreaming(1);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::StartStreaming(uint64_t nFrameCount)
{
	if (!m_pIStDevice.IsValid()) return;
	if (!m_pIStDataStream.IsValid()) return;
	try
	{
		// Start the image acquisition of the host side.
		m_pIStDataStream->StartAcquisition(nFrameCount);

		// Start the image acquisition of the camera side.
		m_pIStDevice->AcquisitionStart();

		m_isStreamingStarted = true;
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::StopStreaming()
{
	if (!m_pIStDevice.IsValid()) return;
	if (!m_pIStDataStream.IsValid()) return;
	try
	{
		// Stop the image acquisition of the camera side.
		m_pIStDevice->AcquisitionStop();

		// Stop the image acquisition of the host side.
		m_pIStDataStream->StopAcquisition();

		m_isStreamingStarted = false;

		ConvertImageToVisibleFormat();
	}
	catch (const GenICam::GenericException &e)
	{
		if (!m_isDeviceLostDetected)
		{
			OnException(e);
		}
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::ConvertImageToVisibleFormat()
{
	GenApi::AutoLock objAutoLock(m_objLockForStreamingBuffer);
	if (m_pIStStreamBuffer.IsValid())
	{
		if (!m_pIStImageBuffer.IsValid())
		{
			m_pIStImageBuffer = CreateIStImageBuffer();
		}

		IStImage *pIStImage = m_pIStStreamBuffer->GetIStImage();
		const EStPixelFormatNamingConvention_t ePFNCSrc = pIStImage->GetImagePixelFormat();
		const IStPixelFormatInfo *pIStPixelFormatInfo = GetIStPixelFormatInfo(ePFNCSrc);

		const EStPixelFormatNamingConvention_t ePFNCDest = pIStPixelFormatInfo->IsColor() ? StPFNC_BGR8 : StPFNC_Mono8;

		m_pIStPixelFormatConverter->SetDestinationPixelFormat(ePFNCDest);
		m_pIStPixelFormatConverter->Convert(pIStImage, m_pIStImageBuffer);

		m_pIStStreamBuffer.Reset(NULL);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CMFCSingleDocViewerDoc::GetBitmapImage(CDC *pDC, CBitmap &objCBitmap)
{
	ConvertImageToVisibleFormat();
	if (!m_pIStImageBuffer.IsValid()) return(FALSE);
	{
		IStImage *pIStImage = m_pIStImageBuffer->GetIStImage();
		const int nImageWidth = (int)pIStImage->GetImageWidth();
		const int nImageHeight = (int)pIStImage->GetImageHeight();
		const int nImageLinePitch = (int)pIStImage->GetImageLinePitch();
		const EStPixelFormatNamingConvention_t ePFNC = pIStImage->GetImagePixelFormat();

		objCBitmap.CreateCompatibleBitmap(pDC, nImageWidth, nImageHeight);

		BITMAPINFO *psBitmapInfo = (BITMAPINFO*)&m_vecBitmapInfo[0];

		if (ePFNC == StPFNC_Mono8)
		{
			psBitmapInfo->bmiHeader.biBitCount = 8;
			psBitmapInfo->bmiHeader.biClrUsed = 256;
		}
		else if (ePFNC == StPFNC_BGR8)
		{
			psBitmapInfo->bmiHeader.biBitCount = 24;
			psBitmapInfo->bmiHeader.biClrUsed = 0;
		}
		else
		{
			psBitmapInfo->bmiHeader.biBitCount = 32;
			psBitmapInfo->bmiHeader.biClrUsed = 0;
		}
		psBitmapInfo->bmiHeader.biHeight = -nImageHeight;
		psBitmapInfo->bmiHeader.biSizeImage = nImageHeight * nImageLinePitch;
		psBitmapInfo->bmiHeader.biWidth = nImageWidth;

		SetDIBits(pDC->GetSafeHdc(), (HBITMAP)objCBitmap.GetSafeHandle(), 0, nImageHeight, m_pIStImageBuffer->GetBufferPtr(), psBitmapInfo, DIB_RGB_COLORS);
		objCBitmap.SetBitmapDimension(nImageWidth, nImageHeight);

	}
	return(TRUE);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CMFCSingleDocViewerDoc::GetLatestImage(StApi::IStImageBuffer *pIStImageBuffer)
{
	ConvertImageToVisibleFormat();
	if (!m_pIStImageBuffer.IsValid()) return(FALSE);
	pIStImageBuffer->CopyImage(m_pIStImageBuffer->GetIStImage());
	return(TRUE);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HWND CMFCSingleDocViewerDoc::GetWndHandle()
{
	if (m_hWnd) return(m_hWnd);

	POSITION pos = GetFirstViewPosition();
	m_hWnd = GetNextView(pos)->GetSafeHwnd();
	return(m_hWnd);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::OnUpdateCameraSnap(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsDeviceOpened() && (!IsStreamingStarted()));
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::OnUpdateCameraAcqStart(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsDeviceOpened() && (!IsStreamingStarted()));
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::OnUpdateCameraAcqStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsDeviceOpened() && IsStreamingStarted());
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::OnUpdateFileSaveAs(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pIStImageBuffer.IsValid());
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::OnPropertiesWnd()
{
	if (IsDeviceOpened())
	{
		CFrameWndEx *pCFrameWndEx = dynamic_cast<CFrameWndEx*>(AfxGetApp()->GetMainWnd());
		m_wndProperties.ShowWindow(m_wndProperties.IsVisible() ? SW_HIDE : SW_SHOW);
		pCFrameWndEx->DockPane(&m_wndProperties);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CMFCSingleDocViewerDoc::OnUpdatePropertiesWnd(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsDeviceOpened());
	if (IsDeviceOpened())
	{
		pCmdUI->SetCheck(m_wndProperties.IsVisible());
	}
}
