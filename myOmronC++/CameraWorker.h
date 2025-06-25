#pragma once
#include <StApi_TL.h>	// TL : Transport Layer
#include <StApi_IP.h>	// IP : Image Processing
#include <ShlObj.h>

using namespace StApi;

struct StApiRaw
{
	static constexpr const char* extension = ".StApiRaw";
	static constexpr EStStillImageFileFormat_t fileFormat = StStillImageFileFormat_StApiRaw;
};
struct BMP
{
	static constexpr const char* extension = ".bmp";
	static constexpr EStStillImageFileFormat_t fileFormat = StStillImageFileFormat_Bitmap;
};
struct TIFF
{
	static constexpr const char* extension = ".tif";
	static constexpr EStStillImageFileFormat_t fileFormat = StStillImageFileFormat_TIFF;
};
struct PNG
{
	static constexpr const char* extension = ".png";
	static constexpr EStStillImageFileFormat_t fileFormat = StStillImageFileFormat_PNG;
};
struct JPEG
{
	static constexpr const char* extension = ".jpg";
	static constexpr EStStillImageFileFormat_t fileFormat = StStillImageFileFormat_JPEG;
};
struct CSV
{
	static constexpr const char* extension = ".csv";
	static constexpr EStStillImageFileFormat_t fileFormat = StStillImageFileFormat_CSV;
};

/*
@brief 기본적인 카메라 작업을 수행하는 클래스.
@brief 단순한 이미지 획득 및 저장 기능을 제공하며, 카메라 초기화, 이미지 획득 시작 및 종료 기능을 포함합니다.
*/
class CameraWorker
{
public:
	/*
	@brief 클래스 생성자
	@param imageCount : 수집할 이미지 수
	*/
	explicit CameraWorker(uint64_t imageCount = 100);
	/* @brief 클래스 소멸자 */
	~CameraWorker();
	
	/* @brief 카메라 제어에 필요한 여러 객체 초기화 함수 */
	bool initialize();
	/* @brief 이미지 획득 시작 함수 */
	void StartAcquisition();
	/* @brief 이미지 획득 종료 함수 */
	void StopAcquisition();

	/*
	@brief 이미지 저장 함수
	@param pImageBuffer : 저장할 이미지 버퍼 포인터
	@param savePath : 이미지 저장 경로
	*/
	template<typename FORMAT>
	void SaveImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& savePath);

protected:
	/*
	@brief 이미지 저장 경로 설정 함수
	@return : 이미지 저장 경로 문자열
	*/
	GenICam::gcstring SetSavePath(GenICam::gcstring frameID);

	/*
	@brief 이미지 로드 함수
	@param pImageBuffer : 로드한 이미지를 저장할 이미지 버퍼 포인터
	@param filePath : 불러올 이미지 파일 경로
	*/
	void LoadSavedImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& filePath);
	
	bool m_initialized;

	/*
	@brief StApi 라이브러리 초기화 객체
	@brief StApi 라이브러리를 초기화하고 종료 시 자동으로 정리합니다.
	*/
	CStApiAutoInit m_StApiAutoInit;
	/*
	@brief 카메라 시스템 객체
	@brief 카메라 시스템은 카메라 디바이스를 관리하고, 디바이스와의 연결을 설정하는 역할을 합니다.
	*/
	CIStSystemPtr m_pSystem;
	/*
	@brief 카메라 디바이스 객체
	@brief 카메라 디바이스는 카메라의 기능을 제어하고, 이미지 데이터를 획득하는 역할을 합니다.
	*/
	CIStDevicePtr m_pDevice;
	/*
	@brief 데이터 스트림 객체
	@brief 카메라에서 획득한 이미지 데이터를 호스트(PC)로 전송하는 역할을 합니다.
	@brief 데이터스트림 : 카메라에서 이미지를 연속적으로 받아오는 통로를 의미(파이프라인과 유사함)
	*/
	CIStDataStreamPtr m_pDataStream;
	
private:
	/*
	@brief 픽셀 포맷 변환 함수
	@param pSrcImage : 변환할 원본 이미지 포인터
	@param dstFormat : 변환할 대상 픽셀 포맷
	@param pDstBuffer : 변환된 이미지를 저장할 이미지 버퍼 포인터
	*/
	void ConvertToBGR8(IStImage* pSrcImage, EStPixelFormatNamingConvention_t dstFormat, CIStImageBufferPtr& pDstBuffer);

	uint64_t m_imageCount;	// 획득할 이미지 수
};