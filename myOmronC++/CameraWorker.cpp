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
			CIStStreamBufferPtr pStreamBuffer(m_pDataStream->RetrieveBuffer(5000));

			// 획득한 데이터에 이미지 데이터가 있는지 확인
			if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				// 이미지 데이터가 있는 경우 IStImage 객체 생성
				IStImage* pImage = pStreamBuffer->GetIStImage();
				
				//OPTIMIZE: 간단하게 프레임 ID를 문자열로 변환할 수 있지 않을까? --> 아래 코드의 string 데이터 타입으로 변환하기
				//std::string strFrameID = std::to_string(pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID());
                GenICam::gcstring frameID = GenICam::gcstring(std::to_string(pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()).c_str());
				
				PrintFrameInfo(pImage, pStreamBuffer);
				
				// 이미지를 저장하기 위한 이미지 버퍼 생성
				CIStImageBufferPtr pImageBuffer(CreateIStImageBuffer());
				ConvertPixelFormat(pImage, true, pImageBuffer);
				
				GenICam::gcstring savePath = SetSavePath(pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID());
				SaveImage<BMP>(pImageBuffer, savePath);
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

void CameraWorker::PrintFrameInfo(const IStImage* pImage, const CIStStreamBufferPtr& pStreamBuffer)
{
	//NOTE: Frame과 Image의 차이점
	// Frame: 버퍼에서 읽어온 데이터
	// Image: 프레임을 이미지 객체로 변환하거나 이미지로 저장할 때 불림
	std::cout << "Block ID: " << pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
		<< "\tSize: " << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
		<< "\tFirst byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer()))
		<< std::endl;
		// reinterpret_cast : 서로 관련 없는 포인터 타입 간의 변환을 수행하는 연산자
		// dynamic_cast가 아닌 static_cast를 사용한 이유 :
		// dynamic_cast는 상속 관계가 있는 클래스 포인터/참조를 안전하게 변환할 때 사용되며,
		// 여기에서는 단순히 기본 타입 간의 변환(uint8_t* -> uint32_t) 이므로 static_cast를 사용해도 안전
}

void CameraWorker::LoadSavedImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& srcDir)
{
	try
	{
		// 이미지 파일 입출력을 위한 filer 객체 생성
		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));

		std::wcout << std::endl << L"Loading " << srcDir.w_str().c_str() << L"... ";
		//NOTE: w_str(): wide string(wchar_t*) 포인터로 반환
		//NOTE: c_str(): char* 포인터로 반환
		//NOTE: L: wide string 리터럴을 의미, 각 문자가 2바이트로 표현됨
		pStillImageFiler->Load(pImageBuffer, srcDir);

		std::cout << "done." << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Load image error: " << e.GetDescription() << std::endl;
	}
}

GenICam::gcstring CameraWorker::SetSavePath(std::string savePath, const uint64_t frameID)
{
	try
	{
		//std::string savePath = "C:\\Users\\mykir\\Work\\Experiments\\";
		// frameID를 문자열로 변환
		std::string strFrameID = std::to_string(frameID);

		// 사용자 지정 경로와 frameID를 결합하여 저장 경로 생성
		std::string filePath = savePath + m_pDevice->GetIStDeviceInfo()->GetDisplayName().c_str() + strFrameID;

		return GenICam::gcstring(filePath.c_str());
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Set save path error: " << e.GetDescription() << std::endl;
	}
}

void CameraWorker::ConvertPixelFormat(IStImage* pSrcImage, bool setColor, CIStImageBufferPtr& pDstBuffer)
{
	// 픽셀 포맷 변환을 위한 converter 객체 생성
	CIStPixelFormatConverterPtr pPixelFormatConverter(CreateIStConverter(StConverterType_PixelFormat));

	// BGR8 포맷으로 변환
	if (setColor)
	{
		pPixelFormatConverter->SetDestinationPixelFormat(StPFNC_BGR8);
	}
	else
	{
		pPixelFormatConverter->SetDestinationPixelFormat(StPFNC_Mono8);
	}
	pPixelFormatConverter->Convert(pSrcImage, pDstBuffer);
}

template<typename FORMAT>
void CameraWorker::SaveImage(CIStImageBufferPtr& pImageBuffer, GenICam::gcstring& dstDir)
{
	try
	{
		// 이미지 저장 경로에 확장자 추가 by 템플릿
		//GenICam::gcstring strSaveDir = dstDir;
		dstDir.append(FORMAT::extension);
		
		// 이미지 저장을 위한 filer 객체 생성
		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));

		// 이미지 저장
		//std::wcout << std::endl << L"Saving " << strSaveDir.w_str().c_str() << L"... ";
		std::wcout << L"Saving " << dstDir.w_str().c_str() << L"... ";
		pStillImageFiler->Save(pImageBuffer->GetIStImage(), FORMAT::fileFormat, dstDir);
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