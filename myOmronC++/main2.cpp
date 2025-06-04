#include <StApi_TL.h>

using namespace StApi;

using namespace std;

//typedef void* UserParam_t;

// 이미지 스트림에서 새 버퍼가 도착했을 떄 호출되는 콜백 함수
void OnCallback(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext)
{
	// 콜백 파라미터의 타입을 확인
	if (pIStCallbackParamBase->GetCallbackType() == StCallbackType_GenTLEvent_DataStreamNewBuffer)
	{
		// 콜백 파라미터를 IStCallbackParamGenTLEventNewBuffer로 캐스팅
		IStCallbackParamGenTLEventNewBuffer* pIStCallbackParamGenTLEventNewBuffer = dynamic_cast<IStCallbackParamGenTLEventNewBuffer*>(pIStCallbackParamBase);

		try
		{
			// 데이터 스트림 포인터 얻기
			IStDataStream *pIStDataStream = pIStCallbackParamGenTLEventNewBuffer->GetIStDataStream();
			
			// 버퍼를 가져오기 (타임아웃: 0 -> 즉시 반환)
			CIStStreamBufferPtr pIStStreamBuffer(pIStDataStream->RetrieveBuffer(0));
			
			// 버퍼에 이미지가 존재하는지 확인
			if (pIStStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				IStImage *pIStImage = pIStStreamBuffer->GetIStImage();
				
				UNREFERENCED_PARAMETER(pvContext);
				
				// 이미지 정보 출력
				cout << "Block ID: " << pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< "Size:" << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight()
					<< "First byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pIStImage->GetImageBuffer())) << endl;
			}
			else
			{
				cout << "No image present in the buffer." << endl;
			}
		}
		catch (const GenICam::GenericException& e)
		{
			cerr << "Exception:" << e.GetDescription() << endl;
		}
	}
}

// 콜백 함수 등록을 위한 래퍼 함수, __stdcall 호출 규약을 사용하여 정의
void __stdcall OnStCallbackFunction(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext)
{
	OnCallback(pIStCallbackParamBase, pvContext);
}

int main(int /*argc */, char** /* argv */)
{
	try
	{
		// StApi 초기화 객체 (생성자에서 자동으로 Init, 소멸자에서 자동으로 Terminate)
		CStApiAutoInit StApiAutoInit;

		// StApi 시스템 객체 생성
		CIStSystemPtr pIstSystem(CreateIStSystem());

		// 디바이스 연결
		CIStDevicePtr pIstDevice(pIstSystem->CreateFirstIStDevice());

		cout << "Device: " << pIstDevice->GetIStDeviceInfo()->GetDisplayName() << endl;

		// 데이터 스트림 생성
		CIStDataStreamPtr pIstDataStream(pIstDevice->CreateIStDataStream(0));

		// 콜백 함수 등록
		//RegisterCallback(pIstDataStream, &OnStCallbackFunction, (void*)NULL);		// CHECK: (void*)NULL 대신 nullptr 사용가능한지 확인하기
		RegisterCallback(pIstDataStream, &OnStCallbackFunction, nullptr);		// CHECK: (void*)NULL 대신 nullptr 사용가능한지 확인하기

		// 호스트(PC) 측 이미지 획득 시작
		pIstDataStream->StartAcquisition();
		// 카메라 측 이미지 획득 시작
		pIstDevice->AcquisitionStart();

		cout << endl << "Press Enter to stop acquisition..." << endl;
		while (cin.get() != '\n'); // Wait for Enter key press

		// 카메라 측 이미지 획득 중지
		pIstDevice->AcquisitionStop();

		// 호스트(PC) 측 이미지 획득 중지
		pIstDataStream->StopAcquisition();
	}
	catch (const GenICam::GenericException& e)
	{
		cerr << "Exception:" << e.GetDescription() << endl;
	}
}