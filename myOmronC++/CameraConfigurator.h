#pragma once
#include "config.h"

#include <StApi_TL.h>
#ifdef ENABLED_ST_GUI
#include <StApi_GUI.h>
#endif // ENABLED_ST_GUI

using namespace StApi;
using namespace GenApi;

/*
@brief 카메라 설정 파일을 저장하고 불러오는 기능을 제공하는 클래스
*/
class CameraConfigurator
{
public:
	static void DisplayNodes(GenApi::CNodePtr pINode);
	/*
	@brief 카메라 설정 파일 저장 함수
	@param dstDir : 설정 파일을 저장할 디렉토리 경로
	*/
	static void Save(const CIStDevicePtr& device, const std::string& dstDir);
	/*
	@brief 카메라 설정 파일 불러오기 함수
	@param srcDir : 설정 파일이 저장된 디렉토리 경로
	*/
	static void Load(const CIStDevicePtr& device, const std::string& srcDir);

	// static을 사용한 이유:
	// 이 클래스는 카메라 설정을 저장하고 불러오는 기능만을 제공하며,
	// 인스턴스를 생성할 필요가 없기 때문입니다.
protected:

private:
};

