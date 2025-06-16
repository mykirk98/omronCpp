#include <StApi_TL.h>
#include <StApi_IP.h>

using namespace StApi;

#include <ShlObj.h>

//using namespace std;

const uint64_t numOfImages = 1;

int main()
{
	// 다국어 출력이 깨지지 않도록 시스템의 로케일을 std::wcout에 설정
	std::wcout.imbue(std::locale("", std::locale::ctype));

	try
	{
		CStApiAutoInit stApiAutoInit;
		CIStSystemPtr stSystem(CreateIStSystem());
		CIStDevicePtr stDevice(stSystem->CreateFirstIStDevice());
		std::cout << "Device: " << stDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;

		CIStDataStreamPtr stDataStream(stDevice->CreateIStDataStream(0));
		
		stDataStream->StartAcquisition(numOfImages);
		stDevice->AcquisitionStart();

		// 이미지 저장 경로 설정
		wchar_t szPath[MAX_PATH];
		SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, 0, szPath);	// C:\Users\사용자\Pictures
		GenICam::gcstring strFileNameHeader(szPath);
		strFileNameHeader.append("\\");
		strFileNameHeader.append(stDevice->GetIStDeviceInfo()->GetDisplayName());	// C:\Users\사용자\Pictures\카메라이름
		
		bool isImagSaved = false;

		GenICam::gcstring strFileNameRaw(strFileNameHeader);
		strFileNameRaw.append(".StApiRaw");

		CIStStreamBufferPtr stStreamBuffer(stDataStream->RetrieveBuffer(5000));

		if (stStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
		{
			IStImage* pStImage = stStreamBuffer->GetIStImage();

			std::cout << "\r BlockId: " << stStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
				<< " Size: " << pStImage->GetImageWidth() << " x " << pStImage->GetImageHeight()
				<< " First byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pStImage->GetImageBuffer()));

			// 정지(still) 이미지를 파일로 처리하는 기능을 담당하는 객체 생성
			CIStStillImageFilerPtr pStStillImageFiler(CreateIStFiler(StFilerType_StillImage));
			//NOTE: L을 안붙이면 다국어가 깨져서 출력됨
			std::wcout << std::endl << L"Saving " << strFileNameRaw.w_str().c_str() << L"... ";		// 저장할 파일명 출력
			//std::wcout << std::endl << "Saving " << strFileNameRaw.w_str().c_str() << "... ";		// 저장할 파일명 출력
			pStStillImageFiler->Save(pStImage, StStillImageFileFormat_StApiRaw, strFileNameRaw);	// 획득한 이미지를 StApiRaw 포맷으로 저장
			std::cout << "done..." << std::endl;
			isImagSaved = true;
		}
		else
		{
			std::cout << "Image data does not exist" << std::endl;
		}

		stDevice->AcquisitionStop();
		stDataStream->StopAcquisition();

		
		if (isImagSaved)
		{
			// StApiRaw 파일에서 이미지를 읽어올 버퍼 객체 생성
			CIStImageBufferPtr stImageBuffer(CreateIStImageBuffer());

			// 이미지 파일 입출력을 위한 filer 객체 생성
			CIStStillImageFilerPtr pStStillImageFiler(CreateIStFiler(StFilerType_StillImage));

			// StApiRaw 파일을 버퍼로 로드
			std::wcout << std::endl << L"Loading " << strFileNameRaw.w_str().c_str() << L"... ";
			// NOTE: w_str() : wide string(wchar_t*)포인터로 반환
			// NOTE: c_str() : char* 포인터로 반환
			// NOTE: L : wide string 리터럴을 의미, 각 문자가 2바이트로 표현됨
			pStStillImageFiler->Load(stImageBuffer, strFileNameRaw);
			std::cout << "done" << std::endl;

			// 픽셀 포맷 변환을 위한 converter 객체 생성
			CIStPixelFormatConverterPtr pStPixelFormatConverter(CreateIStConverter(StConverterType_PixelFormat));

			// BGR8 포맷으로 변환
			pStPixelFormatConverter->SetDestinationPixelFormat(StPFNC_BGR8);
			pStPixelFormatConverter->Convert(stImageBuffer->GetIStImage(), stImageBuffer);

			// 변환된 이미지 데이터에 접근
			IStImage* pConvertedImage = stImageBuffer->GetIStImage();

			// bitmap 파일로 저장
			GenICam::gcstring strImageFileName(strFileNameHeader);
			strImageFileName.append(".bmp");

			std::wcout << std::endl << L"Saving " << strImageFileName.w_str().c_str() << L"... ";
			pStStillImageFiler->Save(pConvertedImage, StStillImageFileFormat_Bitmap, strImageFileName);
			std::cout << "done" << std::endl;
		}
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Exception: " << e.GetDescription() << std::endl;
	}
	while (std::cin.get() != '\n');
}