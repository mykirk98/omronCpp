#include "CameraWorker.h"

CameraWorker::CameraWorker(uint64_t imageCount)
	: m_imageCount(imageCount)
	, m_initialized(false)
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
			//TODO: LinearCapture 메소드로 리팩토링하기
			// 버퍼 포인터를 5000ms의 타임아웃으로 검색
			CIStStreamBufferPtr pStreamBuffer = m_pDataStream->RetrieveBuffer(5000);

			// 획득한 데이터에 이미지 데이터가 있는지 확인
			if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				// 이미지 데이터가 있는 경우 IStImage 객체 생성
				IStImage* pImage = pStreamBuffer->GetIStImage();
				
				//OPTIMIZE: 간단하게 프레임 ID를 문자열로 변환할 수 있지 않을까? --> 아래 코드의 string 데이터 타입으로 변환하기
				//std::string strFrameID = std::to_string(pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID());
                GenICam::gcstring frameID = GenICam::gcstring(std::to_string(pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()).c_str());
				
				// 이미지 정보 출력
				std::cout << "BlockId=" << frameID
					<< " Size: " << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
					<< " First byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer()))
					<< std::endl;
				
				// 이미지를 저장하기 위한 이미지 버퍼 생성
				CIStImageBufferPtr pImageBuffer(CreateIStImageBuffer());
				ConvertToBGR8(pImage, StPFNC_BGR8, pImageBuffer);
				
				//GenICam::gcstring savePath = SetSavePath(frameID);
				//SaveImage<BMP>(pImageBuffer, savePath);
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

GenICam::gcstring CameraWorker::SetSavePath(const GenICam::gcstring frameID)
{
	try
	{
		// 사용자 문서 폴더 경로 가져오기
		wchar_t szPath[MAX_PATH] = { 0 };
		SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, 0, szPath);
		
		// 이미지 저장 경로 설정
		GenICam::gcstring strFileNameHeader(szPath);
		strFileNameHeader.append("\\");
		strFileNameHeader.append("OmronCameraExperiments\\");
		strFileNameHeader.append(m_pDevice->GetIStDeviceInfo()->GetDisplayName());
        strFileNameHeader.append(frameID);
		
		return strFileNameHeader;
		//NOTE: StApiRaw: 카메라에서 획득한 원본 이미지 데이터와 관련 메타데이터를 그대로 저장
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Set save path error: " << e.GetDescription() << std::endl;
		return GenICam::gcstring();
	}
}

void CameraWorker::LoadSavedImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& filePath)
{
	try
	{
		// 이미지 파일 입출력을 위한 filer 객체 생성
		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));

		std::wcout << std::endl << L"Loading " << filePath.w_str().c_str() << L"... ";
		//NOTE: w_str(): wide string(wchar_t*) 포인터로 반환
		//NOTE: c_str(): char* 포인터로 반환
		//NOTE: L: wide string 리터럴을 의미, 각 문자가 2바이트로 표현됨
		pStillImageFiler->Load(pImageBuffer, filePath);

		std::cout << "done." << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Load image error: " << e.GetDescription() << std::endl;
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

template<typename FORMAT>
void CameraWorker::SaveImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& savePath)
{
	try
	{
		// 이미지 저장 경로에 확장자 추가 by 템플릿
		GenICam::gcstring strSaveDir = savePath;
		strSaveDir.append(FORMAT::extension);
		
		// 이미지 저장을 위한 filer 객체 생성
		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));

		// 이미지 저장
		std::wcout << std::endl << L"Saving " << strSaveDir.w_str().c_str() << L"... ";
		pStillImageFiler->Save(pImageBuffer->GetIStImage(), FORMAT::fileFormat, strSaveDir);
		std::cout << "done" << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Save image error: " << e.GetDescription() << std::endl;
	}
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