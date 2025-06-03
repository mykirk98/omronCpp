// DefectivePixelDetectionPane.cpp
//

#include "stdafx.h"
#include "StViewer.h"
#include "DefectivePixelDetectionPane.h"

using namespace StApi;
using namespace GenApi;


// CDefectivePixelDetectionPane

IMPLEMENT_DYNAMIC(CDefectivePixelDetectionPane, CDockablePane)
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CDefectivePixelDetectionPane::CDefectivePixelDetectionPane(GenApi::INodeMap *pINodeMap, IStreamingCtrl *pIStreamingCtrl,  StApi::IStImageDisplayWnd *pIStImageDisplayWnd) :
	m_pIStImageAveragingFilter(CreateIStFilter(StFilterType_ImageAveraging)),
	m_objCDefectivePixelManager(pINodeMap, pIStImageDisplayWnd),
	m_pIBoolean_PixelCorrectionAllEnabled(pINodeMap->GetNode("PixelCorrectionAllEnabled")),
	m_pIFloat_AcquisitionFrameRate(pINodeMap->GetNode("AcquisitionFrameRate")),
	m_pIFloat_ExposureTime(pINodeMap->GetNode("ExposureTime")),
	m_objListCtrl(&m_objCDefectivePixelManager),
	m_pIStreamingCtrl(pIStreamingCtrl),
	m_pIStImageBuffer(CreateIStImageBuffer()),
	m_nRcvedFrameCount(SIZE_MAX),
	m_nFrameCount(10), m_bSaveAveragedImage(false), m_bFirstTime(true)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CDefectivePixelDetectionPane::~CDefectivePixelDetectionPane()
{
}


BEGIN_MESSAGE_MAP(CDefectivePixelDetectionPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_BN_CLICKED(ID_BUTTON_DETECT_DEFECTIVE_PIXEL, &CDefectivePixelDetectionPane::OnBnClickedDetect)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_DETECT_DEFECTIVE_PIXEL, &CDefectivePixelDetectionPane::OnUpdateDetect)
	ON_BN_CLICKED(ID_BUTTON_CLEAR_DETECTED_PIXEL_INFO, &CDefectivePixelDetectionPane::OnBnClickedClearDetected)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_CLEAR_DETECTED_PIXEL_INFO, &CDefectivePixelDetectionPane::OnUpdateClearDetected)
	ON_BN_CLICKED(ID_BUTTON_REGSTER_DEFECTIVE_PIXEL, &CDefectivePixelDetectionPane::OnBnClickedRegister)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_REGSTER_DEFECTIVE_PIXEL, &CDefectivePixelDetectionPane::OnUpdateRegister)
	ON_BN_CLICKED(ID_BUTTON_DEREGSTER_DEFECTIVE_PIXEL, &CDefectivePixelDetectionPane::OnBnClickedDeregister)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_DEREGSTER_DEFECTIVE_PIXEL, &CDefectivePixelDetectionPane::OnUpdateDeregister)
	ON_BN_CLICKED(ID_BUTTON_GET_REGISTERED_PIXE_INFO, &CDefectivePixelDetectionPane::OnBnClickedGetRegisteredPixelInfo)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_GET_REGISTERED_PIXE_INFO, &CDefectivePixelDetectionPane::OnUpdateGetRegisteredPixelInfo)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SAVE_AVERAGED_IMAGE, &CDefectivePixelDetectionPane::OnUpdateSaveAveragedImage)
	ON_BN_CLICKED(ID_BUTTON_SAVE_AVERAGED_IMAGE, &CDefectivePixelDetectionPane::OnBnClickedSaveAveragedImage)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_HIGHLIGHT_DEFECTIVE_PIXELS, &CDefectivePixelDetectionPane::OnUpdateHighlightDefectivePixels)
	ON_BN_CLICKED(ID_BUTTON_HIGHLIGHT_DEFECTIVE_PIXELS, &CDefectivePixelDetectionPane::OnBnClickedHighlightDefectivePixels)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PIXEL_CORRECTION_ENABLED, &CDefectivePixelDetectionPane::OnUpdatePixelCorrectionEnabled)
	ON_BN_CLICKED(ID_BUTTON_PIXEL_CORRECTION_ENABLED, &CDefectivePixelDetectionPane::OnBnClickedPixelCorrectionEnabled)
END_MESSAGE_MAP()


// CCameraSidePixelCorrectionDlg
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnIStImage(StApi::IStImage *pIStImage)
{
	try
	{
		if (m_nRcvedFrameCount < m_nFrameCount)
		{

			m_pIStImageAveragingFilter->Filter(pIStImage);

			if (++m_nRcvedFrameCount == m_nFrameCount)
			{
				m_objRcvImageDoneEvent.SetEvent();
			}
		}
	}
	catch (...)
	{
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnUpdateDetect(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnBnClickedDetect()
{
	HCURSOR hCursor = NULL;
	try
	{
		CString strMsg;
		strMsg = TEXT("Click the [OK] button in a shaded or uniform subject. If you click the [OK] button while shooting a non-uniform subject, normal pixels will be detected as defective pixels, and detection will take longer.");
		if (IDOK != AfxMessageBox(strMsg, MB_ICONINFORMATION | MB_OKCANCEL))
		{
			return;
		}
		hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

		m_pIStImageAveragingFilter->ClearImageData();
		m_objRcvImageDoneEvent.ResetEvent();
		m_nRcvedFrameCount = 0;


		const bool isRunning = m_pIStreamingCtrl->IsAcquisitionRunning();
		if (!isRunning)
		{
			m_pIStreamingCtrl->StartImageAcquisition();
		}

		m_wndStatusBar.SetPaneText(0, TEXT("During image acquisition."));
		const double dblExposureTimeMs = IsReadable(m_pIFloat_ExposureTime) ? m_pIFloat_ExposureTime->GetValue() / 1000 : 0;
		const double dblAcquisitionFrameTimeMs = IsReadable(m_pIFloat_AcquisitionFrameRate) ? 1000.0 / m_pIFloat_AcquisitionFrameRate->GetValue() : 0;
		const double dblMaxFrameTime = max(dblExposureTimeMs, dblAcquisitionFrameTimeMs);
		const DWORD dwTotalTimeout = (DWORD)(dblMaxFrameTime * (m_nFrameCount + 1) * 1.5);

		if (WaitForSingleObject(m_objRcvImageDoneEvent, dwTotalTimeout) != WAIT_OBJECT_0)
		{
			SetCursor(hCursor);
			throw RUNTIME_EXCEPTION("Timeout");
		}

		if (!isRunning)
		{
			m_pIStreamingCtrl->StopImageAcquisition();
		}

		m_pIStImageAveragingFilter->GetAveragedImage(m_pIStImageBuffer);


		m_wndStatusBar.SetPaneText(0, TEXT("Detecting defective pixels."));
		m_objCDefectivePixelManager.DetectDefectivePixel(m_pIStImageBuffer->GetIStImage());
		m_objListCtrl.UpdateDefectivePixelList();

		SetCursor(hCursor);

		if (m_bSaveAveragedImage)
		{
			CFileDialog dlg(FALSE, TEXT("bmp"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, TEXT("Bitmap(*.bmp)|*.bmp"));
			dlg.m_ofn.lpstrTitle = TEXT("Save Still Image File");
			if (dlg.DoModal() == IDOK)
			{
				GenICam::gcstring strFileName(dlg.m_ofn.lpstrFile);
				SaveImage(m_pIStImageBuffer->GetIStImage(), strFileName);
			}
		}


		m_wndStatusBar.SetPaneText(0, TEXT("Defective pixel detection completed."));
	}
	catch (const GenICam::GenericException &e)
	{
		SetCursor(hCursor);
		OnException(e);
		const GenICam::gcstring strMsg(e.GetDescription());
		m_wndStatusBar.SetPaneText(0, GCSTRING_2_LPCTSTR(strMsg));
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnUpdateClearDetected(CCmdUI *pCmdUI)
{
	const size_t nDetectedPixelCount = m_objCDefectivePixelManager.GetDetectedDefectivePixelCount();
	pCmdUI->Enable(0 < nDetectedPixelCount);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnBnClickedClearDetected()
{
	try
	{
		m_objCDefectivePixelManager.ClearDetectedPixelList();
		m_objListCtrl.UpdateDefectivePixelList();
	}
	catch (const GenICam::GenericException &e)
	{
		OnException(e);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnUpdateRegister(CCmdUI *pCmdUI)
{
	const size_t nNotRegistedPixelCount = m_objListCtrl.GetSelectedNotRegisteredItemCount();
	pCmdUI->Enable(0 < nNotRegistedPixelCount);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnBnClickedRegister()
{

	const bool isRunning = m_pIStreamingCtrl->IsAcquisitionRunning();
	if (isRunning)
	{
		m_pIStreamingCtrl->StopImageAcquisition();
	}
	m_objListCtrl.RegisterSelectedPixel();

	if (isRunning)
	{
		m_pIStreamingCtrl->StartImageAcquisition();
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnUpdateDeregister(CCmdUI *pCmdUI)
{
	const size_t nRegistedPixelCount = m_objListCtrl.GetSelectedRegisteredItemCount();
	pCmdUI->Enable(0 < nRegistedPixelCount);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnBnClickedDeregister()
{
	const bool isRunning = m_pIStreamingCtrl->IsAcquisitionRunning();
	if (isRunning)
	{
		m_pIStreamingCtrl->StopImageAcquisition();
	}
	m_objListCtrl.DeregisterSelectedPixel();

	if (isRunning)
	{
		m_pIStreamingCtrl->StartImageAcquisition();
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnUpdatePixelCorrectionEnabled(CCmdUI *pCmdUI)
{
	try
	{
		pCmdUI->Enable(IsWritable(m_pIBoolean_PixelCorrectionAllEnabled));
		if (IsReadable(m_pIBoolean_PixelCorrectionAllEnabled))
		{
			pCmdUI->SetRadio(m_pIBoolean_PixelCorrectionAllEnabled->GetValue());
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
void CDefectivePixelDetectionPane::OnBnClickedPixelCorrectionEnabled()
{
	try
	{
		if (IsWritable(m_pIBoolean_PixelCorrectionAllEnabled))
		{
			m_pIBoolean_PixelCorrectionAllEnabled->SetValue(!m_pIBoolean_PixelCorrectionAllEnabled->GetValue());
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
void CDefectivePixelDetectionPane::SaveImage(IStImage *pIStImage, GenICam::gcstring &strFileName)
{
	CIStImageBufferPtr pIStImageBuffer;
	if (pIStImage->GetImagePixelFormat() != StPFNC_Mono8)
	{
		pIStImageBuffer = CreateIStImageBuffer();

		CIStPixelFormatConverterPtr pIStPixelFormatConverter(CreateIStConverter(StConverterType_PixelFormat));
		pIStPixelFormatConverter->SetDestinationPixelFormat(StPFNC_Mono8);
		pIStPixelFormatConverter->SetBayerInterpolationMethod(StBayerInterpolationMethod_Mono);
		pIStPixelFormatConverter->Convert(pIStImage, pIStImageBuffer);
		pIStImage = pIStImageBuffer->GetIStImage();
	}

	CIStStillImageFilerPtr pIStStillImageFiler(CreateIStFiler(StFilerType_StillImage));
	pIStStillImageFiler->Save(pIStImage, StStillImageFileFormat_Bitmap, strFileName);
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnUpdateGetRegisteredPixelInfo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnBnClickedGetRegisteredPixelInfo()
{
	const bool isRunning = m_pIStreamingCtrl->IsAcquisitionRunning();
	if (isRunning)
	{
		m_pIStreamingCtrl->StopImageAcquisition();
	}
	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	m_objCDefectivePixelManager.GetRegisteredDefectivePixelList();
	m_objListCtrl.UpdateDefectivePixelList();
	SetCursor(hCursor);
	if (isRunning)
	{
		m_pIStreamingCtrl->StartImageAcquisition();
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnUpdateSaveAveragedImage(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
	pCmdUI->SetRadio(m_bSaveAveragedImage);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnBnClickedSaveAveragedImage()
{
	m_bSaveAveragedImage = !m_bSaveAveragedImage;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnUpdateHighlightDefectivePixels(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
	pCmdUI->SetRadio(m_objCDefectivePixelManager.GetHighlight());

}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnBnClickedHighlightDefectivePixels()
{
	m_objCDefectivePixelManager.SetHighlight(!m_objCDefectivePixelManager.GetHighlight());
}

#define ID_FIRST 100
#define ID_LIST_CTRL (ID_FIRST)

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int CDefectivePixelDetectionPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	{
		const DWORD dwStyle = LVS_REPORT | LVS_SHOWSELALWAYS | WS_BORDER | WS_TABSTOP | WS_CLIPSIBLINGS | WS_CHILD;
		m_objListCtrl.Create(dwStyle, CRect(0, 0, 0, 0), this, ID_LIST_CTRL);
		m_objListCtrl.SetExtendedStyle(m_objListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
		m_objListCtrl.ShowWindow(SW_SHOW);
		m_objListCtrl.OnInitialUpdate();
	}
	{
		const UINT pnIndicators[] = { ID_SEPARATOR };
		m_wndStatusBar.Create(this);
		m_wndStatusBar.SetIndicators(pnIndicators, _countof(pnIndicators));
	}
	{
		m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_DEFECTIVE_PIXEL_DETECTION);
		m_wndToolBar.LoadToolBar(IDR_DEFECTIVE_PIXEL_DETECTION, 0, 0, TRUE);
		m_wndToolBar.CleanUpLockedImages();
		m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDR_DEFECTIVE_PIXEL_DETECTION_256 : IDR_DEFECTIVE_PIXEL_DETECTION, 0, 0, TRUE);

		m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
		m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
		m_wndToolBar.SetOwner(this);

		// All commands are passed through this control, not through the parent frame:
		m_wndToolBar.SetRouteCommandsViaFrame(FALSE);
	}

	AdjustLayout();
	return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::AdjustLayout()
{
	if (GetSafeHwnd() == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	const LONG cyToolBar = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	const LONG cyStatusBar = m_wndStatusBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyToolBar, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndStatusBar.SetWindowPos(NULL, rectClient.left, rectClient.bottom - cyStatusBar, rectClient.Width(), cyStatusBar, SWP_NOACTIVATE | SWP_NOZORDER);
	m_objListCtrl.SetWindowPos(NULL, rectClient.left, rectClient.top + cyToolBar, rectClient.Width(), rectClient.Height() - cyToolBar - cyStatusBar, SWP_NOACTIVATE | SWP_NOZORDER);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);
	AdjustLayout();

}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelDetectionPane::ShowPane(BOOL bShow, BOOL bDelay, BOOL bActivate/* = TRUE*/)
{
	if (bShow)
	{
		if (m_bFirstTime)
		{
			OnBnClickedGetRegisteredPixelInfo();
			m_bFirstTime = false;
		}

	}
	CDockablePane::ShowPane(bShow, bDelay, bActivate);
}