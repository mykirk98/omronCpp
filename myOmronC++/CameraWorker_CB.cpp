#include "CameraWorker_CB.h"

CameraWorker_CB::CameraWorker_CB()
	: m_initialized(false)
{

}

CameraWorker_CB::~CameraWorker_CB()
{
	stopAcquisition();
}

bool CameraWorker_CB::initialize()
{
	try
	{
		// 시스템 객체 생성 (장치 검색 및 연결)
		m_pSystem = CreateIStSystem();

		// 첫 번쨰 장치 생성 및 연결
		m_pDevice = m_pSystem->CreateFirstIStDevice();

		// 장치 정보 출력
		std::cout << "Device: " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;

		// 이미지 스트림 데이터를 처리하기 위한 데이터스트림 객체 생성
		m_pDataStream = m_pDevice->CreateIStDataStream(0);

		// 데이터 스트림 콜백 설정 (this 포인터를 pvContext로 전달)
		RegisterCallback(m_pDataStream, &CameraWorker_CB::OnStCallbackFunction, this);

		m_initialized = true;
		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Initialization error: " << e.GetDescription() << std::endl;
		return false;
	}
}

void CameraWorker_CB::startAcquisition()
{
	if (m_initialized == false)
	{
		std::cerr << "Camera not initialized. Call initialize() first." << std::endl;
		return;
	}
	else
	{
		try
		{
			// 호스트(PC) 측 이미지 획득 시작
			m_pDataStream->StartAcquisition();

			// 카메라 측 이미지 획득 시작
			m_pDevice->AcquisitionStart();
		}
		catch (const GenICam::GenericException& e)
		{
			std::cerr << "Start acquisition error: " << e.GetDescription() << std::endl;
		}
	}
}

void CameraWorker_CB::stopAcquisition()
{
	if (m_initialized == false)
	{
		return;
	}

	else
	{
		try
		{
			// 카메라 측 이미지 획득 중지
			m_pDevice->AcquisitionStop();

			// 호스트(PC) 측 이미지 획득 중지
			m_pDataStream->StopAcquisition();
		}
		catch (const GenICam::GenericException& e)
		{
			std::cerr << "Stop acquisition error: " << e.GetDescription() << std::endl;
		}
	}
}

void __stdcall CameraWorker_CB::OnStCallbackFunction(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext)
{
	if (pvContext)
	{
		// pvContext로 넘긴 this 포인터를 다시 캐스팅하여 멤버 함수 호출
		static_cast<CameraWorker_CB*>(pvContext)->handleCallback(pIStCallbackParamBase);
	}
}

// 멤버 콜백 처리 함수
void CameraWorker_CB::handleCallback(IStCallbackParamBase* pCallbackParam)
{
	// 콜백 파라미터의 타입 확인
	if (pCallbackParam->GetCallbackType() == StCallbackType_GenTLEvent_DataStreamNewBuffer)
	{
		IStCallbackParamGenTLEventNewBuffer* pNewBufferParam = dynamic_cast<IStCallbackParamGenTLEventNewBuffer*>(pCallbackParam);

		try
		{
			IStDataStream* pDataStream = pNewBufferParam->GetIStDataStream();
			
			CIStStreamBufferPtr pBuffer(pDataStream->RetrieveBuffer(0));
			
			if (pBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				IStImage* pImage = pBuffer->GetIStImage();
				std::cout << "Block ID: " << pBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< "Size: " << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
					<< "First byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer())) << std::endl;
			}
			else
			{
				std::cout << "No image present in the buffer." << std::endl;
			}
		}
		catch (const GenICam::GenericException& e)
		{
			std::cerr << "Callback Exception: " << e.GetDescription() << std::endl;
		}
	}
}

// 사용 예시 (main.cpp에서 호출)
/*
int main()
{
	CameraWorker_CB cameraWorker;
	if (cameraWorker.initialize())
	{
		cameraWorker.startAcquisition();
		
		// ... 이미지 처리 로직 ...
		
		std::cout << "Press Enter to stop acquisition..." << std::endl;
		std::cin.get(); // 사용자 입력 대기

		cameraWorker.stopAcquisition();
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}

*/