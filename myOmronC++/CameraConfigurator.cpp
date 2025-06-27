#include "CameraConfigurator.h"

void CameraConfigurator::Save(const CIStDevicePtr& device, const std::string& dstDir)
{
	try
	{
		GenICam::gcstring filePath = GenICam::gcstring(dstDir.c_str());
		// 카메라 노드 맵 가져오기
		GenApi::CNodeMapPtr pNodeMap(device->GetRemoteIStPort()->GetINodeMap());
		// 설정값을 저장하기 위한 FeatureBag 객체 생성
		CIStFeatureBagPtr pFeatureBag(CreateIStFeatureBag());
		// 노드 맵의 모든 설정값을 FeatureBag에 저장
		pFeatureBag->StoreNodeMapToBag(pNodeMap);

		// 파일(.cfg)로 저장
		std::wcout << std::endl << L"Saving " << filePath.w_str().c_str() << L"... ";
		pFeatureBag->SaveToFile(filePath);
		std::cout << "done" << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Saving configuration failed: " << e.GetDescription() << std::endl;
	}
}

void CameraConfigurator::Load(const CIStDevicePtr& device, const std::string& srcDir)
{
	try
	{
		GenICam::gcstring filePath = GenICam::gcstring(srcDir.c_str());
		// 카메라 노드 맵 가져오기
		GenApi::CNodeMapPtr pNodeMap(device->GetRemoteIStPort()->GetINodeMap());
		// 설정값을 불러오기 위한 FeatureBag 객체 생성
		CIStFeatureBagPtr pFeatureBag(CreateIStFeatureBag());
		// 파일(.cfg)에서 설정값을 불러와 FeatureBag에 저장
		pFeatureBag->StoreFileToBag(filePath);

		// 노드맵에 FeatureBag의 설정값 적용
		std::wcout << std::endl << L"Loading " << filePath.w_str().c_str() << L"... ";
		pFeatureBag->Load(pNodeMap);
		std::cout << "done" << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Loading configuration failed: " << e.GetDescription() << std::endl;
	}
}
