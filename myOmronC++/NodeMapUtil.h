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
class NodeMapUtil
{
public:
	/*
	@brief 카메라 설정 노드를 출력하는 함수
	@param pINode : 카메라 설정 노드 포인터
	*/
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

	static void SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName);

	static void SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource);
protected:

private:

};

