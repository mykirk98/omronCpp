#include "GigECamera.h"

GigECamera::GigECamera(std::string saveRootDir)
	: m_pInterface(nullptr)
	, m_saveRootDir(saveRootDir)
	, pICommandTriggerSoftware(nullptr)
	, m_serialNumber("")
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

		GigEConfigurator::UpdateDeviceIPAddress(m_pInterface->GetIStPort()->GetINodeMap(), interfaceDeviceIndex, m_pInterface->GetIStDeviceInfo(interfaceDeviceIndex)->GetSerialNumber());

		GenApi::CIntegerPtr pGevDeviceForceIPAddress(m_pInterface->GetIStPort()->GetINodeMap()->GetNode(GEV_DEVICE_FORCE_IP_ADDRESS));
		const int64_t nDeviceIPAddress = pGevDeviceForceIPAddress->GetValue();

		for (size_t i = 0; i < 30; ++i)
		{
			Sleep(1000);
			IStDeviceReleasable* pDeviceReleasable(GigEConfigurator::CreateIStDeviceByIPAddress(m_pInterface, nDeviceIPAddress));
			//if (pDeviceReleasable != nullptr)
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

		m_serialNumber = m_pDevice->GetIStDeviceInfo()->GetSerialNumber();
		// Get the INodeMap interface pointer for the camera settings.
		GenApi::CNodeMapPtr pINodeMap(m_pDevice->GetRemoteIStPort()->GetINodeMap());
		// Set the TriggerSelector to FrameStart.
		SetTriggerMode(pINodeMap, TRIGGER_SELECTOR_FRAME_START, TRIGGER_MODE_ON, TRIGGER_SOURCE_SOFTWARE);
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
			PrintFrameInfo(pStreamBuffer);

			CIStImageBufferPtr pImageBuffer(CreateIStImageBuffer());
			ConvertPixelFormat(pImage, true, pImageBuffer);
			GenICam::gcstring savePath = SetSavePath(pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID());
			SaveImage<BMP>(pImageBuffer, savePath);
		}
	}
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

				PrintFrameInfo(pStreamBuffer);

				CIStImageBufferPtr pImageBuffer(CreateIStImageBuffer());
				ConvertPixelFormat(pImage, true, pImageBuffer);
				GenICam::gcstring savePath = SetSavePath(pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID());
				SaveImage<BMP>(pImageBuffer, savePath);

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

void GigECamera::SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName)
{
	try
	{
		// Get the IEnumeration interface pointer for the specified enumeration name.
		GenApi::CEnumerationPtr pIEnumeration(pInodeMap->GetNode(szEnumerationName));

		// Get the IEnumEntry interface pointer for the specified value name.
		GenApi::CEnumEntryPtr pIEnumEntry(pIEnumeration->GetEntryByName(szValueName));

		// Get the integer value corresponding to the set value name using the IEnumEntry interface pointer.
		// Update the settings using the IEnumeration interface pointer.
		pIEnumeration->SetIntValue(pIEnumEntry->GetValue());
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[GigECamera] Setting enumeration failed: " << e.GetDescription() << std::endl;
	}
}

void GigECamera::SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource)
{
	try
	{
		// Set the TriggerSelector to FrameStart.
		SetEnumeration(pINodeMap, TRIGGER_SELECTOR, triggerSelector);
		// Set the TriggerMode to On.
		SetEnumeration(pINodeMap, TRIGGER_MODE, triggerMode);
		// Set the TriggerSource to Software.
		SetEnumeration(pINodeMap, TRIGGER_SOURCE, triggerSource);
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[GigECamera] Setting trigger mode failed: " << e.GetDescription() << std::endl;
	}
}

void GigECamera::PrintFrameInfo(const CIStStreamBufferPtr& pStreamBuffer)
{
	try
	{
		std::cout << "Block ID: " << pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
			<< "\tSize: " << pStreamBuffer->GetIStImage()->GetImageWidth() << " x " << pStreamBuffer->GetIStImage()->GetImageHeight()
			<< "\tFirst byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pStreamBuffer->GetIStImage()->GetImageBuffer()))
			<< "\ttime stamp: " << pStreamBuffer->GetIStStreamBufferInfo()->GetTimestamp()
			<< std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[GigECamera] Printing frame info error: " << e.GetDescription() << std::endl;
	}
}

void GigECamera::SetThreadPool(std::shared_ptr<ImageSaverThreadPool> pThreadPool)
{
	m_pThreadPool = pThreadPool;
}

void GigECamera::ConvertPixelFormat(IStImage* pSrcImage, bool isColor, CIStImageBufferPtr& pDstBuffer)
{
	try
	{
		// Create a data converter object for pixel format conversion.
		CIStPixelFormatConverterPtr pPixelFormatConverter(CreateIStConverter(StConverterType_PixelFormat));

		if (isColor)
		{
			pPixelFormatConverter->SetDestinationPixelFormat(StPFNC_BGR8);
		}
		else
		{
			pPixelFormatConverter->SetDestinationPixelFormat(StPFNC_Mono8);
		}
		// Convert the pixel format of the source image to the destination buffer.
		pPixelFormatConverter->Convert(pSrcImage, pDstBuffer);
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[GigECamera] Converting pixel format error: " << e.GetDescription() << std::endl;
	}
}

GenICam::gcstring GigECamera::SetSavePath(const uint64_t frameID)
{
	try
	{
		std::string filePath = m_saveRootDir + m_serialNumber.c_str() + "-" + std::to_string(frameID);

		return GenICam::gcstring(filePath.c_str());
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[GigECamera] Setting save path error: " << e.GetDescription() << std::endl;
	}
	return GenICam::gcstring();
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