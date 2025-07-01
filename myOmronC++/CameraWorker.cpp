#include "CameraWorker.h"

CameraWorker::CameraWorker(uint64_t imageCount)
	: m_imageCount(imageCount)
{
}

CameraWorker::~CameraWorker()
{
	StopAcquisition();
}

bool CameraWorker::Initialize(const CIStSystemPtr& pSystem)
{
	try
	{
		// ī�޶� ��ü ����
		m_pDevice = pSystem->CreateFirstIStDevice();//TODO: CreateFirstIStDevice()���� �ٸ� ������� ī�޶� ������ �� ���� ������?
		std::cout << "Device=" << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;
		// �̹��� ��Ʈ�� �����͸� ó���ϱ� ���� �����ͽ�Ʈ�� ��ü ����
		m_pDataStream = m_pDevice->CreateIStDataStream(0);
		
		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Initialization error: " << e.GetDescription() << std::endl;
		return false;
	}
}

void CameraWorker::StartAcquisition()
{
	try
	{
		// ȣ��Ʈ(PC) ���� �̹��� ȹ�� ����
		m_pDataStream->StartAcquisition(m_imageCount);
		// ī�޶� ���� �̹��� ȹ�� ����
		m_pDevice->AcquisitionStart();
		
		std::string dstCfgDir = "C:\\Users\\USER\\Pictures\\Features.cfg";
		//SaveConfigFile(dstCfgDir);
		//LoadConfigFile(dstCfgDir);
		//CameraConfigurator::Load(m_pDevice, dstCfgDir);
		// CameraConfigurator::DisplayNodes(m_pDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("Root"));
		SequentialCapture();
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Start acquisition error: " << e.GetDescription() << std::endl;
	}
}

void CameraWorker::StopAcquisition()
{
	try
	{
		if (m_pDevice)
		{
			// ī�޶� �� �̹��� ȹ�� ����
			m_pDevice->AcquisitionStop();
			// ȣ��Ʈ(PC) �� �̹��� ȹ�� ����
			m_pDataStream->StopAcquisition();
		}
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Stop acqiusition error: " << e.GetDescription() << std::endl;
	}
}

template<typename FORMAT>
void CameraWorker::ConvertAndSaveImage(IStImage* pSrcImage, bool isColor, std::string dstDir, const uint64_t frameID)
{
	try
	{
		// �̹����� �����ϱ� ���� �̹��� ���� ��ü ���� �� �ȼ� ���� ��ȯ
		CIStImageBufferPtr pImageBuffer(CreateIStImageBuffer());
		ConvertPixelFormat(pSrcImage, isColor, pImageBuffer);
		
		// �̹��� ��� ���� �� ����
		GenICam::gcstring savePath = SetSavePath(dstDir, frameID);
		SaveImage<FORMAT>(pImageBuffer, savePath);
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Converting and saving image error: " << e.GetDescription() << std::endl;
	}
}

// ���ø� ������ �ν��Ͻ�ȭ (�� �̹��� ���˿� ���� ȣ���)
template void CameraWorker::ConvertAndSaveImage<StApiRaw>(IStImage*, bool, std::string, uint64_t);
template void CameraWorker::ConvertAndSaveImage<BMP>(IStImage*, bool, std::string, uint64_t);
template void CameraWorker::ConvertAndSaveImage<TIFF>(IStImage*, bool, std::string, uint64_t);
template void CameraWorker::ConvertAndSaveImage<PNG>(IStImage*, bool, std::string, uint64_t);
template void CameraWorker::ConvertAndSaveImage<JPEG>(IStImage*, bool, std::string, uint64_t);
template void CameraWorker::ConvertAndSaveImage<CSV>(IStImage*, bool, std::string, uint64_t);

void CameraWorker::PrintFrameInfo(const IStImage* pImage, CIStStreamBufferPtr& pStreamBuffer)
{
	try
	{
		//NOTE: Frame�� Image�� ������
	// Frame: ���ۿ��� �� �о�� ������
	// Image: �������� �̹��� ��ü�� ��ȯ�ϰų� �̹����� ������ �� �Ҹ�
		std::cout << "Block ID: " << pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
			<< "\tSize: " << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
			<< "\tFirst byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer()))
			<< "\ttimestamp: " << pStreamBuffer->GetIStStreamBufferInfo()->GetTimestamp()
			<< std::endl;
		// reinterpret_cast : ���� ���� ���� ������ Ÿ�� ���� ��ȯ�� �����ϴ� ������
		// dynamic_cast�� �ƴ� static_cast�� ����� ���� :
		// dynamic_cast�� ��� ���谡 �ִ� Ŭ���� ������/������ �����ϰ� ��ȯ�� �� ���Ǹ�,
		// ���⿡���� �ܼ��� �⺻ Ÿ�� ���� ��ȯ(uint8_t* -> uint32_t) �̹Ƿ� static_cast�� ����ص� ����
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Printing frame info error: " << e.GetDescription() << std::endl;
	}
}

void CameraWorker::PrintFrameInfo(const IStImage* pImage, const uint64_t frameID)
{
	try
	{
		//NOTE: Frame�� Image�� ������
	// Frame: ���ۿ��� �� �о�� ������
	// Image: �������� �̹��� ��ü�� ��ȯ�ϰų� �̹����� ������ �� �Ҹ�
		std::cout << "Block ID: " << frameID
			<< "\tSize: " << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
			<< "\tFirst byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer()))
			<< std::endl;
		// reinterpret_cast : ���� ���� ���� ������ Ÿ�� ���� ��ȯ�� �����ϴ� ������
		// dynamic_cast�� �ƴ� static_cast�� ����� ���� :
		// dynamic_cast�� ��� ���谡 �ִ� Ŭ���� ������/������ �����ϰ� ��ȯ�� �� ���Ǹ�,
		// ���⿡���� �ܼ��� �⺻ Ÿ�� ���� ��ȯ(uint8_t* -> uint32_t) �̹Ƿ� static_cast�� ����ص� ����
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Printing frame info error: " << e.GetDescription() << std::endl;
	}
}

void CameraWorker::LoadSavedImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& srcDir)
{
	try
	{
		// �̹��� ���� ������� ���� filer ��ü ����
		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));
		
		//NOTE: w_str(): wide string(wchar_t*) �����ͷ� ��ȯ
		//NOTE: c_str(): char* �����ͷ� ��ȯ
		//NOTE: L: wide string ���ͷ��� �ǹ�, �� ���ڰ� 2����Ʈ�� ǥ����
		std::wcout << std::endl << L"Loading " << srcDir.c_str() << L"... ";
		pStillImageFiler->Load(pImageBuffer, srcDir);
		
		std::cout << "done." << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Loading image error: " << e.GetDescription() << std::endl;
	}
}

void CameraWorker::SequentialCapture()
{
	while (m_pDataStream->IsGrabbing())
	{
		// ���� �����͸� 5000ms�� Ÿ�Ӿƿ����� �˻�
		CIStStreamBufferPtr pStreamBuffer(m_pDataStream->RetrieveBuffer(5000));

		// ȹ���� �����Ϳ� �̹��� �����Ͱ� �ִ��� Ȯ��
		if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
		{
			// IStImage ��ü ����
			IStImage* pImage = pStreamBuffer->GetIStImage();

			const uint64_t frameID = pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID();
			PrintFrameInfo(pImage, frameID);

			//std::string targetDir = "C:\\Users\\mykir\\Work\\Experiments\\";	//NOTE: LAB PC DIRECTORY
			// std::string targetDir = "C:\\Users\\USER\\Pictures\\";//NOTE: HOME PC DIRECTORY
			// std::string targetDir = "/home/msis/Pictures/SentechExperiments/Experiments1/";	//NOTE: LAB LINUX DIRECTORY
			// ConvertAndSaveImage<BMP>(pImage, true, targetDir, frameID);
		}
		else
		{
			std::cout << "No image data present in the buffer." << std::endl;
		}
	}
}

GenICam::gcstring CameraWorker::SetSavePath(const std::string& savePath, const uint64_t frameID)
{
	try
	{
		// frameID�� ���ڿ��� ��ȯ
		std::string strFrameID = std::to_string(frameID);

		// ����� ���� ��ο� frameID�� �����Ͽ� ���� ��� ����
		std::string filePath = savePath + m_pDevice->GetIStDeviceInfo()->GetDisplayName().c_str() + strFrameID;

		return GenICam::gcstring(filePath.c_str());
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Setting save path error: " << e.GetDescription() << std::endl;
		return GenICam::gcstring();
	}
}

void CameraWorker::ConvertPixelFormat(IStImage* pSrcImage, bool isColor, CIStImageBufferPtr& pDstBuffer)
{
	try
	{
		// �ȼ� ���� ��ȯ�� ���� converter ��ü ����
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
void CameraWorker::SaveImage(CIStImageBufferPtr& pImageBuffer, GenICam::gcstring& dstDir)
{
	try
	{
		// �̹��� ���� ��ο� Ȯ���� �߰� by ���ø�
		dstDir.append(FORMAT::extension);
		
		// �̹��� ������ ���� filer ��ü ����
		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));
		
		// �̹��� ����
		std::wcout << L"Saving " << dstDir.c_str() << L"... ";
		pStillImageFiler->Save(pImageBuffer->GetIStImage(), FORMAT::fileFormat, dstDir);
		std::cout << "done" << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Saving image error: " << e.GetDescription() << std::endl;
	}
}


// ��� ���� (main.cpp���� ȣ��)
/*
int main()
{
	CameraWorker cameraWorker(10); // 10���� �̹��� ȹ��
	if (cameraWorker.initialize())
	{
		cameraWorker.StartAcquisition();

		// ... �̹��� ó�� ���� ...
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}
*/