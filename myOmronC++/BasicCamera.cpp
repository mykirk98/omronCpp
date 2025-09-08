#include "BasicCamera.h"

inline long long now_ns()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

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

		std::string csvPath = m_strRootDir + "log.csv";

		m_csv = std::make_unique<CSVWriter>(csvPath);
		if (!m_csv->is_open())
		{
			std::cerr << "[BasicCamera] Warning: Failed to open CSV file: " << csvPath << std::endl;
		}

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
		const long long t_before_acq = now_ns();
		CIStStreamBufferPtr pStreamBuffer(m_pDataStream->RetrieveBuffer(5000));
		const long long t_after_acq = now_ns();

		uint64_t frameID = pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID();

		if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
		{
			IStImage* pImage = pStreamBuffer->GetIStImage();
		}

		if (m_csv && m_csv->is_open())
		{
			m_csv->WriteRow(frameID, t_before_acq, t_after_acq);
		}

	}
}

void BasicCamera::FreeRunCapture1()
{
	while (m_pDataStream->IsGrabbing())
	{
		// 프레임 수신 대기 타임스탬프
		const long long t_before_acq = now_ns();
		// Retrieve the buffer pointer of image data with a timeout of 5000ms.
		CIStStreamBufferPtr pStreamBuffer(m_pDataStream->RetrieveBuffer(5000));
		const long long t_after_acq = now_ns();
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
			const long long t_after_save = now_ns();

			// Convert to OpenCV Mat
			Mat mat = ImageProcess::ConvertToMat(pImage);
			const long long t_after_cv = now_ns();
			//TODO: Image processing

			if (m_csv && m_csv->is_open())
			{
				m_csv->WriteRow(frameID, t_before_acq, t_after_acq, t_after_save, t_after_cv);
			}
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
		camera.FreeRunCapture1();

		camera.StopAcquisition();
	}
}
*/