#pragma once

#include <StApi_TL.h>

using namespace StApi;

/* 청크 데이터를 사용하여 카메라에서 전송되는 이미지와 함께 추가 메타데이터를 수집하는 클래스입니다.
이 클래스는 카메라의 청크 모드를 활성화하고, 청크 데이터를 구성하며, 수집된 청크 데이터를 표시합니다.
청크 데이터는 이미지 프레임에 부가적으로 첨부되는 메타데이터(예: 노출 시간, 게인, 프레임 번호, 타임스탬프 등)를 포함합니다. */
class CameraWorker_Chunk
{
public:
	/*
	클래스 생성자
	@param imageCount : 수집할 이미지 수
	*/
	explicit CameraWorker_Chunk(uint64_t imageCount = 1000);
	/* 클래스 소멸자 */
	~CameraWorker_Chunk();

	/* 카메라 제어에 필요한 여러 객체 초기화 함수 */
	bool initialize();
	/* 이미지 획득 시작 함수 */
	void startAcquisition();
	/* 이미지 획득 종료 함수 */
	void stopAcquisition();

private:
	/* 카메라에 정의된 모든 chunk 항목을 활성화시키고, chunk값을 읽을 수 있도록 노드 포인터를 저장하는 함수 */
	void configureChunkData();
	/* 활성화된 chunk 노드들의 값을 출력하는 함수 */
	void displayNodeValues();

	uint64_t m_imageCount;
	bool initialized;

	CStApiAutoInit m_stApiAutoInit;

	CIStSystemPtr m_pSystem;
	CIStDevicePtr m_pDevice;
	CIStDataStreamPtr m_pDataStream;

	GenApi::CNodeMapPtr m_pNodeMap;
	std::vector<GenApi::INode*> m_ChunkValueList;

	const char* CHUNK_MODE_ACTIVE = "ChunkModeActive";
	const char* CHUNK_SELECTOR = "ChunkSelector";
	const char* CHUNK_ENABLE = "ChunkEnable";
};

