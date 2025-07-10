#include "GigECamera.h"

GigECamera::GigECamera()
	: m_pInterface(nullptr)
	, m_saveRootDir("C:\\Users\\mykir\\Work\\Experiments\\") //NOTE: LAB WINDOWS PC DIRECTORY
{
}

GigECamera::~GigECamera()
{
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

		m_pDataStream = m_pDevice->CreateIStDataStream(0);
		std::cout << "[GigECamera] # of available data streams: " << m_pDevice->GetDataStreamCount() << std::endl;

		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cout << "[GigECamera] Initialization error: " << e.GetDescription() << std::endl;
		return false;
	}


}

void GigECamera::StartAcquisition(uint64_t imageCount)
{
	try
	{
		// Start the image acquisition of the host(PC) side.
		m_pDataStream->StartAcquisition(imageCount);
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
			
			if (m_pThreadPool)
			{
				FrameData frameData;
				frameData.serialNumber = m_pDevice->GetIStDeviceInfo()->GetSerialNumber();
				frameData.frameID = pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID();
				frameData.pImage = pImage;

				m_pThreadPool->Enqueue(frameData);
			}
		}
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
		std::cerr << "[BasicCamera] Printing frame info error: " << e.GetDescription() << std::endl;
	}
}

void GigECamera::SetThreadPool(std::shared_ptr<ImageSaverThreadPool> pThreadPool)
{
	m_pThreadPool = pThreadPool;
}

// Example usage of GigECamera class
/*
#include "GigECamera.h"

int main()
{
	std::cout << "========== GigE Camera Example ==========" << std::endl;

	int numImages = 3; // Number of images to capture

	CStApiAutoInit stApiAutoInit;
	CIStSystemPtr pSystem(CreateIStSystem(StSystemVendor_Default, StInterfaceType_GigEVision));
	//CIStSystemPtr pSystem(CreateIStSystem());

	std::shared_ptr<ImageSaverThreadPool> imageSaverThreadPool = std::make_shared<ImageSaverThreadPool>(1, "C:\\Users\\mykir\\Work\\Experiments\\", true);
	imageSaverThreadPool->Start();

	GigECamera camera;
	if (camera.Initialize(pSystem->GetIStInterface(1), 0))
	{
		camera.StartAcquisition(numImages);
		camera.SetThreadPool(imageSaverThreadPool);

		std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
		camera.SequentialCapture(); // Capture images sequentially
		std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsedSeconds = endTime - startTime;
		double averageFPS = numImages / elapsedSeconds.count(); // Calculate average FPS
		std::cout << "[Main] Average FPS: " << averageFPS << std::endl; // Display average FPS

		camera.StopAcquisition(); // Stop acquisition
	}
	imageSaverThreadPool->Stop(); // Stop the image saver thread pool
	return 0;
}
*/