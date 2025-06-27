#include "CameraWorker.h"

CameraWorker::CameraWorker(uint64_t imageCount)
	: m_imageCount(imageCount)
{
}

CameraWorker::~CameraWorker()
{
	StopAcquisition();
}

bool CameraWorker::initialize()
{
	try
	{
		// 시스템 객체 생성 (장치 검색 및 연결)
		m_pSystem = CreateIStSystem();
		// 첫 번째 장치 생성 및 연결
		m_pDevice = m_pSystem->CreateFirstIStDevice();
		std::cout << "Device=" << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;
		// 이미지 스트림 데이터를 처리하기 위한 데이터스트림 객체 생성
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
		// 호스트(PC) 측의 이미지 획득 시작
		m_pDataStream->StartAcquisition(m_imageCount);
		// 카메라 측의 이미지 획득 시작
		m_pDevice->AcquisitionStart();
		
		std::string dstCfgDir = "C:\\Users\\USER\\Pictures\\Features.cfg";
		//SaveConfigFile(dstCfgDir);
		LoadConfigFile(dstCfgDir);
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
			// 카메라 측 이미지 획득 중지
			m_pDevice->AcquisitionStop();
			// 호스트(PC) 측 이미지 획득 중지
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
		// 이미지를 저장하기 위한 이미지 버퍼 객체 생성 및 픽셀 포맷 변환
		CIStImageBufferPtr pImageBuffer(CreateIStImageBuffer());
		ConvertPixelFormat(pSrcImage, isColor, pImageBuffer);
		
		// 이미지 경로 설정 및 저장
		GenICam::gcstring savePath = SetSavePath(dstDir, frameID);
		SaveImage<FORMAT>(pImageBuffer, savePath);
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Converting and saving image error: " << e.GetDescription() << std::endl;
	}
}

// 템플릿 명시적 인스턴스화 (각 이미지 포맷에 대해 호출됨)
template void CameraWorker::ConvertAndSaveImage<StApiRaw>(IStImage*, bool, std::string, uint64_t);
template void CameraWorker::ConvertAndSaveImage<BMP>(IStImage*, bool, std::string, uint64_t);
template void CameraWorker::ConvertAndSaveImage<TIFF>(IStImage*, bool, std::string, uint64_t);
template void CameraWorker::ConvertAndSaveImage<PNG>(IStImage*, bool, std::string, uint64_t);
template void CameraWorker::ConvertAndSaveImage<JPEG>(IStImage*, bool, std::string, uint64_t);
template void CameraWorker::ConvertAndSaveImage<CSV>(IStImage*, bool, std::string, uint64_t);


void CameraWorker::PrintFrameInfo(const IStImage* pImage, const uint64_t frameID)
{
	try
	{
		//NOTE: Frame과 Image의 차이점
	// Frame: 버퍼에서 갓 읽어온 데이터
	// Image: 프레임을 이미지 객체로 변환하거나 이미지로 저장할 때 불림
		std::cout << "Block ID: " << frameID
			<< "\tSize: " << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
			<< "\tFirst byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer()))
			<< std::endl;
		// reinterpret_cast : 서로 관련 없는 포인터 타입 간의 변환을 수행하는 연산자
		// dynamic_cast가 아닌 static_cast를 사용한 이유 :
		// dynamic_cast는 상속 관계가 있는 클래스 포인터/참조를 안전하게 변환할 때 사용되며,
		// 여기에서는 단순히 기본 타입 간의 변환(uint8_t* -> uint32_t) 이므로 static_cast를 사용해도 안전
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
		// 이미지 파일 입출력을 위한 filer 객체 생성
		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));
		
		//NOTE: w_str(): wide string(wchar_t*) 포인터로 반환
		//NOTE: c_str(): char* 포인터로 반환
		//NOTE: L: wide string 리터럴을 의미, 각 문자가 2바이트로 표현됨
		std::wcout << std::endl << L"Loading " << srcDir.w_str().c_str() << L"... ";
		pStillImageFiler->Load(pImageBuffer, srcDir);
		
		std::cout << "done." << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Loading image error: " << e.GetDescription() << std::endl;
	}
}

void CameraWorker::SaveConfigFile(std::string dstDir)
{
	try
	{
		GenICam::gcstring filePath = GenICam::gcstring(dstDir.c_str());
		// 카메라 노드 맵 가져오기
		GenApi::CNodeMapPtr pNodeMap(m_pDevice->GetRemoteIStPort()->GetINodeMap());
		// 설정값을 저장하기 위한 FeatureBag 객체 생성
		CIStFeatureBagPtr pFeatureBag(CreateIStFeatureBag());
		// 노드 맵의 모든 설정값을 FeatureBag에 저장
		pFeatureBag->StoreNodeMapToBag(pNodeMap);
		
		// 파일(.cfg)로 저장
		std::wcout << std::endl << L"Saving " << filePath.w_str().c_str() << L"... ";
		pFeatureBag->SaveToFile(filePath);
		std::cout << "done" << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Saving config file error: " << e.GetDescription() << std::endl;
	}
}

void CameraWorker::LoadConfigFile(std::string srcDir)
{
	try
	{
		GenICam::gcstring filePath = GenICam::gcstring(srcDir.c_str());
		// 카메라 노드 맵 가져오기
		GenApi::CNodeMapPtr pNodeMap(m_pDevice->GetRemoteIStPort()->GetINodeMap());
		// 설정값을 불러오기 위한 FeatureBag 객체 생성
		CIStFeatureBagPtr pFeatureBag(CreateIStFeatureBag());
		// 파일(.cfg)에서 설정값을 불러와 FeatureBag에 저장
		pFeatureBag->StoreFileToBag(filePath);

		// 노드 맵에 FeatureBag의 설정값을 적용
		std::cout << std::endl << "Loading to the camera ... ";
		pFeatureBag->Load(pNodeMap, true);
		std::cout << "done" << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Loading config file error: " << e.GetDescription() << std::endl;
	}
}

void CameraWorker::SequentialCapture()
{
	while (m_pDataStream->IsGrabbing())
	{
		// 버퍼 포인터를 5000ms의 타임아웃으로 검색
		CIStStreamBufferPtr pStreamBuffer(m_pDataStream->RetrieveBuffer(5000));

		// 획득한 데이터에 이미지 데이터가 있는지 확인
		if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
		{
			// IStImage 객체 생성
			IStImage* pImage = pStreamBuffer->GetIStImage();

			const uint64_t frameID = pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID();
			PrintFrameInfo(pImage, frameID);

			//std::string targetDir = "C:\\Users\\mykir\\Work\\Experiments\\";//NOTE: LAB PC DIRECTORY
			//std::string targetDir = "C:\\Users\\USER\\Pictures\\";//NOTE: HOME PC DIRECTORY
			//ConvertAndSaveImage<BMP>(pImage, true, targetDir, frameID);
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
		// frameID를 문자열로 변환
		std::string strFrameID = std::to_string(frameID);

		// 사용자 지정 경로와 frameID를 결합하여 저장 경로 생성
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
		// 픽셀 포맷 변환을 위한 converter 객체 생성
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
		// 이미지 저장 경로에 확장자 추가 by 템플릿
		dstDir.append(FORMAT::extension);
		
		// 이미지 저장을 위한 filer 객체 생성
		CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));
		
		// 이미지 저장
		std::wcout << L"Saving " << dstDir.w_str().c_str() << L"... ";
		pStillImageFiler->Save(pImageBuffer->GetIStImage(), FORMAT::fileFormat, dstDir);
		std::cout << "done" << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Saving image error: " << e.GetDescription() << std::endl;
	}
}


// 사용 예시 (main.cpp에서 호출)
/*
int main()
{
	CameraWorker cameraWorker(10); // 10개의 이미지 획득
	if (cameraWorker.initialize())
	{
		cameraWorker.StartAcquisition();

		// ... 이미지 처리 로직 ...
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}
*/