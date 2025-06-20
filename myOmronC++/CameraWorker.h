#pragma once
#include <StApi_TL.h>	// TL : Transport Layer
#include <StApi_IP.h>	// IP : Image Processing
#include <iostream>
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
	@brief .stapiraw 확장자로 이미지 저장하는 wrapper 함수
	@param pImageBuffer : 저장할 이미지 버퍼 포인터
	@param savePath : 이미지 저장 경로
	*/
	void SaveStApiRawImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& savePath);
	/*
	@brief .bitmap 확장자로 이미지 저장하는 wrapper 함수
	@param pImageBuffer : 저장할 이미지 버퍼 포인터
	@param savePath : 이미지 저장 경로
	*/
	void SaveBMPImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& savePath);
	/*
	@brief .tiff 확장자로 이미지 저장하는 wrapper 함수
	@param pImageBuffer : 저장할 이미지 버퍼 포인터
	@param savePath : 이미지 저장 경로
	*/
	void SaveTiffImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& savePath);
	/*
	@brief .png 확장자로 이미지 저장하는 wrapper 함수
	@param pImageBuffer : 저장할 이미지 버퍼 포인터
	@param savePath : 이미지 저장 경로
	*/
	void SavePNGImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& savePath);
	/*
	@brief .jpg 확장자로 이미지 저장하는 wrapper 함수
	@param pImageBuffer : 저장할 이미지 버퍼 포인터
	@param savePath : 이미지 저장 경로
	*/
	void SaveJPEGImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& savePath);
	/*
	@brief .csv 확장자로 이미지 저장하는 wrapper 함수
	@param pImageBuffer : 저장할 이미지 버퍼 포인터
	@param savePath : 이미지 저장 경로
	*/
	void SaveCSVImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& savePath);

	template<typename FORMAT>
	void SaveImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& savePath);

protected:
	
private:
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
	void LoadImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& filePath);
	
	/*
	@brief 픽셀 포맷 변환 함수
	@param pSrcImage : 변환할 원본 이미지 포인터
	@param dstFormat : 변환할 대상 픽셀 포맷
	@param pDstBuffer : 변환된 이미지를 저장할 이미지 버퍼 포인터
	*/
	void ConvertToBGR8(IStImage* pSrcImage, EStPixelFormatNamingConvention_t dstFormat, CIStImageBufferPtr& pDstBuffer);

	/*
	@brief 이미지 저장 함수
	@param pImageBuffer : 저장할 이미지 버퍼 포인터
	@param savePath : 이미지 저장 경로
	@param fileFormat : 저장할 이미지 파일 포맷
	*/
	void SaveImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& savePath, EStStillImageFileFormat_t fileFormat);
	

	bool m_initialized;
	bool m_isImageSaved;

	uint64_t m_imageCount;	// 획득할 이미지 수
	
	CStApiAutoInit m_StApiAutoInit;	// StApi 초기화 객체
	// StApi 라이브러리의 자동 초기화를 위한 객체로,
	// 프로그램 시작 시 자동으로 StApi 라이브러리를 초기화하고 종료 시 자동으로 정리합니다.
	CIStSystemPtr m_pSystem;	// 시스템 객체
	CIStDevicePtr m_pDevice;	// 디바이스 객체
	CIStDataStreamPtr m_pDataStream;	// 데이터 스트림 객체
};