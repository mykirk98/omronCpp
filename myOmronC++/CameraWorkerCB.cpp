#include "CameraWorkerCB.h"

CameraWorkerCB::CameraWorkerCB()
	: pICommandTriggerSoftware(nullptr)
{
}

CameraWorkerCB::~CameraWorkerCB()
{
	stopAcquisition();
}

bool CameraWorkerCB::initialize()
{
	try
	{
		// 시스템 객체 생성 (장치 검색 및 연결)
		m_pSystem = CreateIStSystem();
		
		// 첫 번쨰 장치 생성 및 연결
		m_pDevice = m_pSystem->CreateFirstIStDevice();

		// 장치 정보 출력
		std::cout << "Device: " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;

		// 카메라 세팅을 위한 노드맵 가져오기
		GenApi::CNodeMapPtr pINodeMap(m_pDevice->GetRemoteIStPort()->GetINodeMap());
		// 트리거모드 설정
		SetTriggerMode(pINodeMap, "FrameStart", "On", "Software");
		pICommandTriggerSoftware = pINodeMap->GetNode("TriggerSoftware");
		
		// 이미지 스트림 데이터를 처리하기 위한 데이터스트림 객체 생성
		m_pDataStream = m_pDevice->CreateIStDataStream(0);

		// 데이터 스트림 콜백 설정 (this 포인터를 pvContext로 전달)
		RegisterCallback(m_pDataStream, &CameraWorkerCB::OnStCallbackMethod, this);
		//RegisterCallback(m_pDataStream, &CameraWorker_CB::OnStCallbackFunction, nullptr);	// nullptr을 넘길 경우, 콜백 함수에서 this 포인터를 사용할 수 없음
		// NOTE: this를 넘기는 목적 : 콜백이 발생했을 때, 어떤 객체의 멤버 함수로 처리를 위임할지 알려주기 위함

		m_initialized = true;
		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Initialization error: " << e.GetDescription() << std::endl;
		return false;
	}
}

void CameraWorkerCB::startAcquisition()
{
	if (m_initialized == false)
	{
		std::cerr << "Camera not initialized. Call initialize() first." << std::endl;
		return;
	}
	else
	{
		try
		{
			// 호스트(PC) 측 이미지 획득 시작
			m_pDataStream->StartAcquisition();
			
			// 카메라 측 이미지 획득 시작
			m_pDevice->AcquisitionStart();

			/*while (true)
			{
				std::cout << "0: Generate trigger" << std::endl;
				std::cout << "Else: Exit" << std::endl;
				std::cout << "Select: ";

				size_t nindex;
				std::cin >> nindex;
				
				if (nindex == 0)
				{
					pICommandTriggerSoftware->Execute();
				}
				else
				{
					break;
				}
			}*/
		}
		catch (const GenICam::GenericException& e)
		{
			std::cerr << "Start acquisition error: " << e.GetDescription() << std::endl;
		}
	}
}

void CameraWorkerCB::stopAcquisition()
{
	if (m_initialized == false)
	{
		return;
	}
	else
	{
		try
		{
			// 카메라 측 이미지 획득 중지
			m_pDevice->AcquisitionStop();

			// 호스트(PC) 측 이미지 획득 중지
			m_pDataStream->StopAcquisition();
		}
		catch (const GenICam::GenericException& e)
		{
			std::cerr << "Stop acquisition error: " << e.GetDescription() << std::endl;
		}
	}
}

void __stdcall CameraWorkerCB::OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext)
{
	if (pvContext)
	{
		// pvContext로 넘긴 this 포인터를 다시 캐스팅하여 멤버 함수 호출
		static_cast<CameraWorkerCB*>(pvContext)->OnCallback(pIStCallbackParamBase);
		// static_cast : C++에서 형 변환을 할 때 사용하는 연산자, 컴파일 타임에 변환
		// <> : 템플릿을 사용하여 타입을 지정
	}
}

// 멤버 콜백 처리 함수
void CameraWorkerCB::OnCallback(IStCallbackParamBase* pCallbackParam)
{
	try
	{
		// 콜백 파라미터의 타입 확인
		if (pCallbackParam->GetCallbackType() == StCallbackType_GenTLEvent_DataStreamNewBuffer)
		{
			IStCallbackParamGenTLEventNewBuffer* pNewBufferParam = dynamic_cast<IStCallbackParamGenTLEventNewBuffer*>(pCallbackParam);
			// NOTE: dynamic_cast를 사용한 이유 : 다형성을 활용하여 IStCallbackParamBase에서 파생된 
			//									IStCallbackParamGenTLEventNewBuffer 타입으로 안전하게 다운캐스팅하기 위함
			// NOTE: static_cast와의 차이점 : dynamic_cast는 런타임에 타입 체크를 수행하여 실패할 경우 nullptr을 반환
			
			IStDataStream* pDataStream = pNewBufferParam->GetIStDataStream();
			
			CIStStreamBufferPtr pStreamBuffer(pDataStream->RetrieveBuffer(0));
			
			if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				IStImage* pImage = pStreamBuffer->GetIStImage();
				
				PrintFrameInfo(pImage, pStreamBuffer);

				CIStImageBufferPtr pImageBuffer(CreateIStImageBuffer());
				ConvertPixelFormat(pImage, true, pImageBuffer);
				//GenICam::gcstring savePath = SetSavePath(GenICam::gcstring(std::to_string(pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()).c_str()));
				GenICam::gcstring savePath = SetSavePath("C:\\Users\\mykir\\Work\\Experiments\\", pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID());

				SaveImage<BMP>(pImageBuffer, savePath);
			}
			else
			{
				std::cout << "No image present in the buffer." << std::endl;
			}
		}
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Callback Exception: " << e.GetDescription() << std::endl;
	}
}

void CameraWorkerCB::SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName)
{
	try
	{
		// IEnumeration 인터페이스 포인터 가져오기
		GenApi::CEnumerationPtr pIEnumeration(pInodeMap->GetNode(szEnumerationName));

		// 지정된 이름의 IEnumEntry 인터페이스 포인터 가져오기
		GenApi::CEnumEntryPtr pIEnumEntry(pIEnumeration->GetEntryByName(szValueName));

		// IEnumEntry 인터페이스 포인터를 사용하여 정수 값 가져오기
		// IEnumeration 인터페이스 포인터를 사용하여 설정 업데이트
		pIEnumeration->SetIntValue(pIEnumEntry->GetValue());
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Setting enumeration failed: " << e.GetDescription() << std::endl;
	}
}

void CameraWorkerCB::SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource)
{
	try
	{
		// TriggerSelector 노드 설정
		SetEnumeration(pINodeMap, "TriggerSelector", triggerSelector);
		// TriggerMode 노드 설정
		SetEnumeration(pINodeMap, "TriggerMode", triggerMode);
		// TriggerSource 노드 설정
		SetEnumeration(pINodeMap, "TriggerSource", triggerSource);
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Setting trigger mode failed: " << e.GetDescription() << std::endl;
	}
}

// 사용 예시 (main.cpp에서 호출)
/*
int main()
{
	CameraWorkerCB cameraWorker;
	if (cameraWorker.initialize())
	{
		cameraWorker.startAcquisition();

		while (true)
		{
			std::cout << "0: Generate trigger" << std::endl;
			std::cout << "Else: Exit" << std::endl;
			std::cout << "Select: ";

			size_t nindex;
			std::cin >> nindex;
			if (nindex == 0)
			{
				cameraWorker.pICommandTriggerSoftware->Execute();
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}
*/