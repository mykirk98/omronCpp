#include "EventCamera.h"

using namespace GenApi;

namespace
{
	const char* EVENT_SELECTOR = "EventSelector";
	const char* EVENT_NOTIFICATION = "EventNotification";
	const char* EVENT_NOTIFICATION_ON = "On";
	const char* TARGET_EVENT_NAME = "ExposureEnd";
	const char* CALLBACK_NODE_NAME = "EventExposureEndTimestamp";
}

EventCamera::EventCamera(uint64_t imageCount)
	: m_imageCount(imageCount)
	, m_initialized(false)
{

}

EventCamera::~EventCamera()
{
	stopAcquisition();
}

bool EventCamera::initialize()
{
	try
	{
		m_pSystem = CreateIStSystem();

		m_pDevice = m_pSystem->CreateFirstIStDevice();

		std::cout << "Device: " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;

		m_pDataStream = m_pDevice->CreateIStDataStream(0);
		// NOTE:

		m_pNodeMap = m_pDevice->GetRemoteIStPort()->GetINodeMap();

		CNodePtr pNodeCallback(m_pNodeMap->GetNode(CALLBACK_NODE_NAME));
		if (pNodeCallback.IsValid() == false)
		{
			throw ACCESS_EXCEPTION("Failed to get callback node %s.", CALLBACK_NODE_NAME);

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

void EventCamera::enableExposureEndEvent()
{
	CEnumerationPtr pEventSelector(m_pNodeMap->GetNode(EVENT_SELECTOR));
	CEnumEntryPtr pEventSelectorEntry(pEventSelector->GetEntryByName(TARGET_EVENT_NAME));
	pEventSelector->SetIntValue(pEventSelectorEntry->GetValue());

	CEnumerationPtr pEventNotification(m_pNodeMap->GetNode(EVENT_NOTIFICATION));
	CEnumEntryPtr pEventNotificationEntry(pEventNotification->GetEntryByName(EVENT_NOTIFICATION_ON));
	pEventNotification->SetIntValue(pEventNotificationEntry->GetValue());
}

void EventCamera::startAcquisition()
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

			if (pBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				IStImage* pImage = pBuffer->GetIStImage();

				std::cout << "Block ID: " << pBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size: " << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
					<< " First byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer()))
					<< " Timestamp = " << pBuffer->GetIStStreamBufferInfo()->GetTimestamp() << std::endl;
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

void EventCamera::stopAcquisition()
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

void EventCamera::OnNodeCallbackFunction(GenApi::INode* pINode, uint32_t pParam)
{
	handleNodeCallback(pINode);
}

void EventCamera::handleNodeCallback(GenApi::INode* pINode)
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

// Example usage of EventCamera
/*
#include "EventCamera.h"

int main()
{
	EventCamera camera(100);
	if (camera.initialize())
	{
		camera.startAcquisition();
	}

	std::cout << "Press Enter to stop acquisition..." << std::endl;
	std::cin.get();
}
*/