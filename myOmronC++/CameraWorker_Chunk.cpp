#include "CameraWorker_Chunk.h"

CameraWorker_Chunk::CameraWorker_Chunk(uint64_t imageCount)
	: m_imageCount(imageCount)
	, initialized(false)
{

}

CameraWorker_Chunk::~CameraWorker_Chunk()
{
	stopAcquisition();
}

bool CameraWorker_Chunk::initialize()
{
	try
	{
		// 시스템 객체 생성
		m_pSystem = CreateIStSystem();

		// 카메라 객체 생성
		m_pDevice = m_pSystem->CreateFirstIStDevice();
		std::cout << "Device: " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;

		// 데이터 스트림 생성
		m_pDataStream = m_pDevice->CreateIStDataStream(0);
		
		// 카메라의 노드 맵 가져오기
		m_pNodeMap = m_pDevice->GetRemoteIStPort()->GetINodeMap();

		configureChunkData();

		initialized = true;

		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Initialzation error: " << e.GetDescription() << std::endl;
		return false;
	}
}

void CameraWorker_Chunk::startAcquisition()
{
	if (!initialized)
	{
		std::cerr << "Camera is not initialized. Call initialize() first." << std::endl;
		return;
	}
	try
	{
		// 호스트(PC) 측 이미지 획득 시작
		m_pDataStream->StartAcquisition(m_imageCount);
		// 카메라 측 이미지 획득 시작
		m_pDevice->AcquisitionStart();

		while (m_pDataStream->IsGrabbing())
		{
			// 데이터 스트림에서 버퍼 가져오기
			CIStStreamBufferPtr pBuffer(m_pDataStream->RetrieveBuffer(5000));

			if (pBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				IStImage* pImage = pBuffer->GetIStImage();
				
				std::cout << "Block ID: " << pBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size: " << pImage->GetImageWidth() << " x " << pImage->GetImageHeight()
					<< " First Byte: " << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pImage->GetImageBuffer())) << std::endl;

				displayNodeValues();
			}
			else
			{
				std::cout << "No iamge present in the buffer." << std::endl;
			}
		}
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Start acquisition error: " << e.GetDescription() << std::endl;
	}
}

void CameraWorker_Chunk::stopAcquisition()
{
	try
	{
		// 카메라 측 이미지 획득 종료
		m_pDevice->AcquisitionStop();
		// 호스트(PC) 측 이미지 획득 종료
		m_pDataStream->StopAcquisition();

		initialized = false;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Stop acquisition error: " << e.GetDescription() << std::endl;
	}
}

void CameraWorker_Chunk::configureChunkData()
{
	// Chunk Mode 활성화, 카메라가 chunk 데이터를 포함하여 전송하도록 설정
	GenApi::CBooleanPtr pChunkModeActive(m_pNodeMap->GetNode(CHUNK_MODE_ACTIVE));
	pChunkModeActive->SetValue(true);

	// ChunkSelector 노드: 어떤 chunk 항목을 설정할지 선택하는 노드
	GenApi::CEnumerationPtr pChunkSelector(m_pNodeMap->GetNode(CHUNK_SELECTOR));

	// ChunkEnable 노드: 선택된 Chunk 항목을 활성화/비활성화 하는 노드
	GenApi::CBooleanPtr pChunkEnable(m_pNodeMap->GetNode(CHUNK_ENABLE));

	// 카메라가 지원하는 chunk 항목들을 리스트로 가져옴
	GenApi::NodeList_t chunkList;
	pChunkSelector->GetEntries(chunkList);

	// 각 chunk 항목에 대해 활성화 처리
	for (GenApi::NodeList_t::iterator itr = chunkList.begin(); itr != chunkList.end(); ++itr)
	{
		GenApi::CEnumEntryPtr pEnumEntry(*itr);

		// 사용 가능한 chunk 항목인지 확인
		if (GenApi::IsAvailable(pEnumEntry))
		{
			// 현재 chunk 항목을 선택하고 활성화
			pChunkSelector->SetIntValue(pEnumEntry->GetValue());
			if (GenApi::IsWritable(pChunkEnable))
			{
				pChunkEnable->SetValue(true);

				// 해당 chunk 항목의 값을 가져와서 리스트에 추가
				GenApi::CNodePtr pChunkValueNode(m_pNodeMap->GetNode("Chunk" + pEnumEntry->GetSymbolic()));
				if (pChunkValueNode)
				{
					m_ChunkValueList.push_back(pChunkValueNode);
				}
			}
		}
	}
}

void CameraWorker_Chunk::displayNodeValues()
{
	// 저장된 chunk 노드 리스트 순회
	for (auto* node : m_ChunkValueList)
	{
		std::stringstream ss;
		ss << node->GetName();

		// 노드 값을 읽기 위한 CValuePtr로 캐스팅
		GenApi::CValuePtr pValue(node);

		// 노드가 읽을 수 있는 상태인지 확인
		if (!GenApi::IsReadable(pValue))
		{
			ss << " is not readable.";
		}
		else
		{
			ss << "=" << pValue->ToString();
		}
		ss << std::endl;

		std::cout << ss.str();
	}
}


/* 사용 예시 (main.cpp)
#include "CameraWorker_Chunk.h"

int main()
{
	std::cout << "Camera Worker Class with Chunk Data" << std::endl;

	CameraWorker_Chunk camera(100);
	if (camera.initialize())
	{
		camera.startAcquisition();
	}
	return 0;
}
*/