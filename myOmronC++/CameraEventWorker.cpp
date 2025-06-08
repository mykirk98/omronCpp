#include "CameraEventWorker.h"

using namespace GenApi;

namespace
{
	// unnamed namespace : 현재 파일 내에서만 접근 가능하도록 제한하는 기능
	const char* EVENT_SELECTOR = "EventSelector";
	const char* EVENT_NOTIFICATION = "EventNotification";
	const char* EVENT_NOTIFICATION_ON = "On";
	const char* TARGET_EVENT_NAME = "ExposureEnd";
	const char* CALLBACK_NODE_NAME = "EventExposureEndTimestamp";
}

CameraEventWorker::CameraEventWorker(uint64_t imageCount)
// initializer list : 멤버 변수 초기화
	: m_imageCount(imageCount)
	, m_initialized(false)
{

}

CameraEventWorker::~CameraEventWorker()
{
	stopAcquisition();
}

bool CameraEventWorker::initialize()
{
	try
	{
		// 시스템 객체 생성
		m_pSystem = CreateIStSystem();

		// 카메라 객체 생성
		m_pDevice = m_pSystem->CreateFirstIStDevice();

		// 장치 정보 출력
		std::cout << "Device: " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;

		// 데이터 스트림 생성
		m_pDataStream = m_pDevice->CreateIStDataStream(0);
		// NOTE:
		// 데이터스트림 : 카메라에서 이미지를 연속적으로 받아오는 통로를 의미(파이프라인과 유사함)

		// 카메라의 원격 포트에서 노드 맵 가져오기
		m_pNodeMap = m_pDevice->GetRemoteIStPort()->GetINodeMap();

		// 콜백 대상 노드 가져오기
		CNodePtr pNodeCallback(m_pNodeMap->GetNode(CALLBACK_NODE_NAME));
		if (pNodeCallback.IsValid() == false)
		{
			throw ACCESS_EXCEPTION("Failed to get callback node %s.", CALLBACK_NODE_NAME);
			// NOTE: cerr 보다 throw를 사용하는 것이 더 적합하다.
			// 왜냐하면 throw를 사용하면 예외 처리를 통해 프로그램의 흐름을 제어할 수 있기 때문이다.
			// 콜백 노드를 얻지 못하면 이후 처리가 불가능하므로 throw를 사용하여 예외를 발생시키는 것이 좋다.

			// std::cerr << "Failed to get callback node " << CALLBACK_NODE_NAME << std::endl;
			// return false
		}

		RegisterCallback(pNodeCallback, &OnNodeCallbackFunction, (uint32_t)0, cbPostInsideLock);

		enableExposureEndEvent();

		m_initialized = true;
		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Initialization error: " << e.GetDescription() << std::endl;
		return false;
	}
}

void CameraEventWorker::enableExposureEndEvent()
{
	CEnumerationPtr pEventSelector(m_pNodeMap->GetNode(EVENT_SELECTOR));
	CEnumEntryPtr pEventSelectorEntry(pEventSelector->GetEntryByName(TARGET_EVENT_NAME));
	pEventSelector->SetIntValue(pEventSelectorEntry->GetValue());

	CEnumerationPtr pEventNotification(m_pNodeMap->GetNode(EVENT_NOTIFICATION));
	CEnumEntryPtr pEventNotificationEntry(pEventNotification->GetEntryByName(EVENT_NOTIFICATION_ON));
	pEventNotification->SetIntValue(pEventNotificationEntry->GetValue());
}

void CameraEventWorker::startAcquisition()
{
	if (!m_initialized)
	{
		std::cerr << "Camera is not initialized. Call initialize() first." << std::endl;
		return;
	}

	try
	{
		m_pDevice->StartEventAcquisitionThread();
		m_pDataStream->StartAcquisition(m_imageCount);
		m_pDevice->AcquisitionStart();

		while (m_pDataStream->IsGrabbing())
		{
			CIStStreamBufferPtr pBuffer(m_pDataStream->RetrieveBuffer(5000));
			// TODO: pBuffer = m_pDataStream->RetrieveBuffer(5000); 차이 검사하기

			if (pBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				IStImage* pImage = pBuffer->GetIStImage();

				std::cout << "Block ID: " << pBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size: " << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
					<< " First byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer()))
					<< " Timestamp = " << pBuffer->GetIStStreamBufferInfo()->GetTimestamp() << std::endl;
				// 타임스탬프 : 카메라 내부의 정밀 클럭 기준으로 이미지가 획득된 시점의 시간 정보
			}
			else
			{
				std::cout << "No image present in the buffer." << std::endl;
			}
		}
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Start acquisition error: " << e.GetDescription() << std::endl;
	}
}

void CameraEventWorker::stopAcquisition()
{
	try
	{
		m_pDevice->AcquisitionStop();
		m_pDataStream->StopAcquisition();
		m_pDevice->StopEventAcquisitionThread();
		
		m_initialized = false;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Stop acquisition error: " << e.GetDescription() << std::endl;
	}
}

void __stdcall CameraEventWorker::OnNodeCallbackFunction(GenApi::INode* pINode, uint32_t pParam)
{
	handleNodeCallback(pINode);
}

void CameraEventWorker::handleNodeCallback(GenApi::INode* pINode)
{
	try
	{
		std::stringstream ss;
		ss << pINode->GetName();

		if (IsReadable(pINode))
		{
			CValuePtr pValue(pINode);
			if (pValue)
			{
				ss << " = " << pValue->ToString();
			}
			else
			{
				ss << " is not readable.";
			}
			ss << std::endl;
			std::cout << ss.str();
		}
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Callback error: " << e.GetDescription() << std::endl;
	}
}

// 사용 예시 (main.cpp)
/*
int main()
{
	CameraEventWorker camera(100);
	if (camera.initialize())
	{
		camera.startAcquisition();
	}

	std::cout << "Press Enter to stop acquisition..." << std::endl;
	std::cin.get();
}
*/