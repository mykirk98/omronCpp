// CEachDeviceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFCMultiDlgViewer.h"
#include "CEachDeviceDlg.h"
#include "afxdialogex.h"

using namespace GenTL;
using namespace StApi;

static UINT pnIndicators[] =
{
	//ID_SEPARATOR,           // status line indicator
	IDS_STATUS_FPS,
	IDS_STATUS_FRAME_COUNT,
	IDS_STATUS_DROP_COUNT,
};

// CEachDeviceDlg dialog
IMPLEMENT_DYNAMIC(CEachDeviceDlg, CDialogEx)

//-----------------------------------------------------------------------------
// CEachDeviceDlg construction
//-----------------------------------------------------------------------------
CEachDeviceDlg::CEachDeviceDlg(StApi::IStDeviceReleasable *pIStDevice, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DEVICE, pParent),
	m_pIStPixelFormatConverter(CreateIStConverter(StConverterType_PixelFormat)), 
	m_isStreamingStarted(false), m_isDeviceLostDetected(false), m_isNewImage(false), m_nTimerID(0), m_nDroppedFrameCount(0)
{
	OpenDevice(pIStDevice);
	
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
// CEachDeviceDlg destruction
//-----------------------------------------------------------------------------
CEachDeviceDlg::~CEachDeviceDlg()
{
	CloseDevice();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_IMAGE, m_btnImage);
}


BEGIN_MESSAGE_MAP(CEachDeviceDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ACQ_START, &CEachDeviceDlg::OnBnClickedButtonAcqStart)
	ON_BN_CLICKED(IDC_BUTTON_ACQ_STOP, &CEachDeviceDlg::OnBnClickedButtonAcqStop)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CEachDeviceDlg::OnBnClickedButtonClose)
	ON_WM_DRAWITEM()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BUTTON_SNAP, &CEachDeviceDlg::OnBnClickedButtonSnap)
END_MESSAGE_MAP()


// CEachDeviceDlg message handlers

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::CloseDevice()
{
	if (m_pIStDevice.IsValid())
	{
		if (m_pIStDataStream.IsValid())
		{
			StopStreaming();
			m_pIStDataStream.Reset(NULL);
		}
		m_pIStRegisteredCallbackDeviceLost.Reset(NULL);
		m_pIStDevice.Reset(NULL);
		m_isStreamingStarted = false;
		m_isDeviceLostDetected = false;
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::OpenDevice(StApi::IStDeviceReleasable *pIStDeviceReleasable)
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
				m_pIStRegisteredCallbackDeviceLost.Reset(StApi::RegisterCallback(pINodeEventDeviceLost, *this, &CEachDeviceDlg::OnDeviceLost, (void*)NULL, GenApi::cbPostOutsideLock));
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
		if (0 < strUserName.length())
		{
			m_strTitle.Format(TEXT("%s[%s]"), GCSTRING_2_LPCTSTR(strDisplayName), GCSTRING_2_LPCTSTR(strUserName));
		}
		else
		{
			m_strTitle = GCSTRING_2_LPCTSTR(strDisplayName);
		}

		//Create object and get IStDataStream pointer.
		m_pIStDataStream.Reset(m_pIStDevice->CreateIStDataStream(0));
		if (m_pIStDataStream.IsValid())
		{
			//Register callback function to receive images.
			RegisterCallback(m_pIStDataStream, *this, &CEachDeviceDlg::OnStCallback, (void*)NULL);
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
void CEachDeviceDlg::OnStCallback(StApi::IStCallbackParamBase *pIStCallbackParamBase, void * /*pvContext*/)
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
			IStStreamBufferReleasable *pIStStreamBufferReleasable = pIStDataStream->RetrieveBuffer(0);
			if (pIStStreamBufferReleasable == NULL) return;
			if (pIStStreamBufferReleasable->GetIStStreamBufferInfo()->IsIncomplete())
			{
				pIStStreamBufferReleasable->Release();
				return;
			}

			GenApi::AutoLock objAutoLock(m_objLockForStreamingBuffer);

			if (m_pIStStreamBuffer.IsValid())
			{
				m_nDroppedFrameCount += pIStStreamBufferReleasable->GetIStStreamBufferInfo()->GetFrameID() - m_pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID() - 1;
			}
			m_pIStStreamBuffer.Reset(pIStStreamBufferReleasable);

			m_isNewImage = true;
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
void CEachDeviceDlg::OnDeviceLost(GenApi::INode *pINode, void*)
{

	if (GenApi::IsAvailable(pINode))
	{
		if (m_pIStDevice->IsDeviceLost())
		{
			m_isDeviceLostDetected = true;
			GetParent()->PostMessage(WM_DEVICE_LOST, (WPARAM)this, NULL);
		}
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::StartStreaming(uint64_t nFrameCount)
{
	if (!m_pIStDevice.IsValid()) return;
	if (!m_pIStDataStream.IsValid()) return;
	try
	{
		m_nDroppedFrameCount = 0;

		// Start the image acquisition of the host side.
		m_pIStDataStream->StartAcquisition(nFrameCount);

		// Start the image acquisition of the camera side.
		m_pIStDevice->AcquisitionStart();

		m_isStreamingStarted = true;
		UpdateButtonState();
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::StopStreaming()
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
		UpdateButtonState();

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
void CEachDeviceDlg::ConvertImageToVisibleFormat()
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
BOOL CEachDeviceDlg::GetBitmapImage(CDC *pDC, CBitmap &objCBitmap)
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
BOOL CEachDeviceDlg::GetLatestImage(StApi::IStImageBuffer *pIStImageBuffer)
{
	ConvertImageToVisibleFormat();
	if (!m_pIStImageBuffer.IsValid()) return(FALSE);
	pIStImageBuffer->CopyImage(m_pIStImageBuffer->GetIStImage());
	return(TRUE);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CEachDeviceDlg::DestroyWindow()
{
	if (m_nTimerID)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}
	GetParent()->PostMessage(WM_DEVICE_WND_CLOSED, (WPARAM)this, NULL);
	const BOOL ret = CDialogEx::DestroyWindow();
	delete this;
	return(ret);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::OnBnClickedButtonAcqStart()
{
	StopStreaming();
	StartStreaming();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::OnBnClickedButtonSnap()
{
	StopStreaming();
	StartStreaming(1);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::OnBnClickedButtonAcqStop()
{
	StopStreaming();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::OnBnClickedButtonClose()
{
	DestroyWindow();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::UpdateButtonState()
{
	if (GetSafeHwnd())
	{
		struct
		{
			UINT nID;
			bool isEnabled;
		} psBtnState[] = {
			{IDC_BUTTON_ACQ_START, !m_isStreamingStarted},
			{IDC_BUTTON_ACQ_STOP, m_isStreamingStarted}
		};
		for (size_t i = 0; i < _countof(psBtnState); ++i)
		{

			CWnd *pWnd = GetDlgItem(psBtnState[i].nID);
			if (pWnd)
			{
				pWnd->EnableWindow(psBtnState[i].isEnabled);
			}
		}
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CEachDeviceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_nTimerID = SetTimer(m_nTimerID, 33, NULL);

	SetDlgItemText(IDC_EDIT_DEVICE_NAME, m_strTitle);

	UpdateButtonState();

	m_pIStNodeMapDisplayWnd = StApi::CreateIStWnd(StApi::StWindowType_NodeMapDisplay);

	GenApi::CNodeMapPtr pINodeMapRemoteDevice(m_pIStDevice->GetRemoteIStPort()->GetINodeMap());

	try
	{
		const char *pszNames[] =
		{
			"TestImageType",
			"PixelFormat",
			"Width",
			"Height",
			"AcquisitionFrameRate",
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
	m_pIStNodeMapDisplayWnd->SetVisibleAlphabeticMode(false);
	m_pIStNodeMapDisplayWnd->SetVisibleCollapse(false);
	m_pIStNodeMapDisplayWnd->SetVisibleDescription(false);
	m_pIStNodeMapDisplayWnd->SetVisibleExpand(false);
	m_pIStNodeMapDisplayWnd->SetVisibleFilter(false);
	m_pIStNodeMapDisplayWnd->SetVisibleMenu(false);
	m_pIStNodeMapDisplayWnd->SetVisiblePolling(false);
	m_pIStNodeMapDisplayWnd->SetVisibleRefresh(false);
	m_pIStNodeMapDisplayWnd->SetVisibleStatusBar(false);
	m_pIStNodeMapDisplayWnd->SetVisibleVisibility(false);

	CWnd *pWndSettingButton(GetDlgItem(IDC_STATIC_SETTING));
	CRect rectSetting;
	pWndSettingButton->GetWindowRect(&rectSetting);

	ScreenToClient(&rectSetting);
	m_pIStNodeMapDisplayWnd->SetPosition(rectSetting.left, rectSetting.top, rectSetting.Width(), rectSetting.Height());
	m_pIStNodeMapDisplayWnd->Show(GetSafeHwnd(), StApi::StWindowMode_Child);

	CRect rectClient;
	GetWindowRect(&rectClient);
	ScreenToClient(&rectClient);
	// Status bar
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(pnIndicators, _countof(pnIndicators));
	const UINT nStatusWidth = rectClient.Width() / _countof(pnIndicators);
	for (int i = 0; i < _countof(pnIndicators); ++i)
	{
		m_wndStatusBar.SetPaneInfo(i, pnIndicators[i], (i == 0) ? SBPS_STRETCH : SBPS_NORMAL,  nStatusWidth);
	}
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	UpdateStatusBar();

	return TRUE;  // return TRUE unless you set the focus to a control
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (m_objBackgroundBrush.m_hObject == NULL)
	{
		m_objBackgroundBrush.CreateHatchBrush(HS_DIAGCROSS, RGB(192, 192, 192));
	}
	if (nIDCtl == IDC_BUTTON_IMAGE)
	{
		if (m_objCBitmap.m_hObject)
		{
			const CSize sizeImage = m_objCBitmap.GetBitmapDimension();
			const CSize sizeTarget(lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top);
			const double dblAspectRatioOfImage = sizeImage.cx / (double)sizeImage.cy;
			const double dblAspectRatioOfTarget = sizeTarget.cx / (double)sizeTarget.cy;


			CDC objItemDC;
			objItemDC.Attach(lpDrawItemStruct->hDC);
			objItemDC.SetStretchBltMode(COLORONCOLOR);
			CDC dcBitmap;
			dcBitmap.CreateCompatibleDC(&objItemDC);
			HGDIOBJ hOldBitmap = dcBitmap.SelectObject(m_objCBitmap.GetSafeHandle());

			if (dblAspectRatioOfImage < dblAspectRatioOfTarget)
			{
				const UINT nTargetX = (UINT)(sizeTarget.cy * dblAspectRatioOfImage + 0.5);
				objItemDC.StretchBlt(0, 0, nTargetX, sizeTarget.cy, &dcBitmap, 0, 0, sizeImage.cx, sizeImage.cy, SRCCOPY);
				CRect rectBack(nTargetX, 0, sizeTarget.cx, sizeTarget.cy);
				FillRect(lpDrawItemStruct->hDC, &rectBack, (HBRUSH)m_objBackgroundBrush.m_hObject);
			}
			else
			{
				const UINT nTargetY = (UINT)(sizeTarget.cx / dblAspectRatioOfImage + 0.5);
				objItemDC.StretchBlt(0, 0, sizeTarget.cx, nTargetY, &dcBitmap, 0, 0, sizeImage.cx, sizeImage.cy, SRCCOPY);
				CRect rectBack(0, nTargetY, sizeTarget.cx, sizeTarget.cy);
				FillRect(lpDrawItemStruct->hDC, &rectBack, (HBRUSH)m_objBackgroundBrush.m_hObject);
			}
			dcBitmap.SelectObject(hOldBitmap);
			objItemDC.Detach();
		}
		else
		{
			FillRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, (HBRUSH)m_objBackgroundBrush.m_hObject);
		}
	}
	else
	{
		CDialogEx::OnDrawItem(nIDCtl, lpDrawItemStruct);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_isNewImage)
	{
		if (m_objCBitmap.m_hObject)
		{
			m_objCBitmap.DeleteObject();
		}
		if (GetBitmapImage(GetDC(), m_objCBitmap))
		{
			m_isNewImage = false;
			GetDlgItem(IDC_BUTTON_IMAGE)->Invalidate();
		}
	}
	UpdateStatusBar();
	CDialogEx::OnTimer(nIDEvent);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BOOL CEachDeviceDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetDlgItem(IDC_STATIC_SETTING)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	pDC->ExcludeClipRect(&rect);

	return CDialogEx::OnEraseBkgnd(pDC);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CEachDeviceDlg::UpdateStatusBar()
{
	try
	{
		const double dblFPS = m_pIStDataStream->GetCurrentFPS();
		const uint64_t nFrameCount = m_pIStDataStream->GetIStDataStreamInfo()->GetNumDelivered();
		
		for (int i = 0; i < _countof(pnIndicators); ++i)
		{
			CString strText;
			switch (pnIndicators[i])
			{
			case(IDS_STATUS_FPS):
				strText.Format(TEXT("\t%.2f FPS"), dblFPS);
				break;
			case(IDS_STATUS_FRAME_COUNT):
				strText.Format(TEXT("\tRcv:%llu"), nFrameCount);
				break;
			case(IDS_STATUS_DROP_COUNT):
				strText.Format(TEXT("\tDrop:%llu"), m_nDroppedFrameCount);
				break;
			}
			m_wndStatusBar.SetPaneText(i, strText);
		}
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}
}
