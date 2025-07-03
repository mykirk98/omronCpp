#include "BasicCamera.h"
#include "config.h"

#define LOGGING

BasicCamera::BasicCamera(uint64_t imageCount)
	: m_imageCount(imageCount)
	, m_saveRootDir("C:\\Users\\mykir\\Work\\Experiments\\") //NOTE: LAB WINDOWS PC DIRECTORY
	//, m_saveRootDir("C:\\Users\\USER\\Pictures\\")	//NOTE: HOME PC DIRECTORY
	//, m_saveRootDir("/home/msis/Pictures/SentechExperiments/Experiments1/";)	//NOTE: LAB LINUX PC DIRECTORY
{
#ifdef LOGGING
	std::cout << "[BasicCamera] constructed" << std::endl;
#endif // LOGGING

}

BasicCamera::~BasicCamera()
{
#ifdef LOGGING
	std::cout << "[BasicCamera] destructed" << std::endl;
#endif // LOGGING
}

bool BasicCamera::Initialize(const CIStSystemPtr& pSystem)
{
	try
	{
		// Create a camrea device object and connect to the first detected device.
		m_pDevice = pSystem->CreateFirstIStDevice();//TODO: find another way to connect to a specific camera
		std::cout << "[BasicCamera] " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << " : connected" << std::endl;
		// Create a DataStream object for handling image stream data.
		m_pDataStream = m_pDevice->CreateIStDataStream(0);
		std::cout << "[BasicCamera] # of available data stream : " << m_pDevice->GetDataStreamCount() << std::endl;
		
		//std::string dstCfgDir = "C:\\Users\\USER\\Pictures\\Features.cfg";
		//CameraConfigurator::Load(m_pDevice, dstCfgDir);
		//CameraConfigurator::DisplayNodes(m_pDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("Root"));

#ifdef LOGGING
		std::cout << "[BasicCamera] Initialized" << std::endl;
#endif // LOGGING

		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[BasicCamera] Initialization error: " << e.GetDescription() << std::endl;
		return false;
	}
}

void BasicCamera::StartAcquisition()
{
	try
	{
		// Start the image acquisition of the host(PC) side.
		m_pDataStream->StartAcquisition(m_imageCount);
		// Start the image acquisition of the camera side.
		m_pDevice->AcquisitionStart();

#ifdef LOGGING
		std::cout << "[BasicCamera] Acquisition started" << std::endl;
#endif // LOGGING
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[BasicCamera] Start acquisition error: " << e.GetDescription() << std::endl;
	}
}

void BasicCamera::StopAcquisition()
{
	try
	{
		// Stop the image acquisition of the camera side.
		m_pDevice->AcquisitionStop();
		// Stop the image acquisition of the host(PC) side.
		m_pDataStream->StopAcquisition();
#ifdef LOGGING
		std::cout << "[BasicCamera] Acquisition stopped" << std::endl;
#endif // LOGGING

	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[BasicCamera] Stop acqiusition error: " << e.GetDescription() << std::endl;
	}
}

void BasicCamera::PrintFrameInfo(const IStImage* pImage, const uint64_t frameID)
{
	try
	{
		std::cout << "Block ID: " << frameID
			<< "\tSize: " << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
			<< "\tFirst byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer()))
			<< std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[BasicCamera] Printing frame info error: " << e.GetDescription() << std::endl;
	}
}

void BasicCamera::LoadSavedImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& srcDir)
{
	try
	{
		// Create a still image file handling class object (filer) for still image processing.
		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));
		
		std::wcout << std::endl << "[BasicCamera] Loading " << srcDir.c_str() << L"... ";
		pStillImageFiler->Load(pImageBuffer, srcDir);
		std::cout << "done" << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[BasicCamera] Loading image error: " << e.GetDescription() << std::endl;
	}
}

void BasicCamera::SequentialCapture()
{
	while (m_pDataStream->IsGrabbing())
	{
		// Retrieve the buffer pointer of image data with a timeout of 5000ms.
		CIStStreamBufferPtr pStreamBuffer(m_pDataStream->RetrieveBuffer(5000));

		// Check if the acquired data contains image data.
		if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
		{
			// If yes, we create a IStImage object for further image handling.
			IStImage* pImage = pStreamBuffer->GetIStImage();
			uint64_t frameID = pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID();
			PrintFrameInfo(pImage, frameID);
		}
		else
		{
			std::cout << "[BasicCamera] No image data present in the buffer" << std::endl;
		}
	}
#ifdef LOGGING
	std::cout << "[BasicCamera] Sequential capture completed" << std::endl;
#endif // LOGGING

}


// Example usage of CameraWorker class
/*
#include "BasicCamera.h"

int main()
{
	std::cout << "==========Basic Camera Example==========" << std::endl;
	CStApiAutoInit objStApiAutoInit; // Initialize StApi
	CIStSystemPtr pSystem(CreateIStSystem()); // Create a system object for device scan and connection

	BasicCamera basicCamera(10);
	if (basicCamera.Initialize(pSystem))
	{
		basicCamera.StartAcquisition();

		basicCamera.SequentialCapture();

		basicCamera.StopAcquisition();
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}
*/