#include "GigECamera.h"

GigECamera::GigECamera(std::string saveRootDir)
	: m_pInterface(nullptr)
	, m_strSaveRootDir(saveRootDir)
	, pICommandTriggerSoftware(nullptr)
	, m_strSerialNumber("")
	, m_strUserDefinedName("")
{
}

GigECamera::~GigECamera()
{
	StopAcquisition();
}

bool GigECamera::Initialize(IStInterface* pInterface, uint32_t interfaceDeviceIndex)
{
	try
	{
		m_pInterface = pInterface;
		std::cout << "[GigECamera] Interface: " << m_pInterface->GetIStInterfaceInfo()->GetDisplayName() << " initialized" << std::endl;

		GigEUtil::UpdateDeviceIPAddress(m_pInterface->GetIStPort()->GetINodeMap(), interfaceDeviceIndex, m_pInterface->GetIStDeviceInfo(interfaceDeviceIndex)->GetSerialNumber(), m_strUserDefinedName);

		GenApi::CIntegerPtr pGevDeviceForceIPAddress(m_pInterface->GetIStPort()->GetINodeMap()->GetNode(GEV_DEVICE_FORCE_IP_ADDRESS));
		const int64_t nDeviceIPAddress = pGevDeviceForceIPAddress->GetValue();

		for (size_t i = 0; i < 30; ++i)
		{
#ifdef _WIN32
			Sleep(1000);  // 1000 ms
#else
			usleep(1000 * 1000);  // 1000ms = 1초 (usleep은 마이크로초 단위)
#endif
			IStDeviceReleasable* pDeviceReleasable(GigEUtil::CreateIStDeviceByIPAddress(m_pInterface, nDeviceIPAddress));
			if (pDeviceReleasable != NULL)
			{
				m_pDevice.Reset(pDeviceReleasable);
				break;
			}
		}
		if (!m_pDevice.IsValid())
		{
			throw RUNTIME_EXCEPTION("A device with an IP address of %s could not be found.", pGevDeviceForceIPAddress->ToString().c_str());
		}
		std::cout << "[GigECamera] " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << ": connected" << std::endl;
		
		m_strSerialNumber = m_pDevice->GetIStDeviceInfo()->GetSerialNumber().c_str();
		// Get the INodeMap interface pointer for the camera settings.
		GenApi::CNodeMapPtr pINodeMap(m_pDevice->GetRemoteIStPort()->GetINodeMap());
		// Set the TriggerSelector to FrameStart.
		NodeMapUtil::SetTriggerMode(pINodeMap, TRIGGER_SELECTOR_FRAME_START, TRIGGER_MODE_ON, TRIGGER_SOURCE_SOFTWARE);
		// Set the ICommand interface pointer for the TriggerSoftware node.
		pICommandTriggerSoftware = pINodeMap->GetNode(TRIGGER_SOFTWARE);

		m_pDataStream = m_pDevice->CreateIStDataStream(0);
		std::cout << "[GigECamera] # of available data streams: " << m_pDevice->GetDataStreamCount() << std::endl;

		RegisterCallback(m_pDataStream, &GigECamera::OnStCallbackMethod, this);

		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cout << "[GigECamera] Initialization error: " << e.GetDescription() << std::endl;
		return false;
	}
}

void GigECamera::StartAcquisition()
{
	try
	{
		// Start the image acquisition of the host(PC) side.
		m_pDataStream->StartAcquisition();
		// Start the image acquisition of the camera side.
		m_pDevice->AcquisitionStart();
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[GigECamera] Start acquisition error: " << e.GetDescription() << std::endl;
	}
}

void GigECamera::StopAcquisition()
{
	try
	{
		// Stop the image acquisition of the camera side.
		m_pDevice->AcquisitionStop();
		// Stop the image acquisition of the host(PC) side.
		m_pDataStream->StopAcquisition();
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[GigECamera] Stop acquisition error: " << e.GetDescription() << std::endl;
	}
}

void GigECamera::SequentialCapture()
{
	// A while loop for acquiring data and checking status.
	// Here, the acquisition runs until it reaches the assigned numbers of frames.
	while (m_pDataStream->IsGrabbing())
	{
		// Retrieve the buffer pointer of image data with a timeout of 5000ms.
		CIStStreamBufferPtr pStreamBuffer(m_pDataStream->RetrieveBuffer(5000));
		// Check if the acquired data contains image data.
		if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
		{
			// If yes, we create a IStImage object for further image handling.
			IStImage* pImage = pStreamBuffer->GetIStImage();
			//ImageProcess::PrintFrameInfo(pStreamBuffer, m_strUserDefinedName);
		}
	}
}

void GigECamera::ExecuteTrigger()
{
	pICommandTriggerSoftware->Execute();
}

void GigECamera::SetFrameQueue(std::shared_ptr<ThreadSafeQueue<FrameData>> pFrameQueue)
{
	m_pFrameQueue = pFrameQueue;
}

const std::string& GigECamera::GetUserDefinedName()
{
	return m_strUserDefinedName;
}

const std::string& GigECamera::GetSerialNumber()
{
	return m_strSerialNumber;
}

void GigECamera::OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext)
{
	if (pvContext)
	{
		// pvContext is a pointer to the TriggerCamera instance
		static_cast<GigECamera*>(pvContext)->OnCallback(pIStCallbackParamBase);
		//NOTE: static_cast is used here to convert the void pointer back to TriggerCamera pointer
	}
}

void GigECamera::OnCallback(IStCallbackParamBase* pCallbackParam)
{
	try
	{
		// Check callback type. Only NewBuffer event is handled in here
		if (pCallbackParam->GetCallbackType() == StCallbackType_GenTLEvent_DataStreamNewBuffer)
		{
			IStCallbackParamGenTLEventNewBuffer* pNewBufferParam = dynamic_cast<IStCallbackParamGenTLEventNewBuffer*>(pCallbackParam);
			//NOTE: dynamic_cast is used to safely cast the base class pointer to the derived class pointer.
			//NOTE: static_cast is used when you are sure about the type of the object, while dynamic_cast is used for safe downcasting in class hierarchies.

			// Get the IStDataStream interface pointer from the received callback parameter.
			IStDataStream* pDataStream = pNewBufferParam->GetIStDataStream();

			// Retrieve the buffer pointer of image data for that callback indicated there is a buffer received.
			CIStStreamBufferPtr pStreamBuffer(pDataStream->RetrieveBuffer(0));

			// Check if the acquired data contains image data.
			if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				// If yes, we create a IStImage object for further image handling.
				IStImage* pImage = pStreamBuffer->GetIStImage();
				const EStPixelFormatNamingConvention_t ePFNC = pImage->GetImagePixelFormat();
				const IStPixelFormatInfo* const pPixelFormatInfo = GetIStPixelFormatInfo(ePFNC);

				//ImageProcess::PrintFrameInfo(pStreamBuffer, m_strUserDefinedName);

				FrameData frame;
				frame.pImage = pImage;
				frame.serialNumber = GetSerialNumber();
				frame.frameID = pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID();
				frame.cameraName = GetUserDefinedName();
				frame.isMono = pPixelFormatInfo->IsMono();

				// Push the frame data into the frame queue for further processing.
				if (m_pFrameQueue)
					m_pFrameQueue->Push(frame);
				else
					std::cerr << "[GigECamera] Frame queue is not set. Cannot push frame data." << std::endl;

				Mat mat = ImageProcess::ConvertToMat(pImage);
				//std::cout << "[GigECamera] Image converted to openCV Mat." << std::endl;
				Sleep(75);
			}
			else
			{
				std::cout << "[GigECamera] No image present in the buffer." << std::endl;
			}
		}
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[GigECamera] Callback Exception: " << e.GetDescription() << std::endl;
	}
}


// Example usage of GigECamera class
/*
#include "GigECamera.h"

int main()
{
	std::cout << "========== GigE Camera Example ==========" << std::endl;

	CStApiAutoInit stApiAutoInit;
	CIStSystemPtr pSystem(CreateIStSystem(StSystemVendor_Default, StInterfaceType_GigEVision));
	//CIStSystemPtr pSystem(CreateIStSystem());

	GigECamera camera;
	if (camera.Initialize(pSystem->GetIStInterface(1), 0))
	{
		camera.StartAcquisition();

		//camera.SequentialCapture(); // Capture images sequentially
		while (true)
		{
			std::cout << "Press 0 to trigger an image or 1 to exit: ";
			int input;
			std::cin >> input;

			if (input == 0)
			{
				std::cout << "Triggering image..." << std::endl;
				camera.pICommandTriggerSoftware->Execute(); // Execute software trigger
			}
			else if (input == 1)
			{
				std::cout << "Exiting..." << std::endl;
				break; // Exit the loop
			}
			else
			{
				std::cout << "Invalid input. Please enter 0 or 1." << std::endl;
			}
		}
		camera.StopAcquisition(); // Stop acquisition
	}
	return 0;
}
*/