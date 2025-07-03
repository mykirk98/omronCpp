#include "BasicCamera.h"

BasicCamera::BasicCamera(uint64_t imageCount)
	: m_imageCount(imageCount)
	, m_saveRootDir("C:\\Users\\mykir\\Work\\Experiments\\") //NOTE: LAB WINDOWS PC DIRECTORY
	//, m_saveRootDir("C:\\Users\\USER\\Pictures\\")	//NOTE: HOME PC DIRECTORY
	//, m_saveRootDir("/home/msis/Pictures/SentechExperiments/Experiments1/";)	//NOTE: LAB LINUX PC DIRECTORY
{
}

BasicCamera::~BasicCamera()
{
	StopAcquisition();
}

bool BasicCamera::Initialize(const CIStSystemPtr& pSystem)
{
	try
	{
		// Create a camrea device object and connect to the first detected device.
		m_pDevice = pSystem->CreateFirstIStDevice();//TODO: find another way to connect to a specific camera
		std::cout << "Device=" << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;
		// Create a DataStream object for handling image stream data.
		m_pDataStream = m_pDevice->CreateIStDataStream(0);
		
		std::cout << "[BasicCamera] " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << ": connected." << std::endl;
		std::cout << "[BasicCamera] serial number: " << m_pDevice->GetIStDeviceInfo()->GetSerialNumber() << std::endl;
		std::cout << "[BasicCamera] # of available data stream" << m_pDevice->GetDataStreamCount() << std::endl;
		
		//std::string dstCfgDir = "C:\\Users\\USER\\Pictures\\Features.cfg";
		//CameraConfigurator::Load(m_pDevice, dstCfgDir);
		//CameraConfigurator::DisplayNodes(m_pDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("Root"));

		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Initialization error: " << e.GetDescription() << std::endl;
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
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Start acquisition error: " << e.GetDescription() << std::endl;
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
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Stop acqiusition error: " << e.GetDescription() << std::endl;
	}
}

void BasicCamera::PrintFrameInfo(const IStImage* pImage, CIStStreamBufferPtr& pStreamBuffer)
{
	try
	{
		//NOTE: Difference between Frame and Image:
		// Frame: Frame is a logical grouping of image data that may contain multiple images or metadata. 
		// Image: Image is a single image data that is part of a frame.
		std::cout << "Block ID: " << pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
			<< "\tSize: " << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
			<< "\tFirst byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer()))
			<< "\ttimestamp: " << pStreamBuffer->GetIStStreamBufferInfo()->GetTimestamp()
			<< std::endl;
		// reinterpret_cast : This is used to convert the pointer type without changing the underlying data.
		// dynamic_cast : This is used to safely cast pointers or references to classes in a class hierarchy.
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Printing frame info error: " << e.GetDescription() << std::endl;
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
		std::cerr << "Printing frame info error: " << e.GetDescription() << std::endl;
	}
}

void BasicCamera::LoadSavedImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& srcDir)
{
	try
	{
		// Create a still image file handling class object (filer) for still image processing.
		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));
		
		std::wcout << std::endl << "Loading " << srcDir.c_str() << L"... ";
		pStillImageFiler->Load(pImageBuffer, srcDir);
		
		std::cout << "done." << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Loading image error: " << e.GetDescription() << std::endl;
	}
}

void BasicCamera::SequentialCapture()
{
	size_t numThreads = 1;
	bool convertToColor = true;

	ImageSaverThreadPool saverThreadPool(numThreads, m_saveRootDir, convertToColor);
	saverThreadPool.Start();

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
			//PrintFrameInfo(pImage, frameID);

			FrameData frame;
			frame.pImage = pImage;
			frame.frameID = frameID;
			//frame.serialNumber = m_pDevice->GetIStDeviceInfo()->GetDisplayName().c_str();
			frame.serialNumber = m_pDevice->GetIStDeviceInfo()->GetSerialNumber().c_str();
			frame.timestamp = std::chrono::steady_clock::now();

			saverThreadPool.Enqueue(frame);
		}
		else
		{
			std::cout << "No image data present in the buffer." << std::endl;
		}
	}
	saverThreadPool.Stop();
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

		// image processing and saving logic can be added here...
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}
*/