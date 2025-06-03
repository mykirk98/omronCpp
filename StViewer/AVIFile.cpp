#include "StdAfx.h"
#include "StViewer.h"
#include "AVIFile.h"

#include "AVIFileDlg.h"
#include "CStillImageFilesDlg.h"
#include <map>

using namespace GenApi;
using namespace StApi;


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CSaveMultipleImagesFileBase::CSaveMultipleImagesFileBase(void)
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CSaveMultipleImagesFileBase::~CSaveMultipleImagesFileBase(void)
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CSaveMultipleImagesFileBase::Open(StApi::IStDevice *pIStDevice, StApi::IStImageDisplayWnd *pIStImageDisplayWnd)
{
	bool isSuccess = false;

	m_pIStPixelFormatConverter.Reset(StApi::CreateIStConverter(StApi::StConverterType_PixelFormat));

	const IStPixelFormatConverter *pIStPixelFormatConverterForPreview = pIStImageDisplayWnd->GetIStPixelFormatConverter();
	m_pIStPixelFormatConverter->CopySettings(pIStPixelFormatConverterForPreview);

	GenApi::CNodeMapPtr pINodeMap_DisplayImage(pIStImageDisplayWnd->GetINodeMap());
	GenApi::CEnumerationPtr pIEnumeration_BayerInterpolationMethodForStillImageFile(pINodeMap_DisplayImage->GetNode("BayerInterpolationMethodForStillImageFile"));
	if (pIEnumeration_BayerInterpolationMethodForStillImageFile->GetCurrentEntry()->GetSymbolic().compare("Auto") == 0)
	{
		m_pIStPixelFormatConverter->SetBayerInterpolationMethod(StApi::StBayerInterpolationMethod_BiLinear2);
	}

	isSuccess = mOpen(pIStDevice, pIStImageDisplayWnd);

	return(isSuccess);
}




//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CAVIFile::CAVIFile(void) : m_isFirstFrame(true), m_iTimestampOffset(0), m_dblCameraFrameRate(60.0), m_isNeedToConvBeforeReg(false)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CAVIFile::~CAVIFile(void)
{

}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CAVIFile::mOpen(StApi::IStDevice *pIStDevice, StApi::IStImageDisplayWnd *pIStImageDisplayWnd)
{
	// Create a VideoFiler object to get the IStFilerReleasable interface pointer.
	// After the VideoFiler object is no longer needed, call the IStFilerReleasable::Release(), please discard the VideoFiler object.
	// In the destructor of CIStVideoFilerPtr, IStFilerReleasable::Release() is called.
	m_pIStVideoFiler.Reset(StApi::CreateIStFiler(StApi::StFilerType_Video));

	{
		GenApi::CNodeMapPtr pINodeMap_PFConv_Preview(pIStImageDisplayWnd->GetINodeMap());
		GenApi::CNodeMapPtr pINodeMap_PFConv_Save(m_pIStVideoFiler->GetINodeMap());
		{
			pINodeMap_PFConv_Save->GetNode("ColorMap")->ImposeVisibility(Invisible);
		}
		{
			GenApi::CEnumerationPtr pIEnumeration_Preview(pINodeMap_PFConv_Preview->GetNode("ColorMapType"));
			GenApi::CEnumerationPtr pIEnumeration_Save(pINodeMap_PFConv_Save->GetNode("ColorMapType"));
			pIEnumeration_Save->SetIntValue(pIEnumeration_Preview->GetIntValue());
			pIEnumeration_Save->GetNode()->ImposeVisibility(Invisible);
		}
		{
			GenApi::CBooleanPtr pIBoolean_Preview(pINodeMap_PFConv_Preview->GetNode("ColorMapInversion"));
			GenApi::CBooleanPtr pIBoolean_Save(pINodeMap_PFConv_Save->GetNode("ColorMapInversion"));
			pIBoolean_Save->SetValue(pIBoolean_Preview->GetValue());
			pIBoolean_Save->GetNode()->ImposeVisibility(Invisible);
		}
		{
			GenApi::CIntegerPtr pIInteger_Preview(pINodeMap_PFConv_Preview->GetNode("ColorMapPhase"));
			GenApi::CIntegerPtr pIInteger_Save(pINodeMap_PFConv_Save->GetNode("ColorMapPhase"));
			pIInteger_Save->SetValue(pIInteger_Preview->GetValue());
			pIInteger_Save->GetNode()->ImposeVisibility(Invisible);
		}
	}

	// Get the INodeMap interface pointer for the camera settings.
	GenApi::CNodeMapPtr pINodeMapRemote(pIStDevice->GetRemoteIStPort()->GetINodeMap());

	//Get "AcquisitionFrameRate" to estimate frame number from timestamp value.
	GenApi::CFloatPtr pIFloat_AcquisitionFrameRate(pINodeMapRemote->GetNode("AcquisitionFrameRate"));
	if (pIFloat_AcquisitionFrameRate)
	{
		m_dblCameraFrameRate = pIFloat_AcquisitionFrameRate->GetValue();
	}
	m_pIStVideoFiler->SetFPS(m_dblCameraFrameRate);

	//Show a configuration dialog.
	CAVIFileDlg dlg(m_pIStVideoFiler);
	return(IDOK == dlg.DoModal());
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CAVIFile::RegisterIStStreamBuffer(StApi::IStStreamBuffer *pIStStreamBuffer)
{
	if(m_pIStVideoFiler)
	{
		StApi::IStImage *pIStImage = pIStStreamBuffer->GetIStImage();

		//Get timestamp value
		uint64_t nCurrentTimestampNs = 0;
		try
		{
			nCurrentTimestampNs = pIStStreamBuffer->GetIStStreamBufferInfo()->GetTimestampNS();
		}
		catch (...)
		{
			const uint64_t iCurrentTimestamp = pIStStreamBuffer->GetIStStreamBufferInfo()->GetTimestamp();

			//TODO:Timestamp値を変換する
			nCurrentTimestampNs = iCurrentTimestamp;
		}

		// Calculating the frame number in consideration of the frame drop.
		uint32_t nEstimatedFrameNo = 0;
		if(m_isFirstFrame)
		{
			//Initialize timestamp offset value
			m_iTimestampOffset = nCurrentTimestampNs;
			m_isFirstFrame = false;

			switch (pIStImage->GetImagePixelFormat())
			{
			case(StPFNC_Pol1Mono8):
			case(StPFNC_Pol1MonoX8):
			case(StPFNC_Pol1MonoY8):
			case(StPFNC_Pol1MonoXY8):
			case(StPFNC_Pol1Mono10):
			case(StPFNC_Pol1MonoX10):
			case(StPFNC_Pol1MonoY10):
			case(StPFNC_Pol1MonoXY10):
			case(StPFNC_Pol1Mono12):
			case(StPFNC_Pol1MonoX12):
			case(StPFNC_Pol1MonoY12):
			case(StPFNC_Pol1MonoXY12):
			case(StPFNC_Pol1BayerRG8):
			case(StPFNC_Pol1BayerRGX8):
			case(StPFNC_Pol1BayerRGY8):
			case(StPFNC_Pol1BayerRGXY8):
			case(StPFNC_Pol1BayerRG10):
			case(StPFNC_Pol1BayerRGX10):
			case(StPFNC_Pol1BayerRGY10):
			case(StPFNC_Pol1BayerRGXY10):
			case(StPFNC_Pol1BayerRG12):
			case(StPFNC_Pol1BayerRGX12):
			case(StPFNC_Pol1BayerRGY12):
			case(StPFNC_Pol1BayerRGXY12):
			case(StPFNC_Pol1Mono10p):
			case(StPFNC_Pol1MonoX10p):
			case(StPFNC_Pol1MonoY10p):
			case(StPFNC_Pol1MonoXY10p):
			case(StPFNC_Pol1Mono12p):
			case(StPFNC_Pol1MonoX12p):
			case(StPFNC_Pol1MonoY12p):
			case(StPFNC_Pol1MonoXY12p):
			case(StPFNC_Pol1BayerRG10p):
			case(StPFNC_Pol1BayerRGX10p):
			case(StPFNC_Pol1BayerRGY10p):
			case(StPFNC_Pol1BayerRGXY10p):
			case(StPFNC_Pol1BayerRG12p):
			case(StPFNC_Pol1BayerRGX12p):
			case(StPFNC_Pol1BayerRGY12p):
			case(StPFNC_Pol1BayerRGXY12p):
			case(StPFNC_Pol1MonoC8):
			case(StPFNC_Pol1MonoXC8):
			case(StPFNC_Pol1MonoYC8):
			case(StPFNC_Pol1MonoXYC8):
			case(StPFNC_Pol1MonoC10):
			case(StPFNC_Pol1MonoXC10):
			case(StPFNC_Pol1MonoYC10):
			case(StPFNC_Pol1MonoXYC10):
			case(StPFNC_Pol1MonoC12):
			case(StPFNC_Pol1MonoXC12):
			case(StPFNC_Pol1MonoYC12):
			case(StPFNC_Pol1MonoXYC12):
			case(StPFNC_Pol1BayerRGC8):
			case(StPFNC_Pol1BayerRGXC8):
			case(StPFNC_Pol1BayerRGYC8):
			case(StPFNC_Pol1BayerRGXYC8):
			case(StPFNC_Pol1BayerRGC10):
			case(StPFNC_Pol1BayerRGXC10):
			case(StPFNC_Pol1BayerRGYC10):
			case(StPFNC_Pol1BayerRGXYC10):
			case(StPFNC_Pol1BayerRGC12):
			case(StPFNC_Pol1BayerRGXC12):
			case(StPFNC_Pol1BayerRGYC12):
			case(StPFNC_Pol1BayerRGXYC12):
			case(StPFNC_Pol1MonoC10p):
			case(StPFNC_Pol1MonoXC10p):
			case(StPFNC_Pol1MonoYC10p):
			case(StPFNC_Pol1MonoXYC10p):
			case(StPFNC_Pol1MonoC12p):
			case(StPFNC_Pol1MonoXC12p):
			case(StPFNC_Pol1MonoYC12p):
			case(StPFNC_Pol1MonoXYC12p):
			case(StPFNC_Pol1BayerRGC10p):
			case(StPFNC_Pol1BayerRGXC10p):
			case(StPFNC_Pol1BayerRGYC10p):
			case(StPFNC_Pol1BayerRGXYC10p):
			case(StPFNC_Pol1BayerRGC12p):
			case(StPFNC_Pol1BayerRGXC12p):
			case(StPFNC_Pol1BayerRGYC12p):
			case(StPFNC_Pol1BayerRGXYC12p):
				m_isNeedToConvBeforeReg = true;
				break;
			}
		}
		else
		{
			//estimate frame number from timestamp value.
			uint64_t nDelta = nCurrentTimestampNs - m_iTimestampOffset;
			double dblTmp = nDelta * m_dblCameraFrameRate;
			dblTmp /= 1000000000;

			nEstimatedFrameNo = (uint32_t)(dblTmp + 0.5);
		}

		if (m_isNeedToConvBeforeReg)
		{
			if (!m_pIStImageBuffer.IsValid())
			{
				m_pIStImageBuffer.Reset(StApi::CreateIStImageBuffer(NULL));
			}
			m_pIStPixelFormatConverter->Convert(pIStImage, m_pIStImageBuffer);
			pIStImage = m_pIStImageBuffer->GetIStImage();
		}
		
		//Add new frame to the avi file.
		m_pIStVideoFiler->RegisterIStImage(pIStImage, nEstimatedFrameNo);
		
		//Returned true if AVI file already closed.
		return(m_pIStVideoFiler->IsStopped());
	}
	else
	{
		//Returned true if AVI file already closed.
		return(true);
	}

}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStillImageFiles::CStillImageFiles(void) :
	m_LockList(), m_eventIsEmptyBuffer(FALSE, TRUE), m_eventIsFullBuffer(FALSE, TRUE), m_eventIsStopRequest(FALSE, TRUE), m_hThread(NULL),
	m_eSIFF(StApi::StStillImageFileFormat_Bitmap), m_nSaveAImagePer(10), m_nToRegImageCount(0), m_nSavedImageCount(0), m_nMaximumImageFileCount(100)
{
	TCHAR szPath[MAX_PATH];
	SHGetSpecialFolderPath(NULL, szPath, CSIDL_MYPICTURES, FALSE);

	m_strFilePath = dynamic_cast<CWinAppEx*>(AfxGetApp())->GetSectionString(TEXT("StillImageFile"), TEXT("Path"), szPath);
	m_strFileNamePattern = dynamic_cast<CWinAppEx*>(AfxGetApp())->GetSectionString(TEXT("StillImageFile"), TEXT("FileNamePattern"), L"${yyyy}${MM}${dd}_${HH}${mm}${ss}${ms}_${No}");
	m_nSaveAImagePer = dynamic_cast<CWinAppEx*>(AfxGetApp())->GetSectionInt(TEXT("StillImageFile"), TEXT("SaveAImagePer"), m_nSaveAImagePer);
	m_nMaximumImageFileCount = dynamic_cast<CWinAppEx*>(AfxGetApp())->GetSectionInt(TEXT("StillImageFile"), TEXT("MaximumImageFileCount"), m_nMaximumImageFileCount);
	m_eSIFF = (StApi::EStStillImageFileFormat_t)dynamic_cast<CWinAppEx*>(AfxGetApp())->GetSectionInt(TEXT("StillImageFile"), TEXT("StillImageFileFormat"), m_eSIFF);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CStillImageFiles::~CStillImageFiles(void)
{
	Stop();
	DeleteAllBuffer();
	
	dynamic_cast<CWinAppEx*>(AfxGetApp())->WriteSectionString(TEXT("StillImageFile"), TEXT("Path"), m_strFilePath);
	dynamic_cast<CWinAppEx*>(AfxGetApp())->WriteSectionString(TEXT("StillImageFile"), TEXT("FileNamePattern"), m_strFileNamePattern);

	dynamic_cast<CWinAppEx*>(AfxGetApp())->WriteSectionInt(TEXT("StillImageFile"), TEXT("SaveAImagePer"), m_nSaveAImagePer);
	dynamic_cast<CWinAppEx*>(AfxGetApp())->WriteSectionInt(TEXT("StillImageFile"), TEXT("MaximumImageFileCount"), m_nMaximumImageFileCount);
	dynamic_cast<CWinAppEx*>(AfxGetApp())->WriteSectionInt(TEXT("StillImageFile"), TEXT("StillImageFileFormat"), m_eSIFF);
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CStillImageFiles::mOpen(StApi::IStDevice * /*pIStDevice*/, StApi::IStImageDisplayWnd * /*pIStImageDisplayWnd*/)
{
	m_IStStillImageFiler.Reset(StApi::CreateIStFiler(StApi::StFilerType_StillImage));

	//Show a configuration dialog.
	CStillImageFilesDlg dlg(m_IStStillImageFiler, m_pIStPixelFormatConverter);
	dlg.m_strStillImageFilesPath = m_strFilePath;
	dlg.m_strStillImageFilesPattern = m_strFileNamePattern;
	dlg.m_nSaveAImagePer = m_nSaveAImagePer;
	dlg.m_nMaximumImageFileCount = m_nMaximumImageFileCount;
	dlg.m_nFileType = m_eSIFF;
	const bool bReval = (IDOK == dlg.DoModal());
	if (bReval)
	{
		m_strFilePath = dlg.m_strStillImageFilesPath;
		m_strFileNamePattern = dlg.m_strStillImageFilesPattern;
		m_nSaveAImagePer = dlg.m_nSaveAImagePer;
		m_nMaximumImageFileCount = dlg.m_nMaximumImageFileCount;
		m_eSIFF = (StApi::EStStillImageFileFormat_t)dlg.m_nFileType;
		CreateBuffer(4);
		Start();
	}
	return(bReval);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStillImageFiles::Start()
{
	Stop();
	m_eventIsStopRequest.ResetEvent();
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, CStillImageFiles::sThreadStart, this, 0, NULL));
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStillImageFiles::Stop()
{
	if (m_hThread != NULL)
	{
		m_eventIsStopRequest.SetEvent();
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStillImageFiles::CreateBuffer(size_t nCount)
{
	GenApi::AutoLock objAutoLock(m_LockList);
	for (size_t i = 0; i < nCount; ++i)
	{
		m_vecEmptyBufferList.push_back(StApi::CreateIStImageBuffer());
	}
	m_eventIsEmptyBuffer.SetEvent();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CStillImageFiles::DeleteAllBuffer()
{

	GenApi::AutoLock objAutoLock(m_LockList);

	for (std::vector<StApi::IStImageBufferReleasable*>::iterator itr = m_vecEmptyBufferList.begin(); itr != m_vecEmptyBufferList.end(); ++itr)
	{
		(*itr)->Release();
	}
	m_vecEmptyBufferList.clear();
	m_eventIsEmptyBuffer.ResetEvent();

	for (std::vector<StApi::IStImageBufferReleasable*>::iterator itr = m_vecFullBufferList.begin(); itr != m_vecFullBufferList.end(); ++itr)
	{
		(*itr)->Release();
	}
	m_vecFullBufferList.clear();
	m_eventIsFullBuffer.ResetEvent();
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CStillImageFiles::RegisterIStStreamBuffer(StApi::IStStreamBuffer *pIStStreamBuffer)
{
	if ((m_nSaveAImagePer == 0) || (m_nToRegImageCount % m_nSaveAImagePer == 0))
	{
		StApi::IStImageBufferReleasable *pIStImageBufferReleasable = NULL;
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_eventIsEmptyBuffer.m_hObject, 0))
		{
			GenApi::AutoLock objAutoLock(m_LockList);
			pIStImageBufferReleasable = *m_vecEmptyBufferList.begin();
			m_vecEmptyBufferList.erase(m_vecEmptyBufferList.begin());
			if (m_vecEmptyBufferList.size() == 0) m_eventIsEmptyBuffer.ResetEvent();
		}

		if (pIStImageBufferReleasable != NULL)
		{
			try
			{
				m_pIStPixelFormatConverter->Convert(pIStStreamBuffer->GetIStImage(), pIStImageBufferReleasable);
				{
					GenApi::AutoLock objAutoLock(m_LockList);
					m_vecFullBufferList.push_back(pIStImageBufferReleasable);
					m_eventIsFullBuffer.SetEvent();
				}

			}
			catch (...)
			{
				GenApi::AutoLock objAutoLock(m_LockList);
				m_vecEmptyBufferList.push_back(pIStImageBufferReleasable);
				m_eventIsEmptyBuffer.SetEvent();
			}
		}
	}
	++m_nToRegImageCount;
	return((m_hThread == NULL) || (WAIT_OBJECT_0 == WaitForSingleObject(m_hThread, 0)));
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
unsigned int CStillImageFiles::mWork()
{
	const HANDLE phWaits[] = { m_eventIsStopRequest.m_hObject, m_eventIsFullBuffer.m_hObject};
	for (;;)
	{
		DWORD dwWait = WaitForMultipleObjects(_countof(phWaits), phWaits, FALSE, INFINITE);
		if (dwWait == WAIT_OBJECT_0)
		{
			break;
		}
		StApi::IStImageBufferReleasable *pIStImageBufferReleasable = NULL;
		{
			GenApi::AutoLock objAutoLock(m_LockList);

			pIStImageBufferReleasable = *m_vecFullBufferList.begin();
			m_vecFullBufferList.erase(m_vecFullBufferList.begin());
			if (m_vecFullBufferList.size() == 0) m_eventIsFullBuffer.ResetEvent();
		}

		try
		{
			++m_nSavedImageCount;

			SYSTEMTIME sSystemTime = { 0 };
			GetLocalTime(&sSystemTime);

			CString strFileName(m_strFileNamePattern);


			CString szValue;
			szValue.Format(TEXT("%04u"), sSystemTime.wYear);
			strFileName.Replace(L"${yyyy}", szValue);

			szValue.Format(TEXT("%02u"), sSystemTime.wMonth);
			strFileName.Replace(L"${MM}", szValue);

			szValue.Format(TEXT("%02u"), sSystemTime.wDay);
			strFileName.Replace(L"${dd}", szValue);

			szValue.Format(TEXT("%02u"), sSystemTime.wHour);
			strFileName.Replace(L"${HH}", szValue);

			szValue.Format(TEXT("%02u"), sSystemTime.wMinute);
			strFileName.Replace(L"${mm}", szValue);

			szValue.Format(TEXT("%02u"), sSystemTime.wSecond);
			strFileName.Replace(L"${ss}", szValue);

			szValue.Format(TEXT("%03u"), sSystemTime.wMilliseconds);
			strFileName.Replace(L"${ms}", szValue);
#if _MSC_VER < 1900
	#if _WIN64
			szValue.Format(TEXT("%04I64u"), m_nSavedImageCount);
	#else
			szValue.Format(TEXT("%04u"), m_nSavedImageCount);
	#endif
#else
			szValue.Format(TEXT("%04zu"), m_nSavedImageCount);
#endif
			strFileName.Replace(L"${No}", szValue);

			GenICam::gcstring strFullPath(m_strFilePath.GetBuffer());
			strFullPath.append(GenICam::gcstring(L"\\"));
			strFullPath.append(GenICam::gcstring(strFileName));

			switch(m_eSIFF)
			{
			case(StStillImageFileFormat_StApiRaw):	strFullPath.append(GenICam::gcstring(L".straw")); break;
			case(StStillImageFileFormat_Bitmap):	strFullPath.append(GenICam::gcstring( L".bmp")); break;
			case(StStillImageFileFormat_JPEG):	strFullPath.append(GenICam::gcstring(L".jpg")); break;
			case(StStillImageFileFormat_TIFF):	strFullPath.append(GenICam::gcstring(L".tif")); break;
			case(StStillImageFileFormat_PNG):	strFullPath.append(GenICam::gcstring(L".png")); break;
			case(StStillImageFileFormat_CSV):	strFullPath.append(GenICam::gcstring(L".csv")); break;
			}
			m_IStStillImageFiler->Save(pIStImageBufferReleasable->GetIStImage(), m_eSIFF, strFullPath);
		}
		catch (...)
		{

		}

		{
			GenApi::AutoLock objAutoLock(m_LockList);
			m_vecEmptyBufferList.push_back(pIStImageBufferReleasable);
			m_eventIsEmptyBuffer.SetEvent();
		}

		if ((m_nMaximumImageFileCount != 0) && (m_nMaximumImageFileCount <= m_nSavedImageCount))
		{
			break;
		}
	}

	return(0);
}