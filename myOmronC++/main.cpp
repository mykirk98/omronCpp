#include <StApi_TL.h>		// TL : Transport Layer


// StApi namespace 선언
using namespace StApi;


const uint64_t nCountOfImagesToGrab = 100;	// 이미지 획득 수

int main(int /* args */, char** /* argv */)
{
	try
	{
		// StApi 초기화
		CStApiAutoInit objStApiAutoInit;
		
		// 시스템 객체 생성 (장치 검색 및 연결)
		CIStSystemPtr pIstsystem(CreateIStSystem());

		// 첫 번째 장치 생성 및 연결
		CIStDevicePtr pIstDevice(pIstsystem->CreateFirstIStDevice());

		// 장치 정보 출력
		std::cout << "Device=" << pIstDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;

		// 이미지 스트림 데이터를 처리하기 위한 데이터스트림 객체 생성
		CIStDataStreamPtr pIstDataStream(pIstDevice->CreateIStDataStream(0));	// 0의 의미 : 이 장치의 첫 번째 데이터 스트림을 생성

		// 호스트(PC) 측의 이미지 획득 시작
		pIstDataStream->StartAcquisition(nCountOfImagesToGrab);

		// 카메라 측의 이미지 획득 시작
		pIstDevice->AcquisitionStart();

		// 이미지 획득 및 상태 확인을 위한 루프, 반복문은 지정된 횟수의 프레임에 도달할 때까지 실행
		while (pIstDataStream->IsGrabbing())
		{
			// 버퍼 포인터를 5000ms의 타임아웃으로 검색
			CIStStreamBufferPtr pIstStreamBuffer(pIstDataStream->RetrieveBuffer(5000));

			// 획득한 데이터에 이미지 데이터가 있는지 확인
			if (pIstStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				// 이미지 데이터가 있는 경우 IStImage 객체 생성
				IStImage* pIStImage = pIstStreamBuffer->GetIStImage();

				// 이미지 정보 출력
				std::cout << "BlockId=" << pIstStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< "Size:" << pIStImage->GetImageWidth()
					<< " x "
					<< pIStImage->GetImageHeight()
					<< "First byte=" << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pIStImage->GetImageBuffer()))
					<< std::endl;
					// pIStImage->GetImageBuffer()는 일반적으로 이미지 데이터의 시작 주소를 반환하는 함수이며,
					// 반환 타입이 void*이다.
			}		// 하지만 실제로는 이 버퍼에 픽셀 데이터(바이트 배열)가 들어 있으므로,
					// 이를 바이트 단위로 접근하려면 uint8_t*로 변환해야 함
			else
			{
				// 이미지 데이터가 없는 경우
				std::cout << "No image data present in the buffer." << std::endl;
			}
		}

		// 카메라 측 이미지 획득 중지
		pIstDevice->AcquisitionStop();

		// 호스트(PC) 측 이미지 획득 중지
		pIstDataStream->StopAcquisition();
	}
	catch (const GenICam::GenericException& e)
	{
		// 예외 발생 시 에러 메시지 출력
		std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
	}
	// 프로그램 종료 전 잠시 대기
	std::cout << std::endl << "Press Enter to exit..." << std::endl;
	while (std::cin.get() != '\n');

	return(0);
}