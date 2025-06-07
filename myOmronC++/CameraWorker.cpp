#include "CameraWorker.h"
#include <iostream>

CameraWorker::CameraWorker(uint64_t imageCount)
	: m_imageCount(imageCount)
	, m_initialized(false)
{
}

CameraWorker::~CameraWorker()
{
	if (m_initialized)
	{
		// 카메라 획득 중지
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