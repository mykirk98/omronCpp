#include "CameraEventWorker.h"

using namespace GenApi;

namespace
{
	// unnamed namespace : ���� ���� �������� ���� �����ϵ��� �����ϴ� ���
	const char* EVENT_SELECTOR = "EventSelector";
	const char* EVENT_NOTIFICATION = "EventNotification";
	const char* EVENT_NOTIFICATION_ON = "On";
	const char* TARGET_EVENT_NAME = "ExposureEnd";
	const char* CALLBACK_NODE_NAME = "EventExposureEndTimestamp";
}

CameraEventWorker::CameraEventWorker(uint64_t imageCount)
// initializer list : ��� ���� �ʱ�ȭ
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
		// �ý��� ��ü ����
		m_pSystem = CreateIStSystem();

		// ī�޶� ��ü ����
		m_pDevice = m_pSystem->CreateFirstIStDevice();

		// ��ġ ���� ���
		std::cout << "Device: " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;

		// ������ ��Ʈ�� ����
		m_pDataStream = m_pDevice->CreateIStDataStream(0);
		// NOTE:
		// �����ͽ�Ʈ�� : ī�޶󿡼� �̹����� ���������� �޾ƿ��� ��θ� �ǹ�(���������ΰ� ������)

		// ī�޶��� ���� ��Ʈ���� ��� �� ��������
		m_pNodeMap = m_pDevice->GetRemoteIStPort()->GetINodeMap();

		// �ݹ� ��� ��� ��������
		CNodePtr pNodeCallback(m_pNodeMap->GetNode(CALLBACK_NODE_NAME));
		if (pNodeCallback.IsValid() == false)
		{
			throw ACCESS_EXCEPTION("Failed to get callback node %s.", CALLBACK_NODE_NAME);
			// NOTE: cerr ���� throw�� ����ϴ� ���� �� �����ϴ�.
			// �ֳ��ϸ� throw�� ����ϸ� ���� ó���� ���� ���α׷��� �帧�� ������ �� �ֱ� �����̴�.
			// �ݹ� ��带 ���� ���ϸ� ���� ó���� �Ұ����ϹǷ� throw�� ����Ͽ� ���ܸ� �߻���Ű�� ���� ����.

			// std::cerr << "Failed to get callback node " << CALLBACK_NODE_NAME << std::endl;
			// return false
		}

		// ��忡 �ݹ� �Լ� ��� (�̺�Ʈ �߻� �� �ݹ� �Լ� ȣ��)
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
		// �̺�Ʈ ���� ������ ���� (�񵿱� �̺�Ʈ ���ſ�)
		m_pDevice->StartEventAcquisitionThread();
		//NOTE: �̺�Ʈ ���� ���� �����带 �����Ͽ�, ī�޶󿡼� �߻��ϴ� �ϵ���� �̺�Ʈ�� �ǽð����� �����ϰ�,
		//		��ϵ� �ݹ� �Լ��� ȣ��

		// �̹��� ȹ�� ���� (ȣ��Ʈ(PC) ��)
		m_pDataStream->StartAcquisition(m_imageCount);
		// �̹��� ȹ�� ���� (ī�޶� ��)
		m_pDevice->AcquisitionStart();

		while (m_pDataStream->IsGrabbing())
		{
			// ������ ��Ʈ������ ���� ȹ��
			CIStStreamBufferPtr pBuffer(m_pDataStream->RetrieveBuffer(5000));
			// TODO: pBuffer = m_pDataStream->RetrieveBuffer(5000); ���� �˻��ϱ�

			if (pBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				IStImage* pImage = pBuffer->GetIStImage();

				std::cout << "Block ID: " << pBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size: " << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
					<< " First byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer()))
					<< " Timestamp = " << pBuffer->GetIStStreamBufferInfo()->GetTimestamp() << std::endl;
				// Ÿ�ӽ����� : ī�޶� ������ ���� Ŭ�� �������� �̹����� ȹ��� ������ �ð� ����
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
		// �̹��� ȹ�� ���� (ī�޶� ��)
		m_pDevice->AcquisitionStop();
		// �̹��� ȹ�� ���� (ȣ��Ʈ(PC) ��)
		m_pDataStream->StopAcquisition();
		// �̺�Ʈ ���� ������ ����
		m_pDevice->StopEventAcquisitionThread();
		
		m_initialized = false;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Stop acquisition error: " << e.GetDescription() << std::endl;
	}
}

void CameraEventWorker::OnNodeCallbackFunction(GenApi::INode* pINode, uint32_t pParam)
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

// ��� ���� (main.cpp)
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