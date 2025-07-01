#include "BasicCamera.h"

BasicCamera::BasicCamera(uint64_t imageCount)
	: m_imageCount(imageCount)
	, m_pImage(nullptr)
	, m_frameID(0)
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
		
		std::string dstCfgDir = "C:\\Users\\USER\\Pictures\\Features.cfg";
		//SaveConfigFile(dstCfgDir);
		//LoadConfigFile(dstCfgDir);
		//CameraConfigurator::Load(m_pDevice, dstCfgDir);
		//CameraConfigurator::DisplayNodes(m_pDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("Root"));
		SequentialCapture();
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
		if (m_pDevice)
		{
			// Stop the image acquisition of the camera side.
			m_pDevice->AcquisitionStop();
			// Stop the image acquisition of the host(PC) side.
			m_pDataStream->StopAcquisition();
		}
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Stop acqiusition error: " << e.GetDescription() << std::endl;
	}
}

void BasicCamera::SaveImageToFile(const std::string& dstDir)
{
	ConvertAndSaveImage<BMP>(m_pImage, true, dstDir, m_frameID);
}

template<typename FORMAT>
void BasicCamera::ConvertAndSaveImage(IStImage* pSrcImage, bool isColor, std::string dstDir, const uint64_t frameID)
{
	try
	{
		// Create an image buffer to hold the converted image data.
		CIStImageBufferPtr pImageBuffer(CreateIStImageBuffer());
		ConvertPixelFormat(pSrcImage, isColor, pImageBuffer);
		
		// Convert the image data to the specified extension format and save it.
		GenICam::gcstring savePath = SetSavePath(dstDir, frameID);
		SaveImage<FORMAT>(pImageBuffer, savePath);
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Converting and saving image error: " << e.GetDescription() << std::endl;
	}
}

// Explicit template instantiation for different image formats
template void BasicCamera::ConvertAndSaveImage<StApiRaw>(IStImage*, bool, std::string, uint64_t);
template void BasicCamera::ConvertAndSaveImage<BMP>(IStImage*, bool, std::string, uint64_t);
template void BasicCamera::ConvertAndSaveImage<TIFF>(IStImage*, bool, std::string, uint64_t);
template void BasicCamera::ConvertAndSaveImage<PNG>(IStImage*, bool, std::string, uint64_t);
template void BasicCamera::ConvertAndSaveImage<JPEG>(IStImage*, bool, std::string, uint64_t);
template void BasicCamera::ConvertAndSaveImage<CSV>(IStImage*, bool, std::string, uint64_t);

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
	while (m_pDataStream->IsGrabbing())
	{
		// Retrieve the buffer pointer of image data with a timeout of 5000ms.
		CIStStreamBufferPtr pStreamBuffer(m_pDataStream->RetrieveBuffer(5000));

		// Check if the acquired data contains image data.
		if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
		{
			// If yes, we create a IStImage object for further image handling.
			m_pImage = pStreamBuffer->GetIStImage();

			m_frameID = pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID();
			PrintFrameInfo(m_pImage, m_frameID);

			std::string targetDir = "C:\\Users\\mykir\\Work\\Experiments\\";	//NOTE: LAB PC DIRECTORY
			// std::string targetDir = "C:\\Users\\USER\\Pictures\\";//NOTE: HOME PC DIRECTORY
			// std::string targetDir = "/home/msis/Pictures/SentechExperiments/Experiments1/";	//NOTE: LAB LINUX DIRECTORY
			SaveImageToFile(targetDir);
		}
		else
		{
			std::cout << "No image data present in the buffer." << std::endl;
		}
	}
}

GenICam::gcstring BasicCamera::SetSavePath(const std::string& savePath, const uint64_t frameID)
{
	try
	{
		// change frameID to string
		std::string strFrameID = std::to_string(frameID);

		// Ensure the save path ends with a separator
		std::string filePath = savePath + m_pDevice->GetIStDeviceInfo()->GetDisplayName().c_str() + strFrameID;

		return GenICam::gcstring(filePath.c_str());
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Setting save path error: " << e.GetDescription() << std::endl;
		return GenICam::gcstring();
	}
}

void BasicCamera::ConvertPixelFormat(IStImage* pSrcImage, bool isColor, CIStImageBufferPtr& pDstBuffer)
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
		pPixelFormatConverter->Convert(pSrcImage, pDstBuffer);
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Converting pixel format error: " << e.GetDescription() << std::endl;
	}
}

template<typename FORMAT>
void BasicCamera::SaveImage(CIStImageBufferPtr& pImageBuffer, GenICam::gcstring& dstDir)
{
	try
	{
		// Ensure the destination directory has the correct format
		dstDir.append(FORMAT::extension);
		
		// Create a still image file handling class object (filer) for still image processing.
		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));
		
		// Save the image file in the specified format using the filer we created.
		std::wcout << L"Saving " << dstDir.c_str() << L"... ";
		pStillImageFiler->Save(pImageBuffer->GetIStImage(), FORMAT::fileFormat, dstDir);
		std::cout << "done" << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Saving image error: " << e.GetDescription() << std::endl;
	}
}


// Example usage of CameraWorker class
/*
#include "CameraWorker.h"

int main()
{
	std::cout << "Basic Camera Example" << std::endl;
	CStApiAutoInit objStApiAutoInit; // Initialize StApi
	CIStSystemPtr pSystem(CreateIStSystem()); // Create a system object for device scan and connection

	std::string targetDir = "C:\\Users\\mykir\\Work\\Experiments\\";	//NOTE: LAB PC DIRECTORY
	CameraWorker cameraWorker(10);
	if (cameraWorker.Initialize(pSystem))
	{
		cameraWorker.StartAcquisition();

		// image processing and saving logic can be added here...
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}
*/