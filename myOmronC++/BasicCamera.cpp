#include "BasicCamera.h"

BasicCamera::BasicCamera(std::string rootDir)
	: m_strRootDir(rootDir)
{
}

BasicCamera::~BasicCamera()
{
}

bool BasicCamera::Initialize(const CIStSystemPtr& pSystem)
{
	try
	{
		// Create a camrea device object and connect to the first detected device.
		m_pDevice = pSystem->CreateFirstIStDevice();
		std::cout << "[BasicCamera] " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << " : connected" << std::endl;
		// Create a DataStream object for handling image stream data.
		m_pDataStream = m_pDevice->CreateIStDataStream(0);

		//std::string dstCfgDir = "C:\\Users\\USER\\Pictures\\Features.cfg";
		//NodeMapUtil::Load(m_pDevice, dstCfgDir);
		//NodeMapUtil::DisplayNodes(m_pDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("Root"));

		m_strCameraName = m_pDevice->GetIStDeviceInfo()->GetDisplayName().c_str();
		m_strSerialNumber = m_pDevice->GetIStDeviceInfo()->GetSerialNumber().c_str();

		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[BasicCamera] Initialization error: " << e.GetDescription() << std::endl;
		return false;
	}
}

void BasicCamera::StartAcquisition(uint64_t imageCount)
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
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[BasicCamera] Stop acqiusition error: " << e.GetDescription() << std::endl;
	}
}

void BasicCamera::FreeRunCapture0()
{
	while (m_pDataStream->IsGrabbing())
	{
		CIStStreamBufferPtr pStreamBuffer(m_pDataStream->RetrieveBuffer(5000));

		uint64_t frameID = pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID();

		if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
		{
			IStImage* pImage = pStreamBuffer->GetIStImage();

			// Display the information of the acquired image data.
			std::cout << "BlockId=" << pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
				<< " Size:" << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
				<< " First byte =" << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer())) << std::endl;

		}
	}
}

void BasicCamera::FreeRunCapture1()
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

			// Get image information which changes every frame.
			uint64_t frameID = pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID();
			const EStPixelFormatNamingConvention_t ePFNC = pImage->GetImagePixelFormat();
			const IStPixelFormatInfo* const pPixelFormatInfo = GetIStPixelFormatInfo(ePFNC);
			bool isMono = pPixelFormatInfo->IsMono();
			
			// Save the image
			CIStImageBufferPtr pBuffer(CreateIStImageBuffer());
			ImageProcess::ConvertPixelFormat(pImage, isMono, pBuffer);
			GenICam::gcstring savePath = ImageProcess::SetSavePath(m_strRootDir, m_strCameraName, m_strSerialNumber, frameID);
			ImageProcess::SaveImage<BMP>(pBuffer, savePath);

			// Convert to OpenCV Mat
			Mat mat = ImageProcess::ConvertToMat(pImage);
			//TODO: Image processing
		}
		else
		{
			std::cout << "[BasicCamera] No image data present in the buffer" << std::endl;
		}
	}
}


// Example usage of CameraWorker class
/*
#include "BasicCamera.h"

int main()
{
	int numImages = 100;
	CStApiAutoInit objStApiAutoInit; // Initialize StApi
	CIStSystemPtr pSystem(CreateIStSystem());

	BasicCamera camera(HOME_PC_DIRECTORY);

	if (camera.Initialize(pSystem))
	{
		camera.StartAcquisition(numImages);
		camera.FreeRunCapture0();

		camera.StopAcquisition();
	}
}
*/