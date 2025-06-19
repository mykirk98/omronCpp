#include "CameraWorker.h"

CameraWorker::CameraWorker(uint64_t imageCount)
	: m_imageCount(imageCount)
	, m_initialized(false)
	, m_isImageSaved(false)
	, m_savePath(L"")
{
}

CameraWorker::~CameraWorker()
{
	if (m_initialized)
	{
		StopAcquisition();
	}
}

bool CameraWorker::initialize()
{
	try
	{
		// 시스템 객체 생성 (장치 검색 및 연결)
		m_pSystem = CreateIStSystem();
		
		// 첫 번째 장치 생성 및 연결
		m_pDevice = m_pSystem->CreateFirstIStDevice();
		
		// 장치 정보 출력
		std::cout << "Device=" << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;
		
		// 이미지 스트림 데이터를 처리하기 위한 데이터스트림 객체 생성
		m_pDataStream = m_pDevice->CreateIStDataStream(0);
		
		m_initialized = true;
		
		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Initialization error: " << e.GetDescription() << std::endl;
		return false;
	}
}

void CameraWorker::StartAcquisition()
{
	if (m_initialized == false)
	{
		std::cerr << "Camera not initialized. Call initialize() first." << std::endl;
		return;
	}
	try
	{
		// 호스트(PC) 측의 이미지 획득 시작
		m_pDataStream->StartAcquisition(m_imageCount);

		// 카메라 측의 이미지 획득 시작
		m_pDevice->AcquisitionStart();

		// 이미지 획득 및 상태 확인을 위한 루프
		while (m_pDataStream->IsGrabbing())
		{
			// 버퍼 포인터를 5000ms의 타임아웃으로 검색
			CIStStreamBufferPtr pStreamBuffer = m_pDataStream->RetrieveBuffer(5000);

			// 획득한 데이터에 이미지 데이터가 있는지 확인
			if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				// 이미지 데이터가 있는 경우 IStImage 객체 생성
				IStImage* pImage = pStreamBuffer->GetIStImage();
				
				// 이미지 정보 출력
				std::cout << "BlockId=" << pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size: " << pImage->GetImageWidth()
					<< " x " << pImage->GetImageHeight()
					<< " First byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer()))
					<< std::endl;

				CIStImageBufferPtr pImageBuffer(CreateIStImageBuffer());
				ConvertToBGR8(pImage, StPFNC_BGR8, pImageBuffer);

				SaveBMPImage(pImageBuffer);
			}
			else
			{
				std::cout << "No image data present in the buffer." << std::endl;
			}
		}
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Acquisition error: " << e.GetDescription() << std::endl;
	}
}

void CameraWorker::StopAcquisition()
{
	try
	{
		if (m_pDevice)
		{
			// 카메라 측 이미지 획득 중지
			m_pDevice->AcquisitionStop();
			// 호스트(PC) 측 이미지 획득 중지
			m_pDataStream->StopAcquisition();
		}
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Stop acqiusition error: " << e.GetDescription() << std::endl;
	}
}

GenICam::gcstring CameraWorker::SetSavePath()
{
	try
	{
		// 사용자 문서 폴더 경로 가져오기
		wchar_t szPath[MAX_PATH];
		SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, 0, szPath);

		// 이미지 저장 경로 설정
		GenICam::gcstring strFileNameHeader(szPath);
		strFileNameHeader.append("\\");
		strFileNameHeader.append(m_pDevice->GetIStDeviceInfo()->GetDisplayName());
		//TODO: 프레임ID를 포함한 파일 이름 생성, 현재는 streamBuffer가 StartAcquisition메소드의 지역 변수로 선언되어 있어 접근 불가

		return strFileNameHeader;
		//NOTE: StApiRaw: 카메라에서 획득한 원본 이미지 데이터와 관련 메타데이터를 그대로 저장
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Set save path error: " << e.GetDescription() << std::endl;
		return GenICam::gcstring();
	}
}

//bool CameraWorker::SaveImage(IStImage* pImage)
//{
//	try
//	{
//		GenICam::gcstring strFileName = SetSavePath();
//		strFileName.append(".StApiRaw");
//
//		// 정지(still) 이미지를 파일로 처리하는 기능을 담당하는 객체 생성 (StApi_IP.h 라이브러리에서 제공)
//		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));
//		
//		std::wcout << std::endl << L"Saving " << strFileName.w_str().c_str() << L"... ";
//
//		pStillImageFiler->Save(pImage, StStillImageFileFormat_StApiRaw, strFileName);
//
//		std::cout << "done." << std::endl;
//
//		return true;
//	}
//	catch (const GenICam::GenericException& e)
//	{
//		std::cerr << "Save image error: " << e.GetDescription() << std::endl;
//		return false;
//	}
//}
//
//bool CameraWorker::LoadImage()
//{
//	try
//	{
//		GenICam::gcstring strFileNameRaw = SetSavePath();
//		strFileNameRaw.append(".StApiRaw");
//		
//		// StApiRaw 파일에서 이미지를 읽어올 버퍼 객체 생성, 임시로 저장하고 처리하기 위한 이미지 버퍼 객체
//		//m_pImageBuffer = CreateIStImageBuffer();
//
//		// 이미지 파일 입출력을 위한 filer 객체 생성
//		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));
//
//		std::wcout << std::endl << L"Loading " << strFileNameRaw.w_str().c_str() << L"... ";
//		//NOTE: w_str(): wide string(wchar_t*) 포인터로 반환
//		//NOTE: c_str(): char* 포인터로 반환
//		//NOTE: L: wide string 리터럴을 의미, 각 문자가 2바이트로 표현됨
//
//		//pStillImageFiler->Load(pImageBuffer, strFileNameRaw);.3
//		std::cout << "done." << std::endl;
//
//		return true;
//	}
//	catch (const GenICam::GenericException& e)
//	{
//		std::cerr << "Load image error: " << e.GetDescription() << std::endl;
//		return false;
//	}
//}

//bool CameraWorker::SetImageFormat()
//{
//	try
//	{
//		// 픽셀 포맷 변환을 위한 converter 객체 생성
//		CIStPixelFormatConverterPtr pPixelFormatConverter(CreateIStConverter(StConverterType_PixelFormat));
//		
//		// BGR8 포맷으로 변환
//		pPixelFormatConverter->SetDestinationPixelFormat(StPFNC_BGR8);
//
//		return true;
//	}
//	catch (const GenICam::GenericException& e)
//	{
//		std::cerr << "Set image format error: " << e.GetDescription() << std::endl;
//		return false;
//	}
//}

//bool CameraWorker::SaveBMPImage()
//{
//	try
//	{
//		GenICam::gcstring strFileName = SetSavePath();
//		strFileName.append(".bmp");
//
//		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));
//
//		std::wcout << std::endl << L"Saving " << strFileName.w_str().c_str() << L"... ";
//		std::cout << "done." << std::endl;
//
//		return true;
//	}
//	catch (const GenICam::GenericException& e)
//	{
//		std::cerr << "Save BMP image error: " << e.GetDescription() << std::endl;
//		return false;
//	}
//}

void CameraWorker::SaveBMPImage(CIStImageBufferPtr& pImageBuffer)
{
	try
	{
		GenICam::gcstring strFileName = SetSavePath();
		strFileName.append(".bmp");

		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));

		std::wcout << std::endl << L"Saving " << strFileName.w_str().c_str() << L"... " << std::endl;
		pStillImageFiler->Save(pImageBuffer->GetIStImage(), StStillImageFileFormat_Bitmap, strFileName);
		std::cout << "done." << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Save BMP image error: " << e.GetDescription() << std::endl;
	}
}

void CameraWorker::ConvertToBGR8(IStImage* pSrcImage, EStPixelFormatNamingConvention_t dstFormat, CIStImageBufferPtr& pDstBuffer)
{
	// 픽셀 포맷 변환을 위한 converter 객체 생성
	CIStPixelFormatConverterPtr pPixelFormatConverter(CreateIStConverter(StConverterType_PixelFormat));
	
	// BGR8 포맷으로 변환
	pPixelFormatConverter->SetDestinationPixelFormat(dstFormat);
	pPixelFormatConverter->Convert(pSrcImage, pDstBuffer);
}



// 사용 예시 (main.cpp에서 호출)
/*
int main()
{
	CameraWorker cameraWorker(10); // 10개의 이미지 획득
	if (cameraWorker.initialize())
	{
		cameraWorker.StartAcquisition();

		// ... 이미지 처리 로직 ...
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}
*/