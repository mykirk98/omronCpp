#include "CameraConfigurator.h"

void CameraConfigurator::DisplayNodes(CNodePtr pINode)
{
	try
	{
#ifdef ENABLED_ST_GUI
		// 노드맵 디스플레이 윈도우 객체 생성
		CIStNodeMapDisplayWndPtr pNodeMapDisplayWnd(CreateIStWnd(StWindowType_NodeMapDisplay));
		// 노드를 노드맵 윈도우에 등록
		pNodeMapDisplayWnd->RegisterINode(pINode, "Root");
		// 윈도우의 위치와 크기 설정
		pNodeMapDisplayWnd->SetPosition(0, 0, 480, 640);
		// 윈도우 디스플레이
		pNodeMapDisplayWnd->Show(NULL, StWindowMode_Modal);
#else
		if (IsImplemented(pINode))
		{
			// 노드의 이름과 인터페이스 타입 출력
			std::cout << GetInterfaceName(pINode) << " : " << pINode->GetName() << std::endl;
			// 노드의 정보 출력
			EInterfaceType eInterfaceType = pINode->GetPrincipalInterfaceType();
			if (eInterfaceType == intfICategory)
			{
				// Category 타입인 경우, 해당 Category에 속한 모든 기능을 출력
				CCategoryPtr pCategory(pINode);
				FeatureList_t features;
				pCategory->GetFeatures(features);
				for (FeatureList_t::iterator itr = features.begin(); itr != features.end(); ++itr)
				{
					DisplayNodes(*itr);
				}
			}
			else if (eInterfaceType == intfIEnumeration)
			{
				// Enumeration 타입인 경우, 해당 Enumeration의 모든 항목을 출력
				CEnumerationPtr pEnumeration(pINode);
				NodeList_t nodeList;
				pEnumeration->GetEntries(nodeList);
				for (NodeList_t::iterator itr = nodeList.begin(); itr != nodeList.end(); ++itr)
				{
					DisplayNodes(*itr);
				}
			}
		}
#endif // ENABLED_ST_GUI
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Displaying node failed: " << e.GetDescription() << std::endl;
	}
}

void CameraConfigurator::Save(const CIStDevicePtr& device, const std::string& dstDir)
{
	try
	{
		GenICam::gcstring filePath = GenICam::gcstring(dstDir.c_str());
		// 카메라 노드 맵 가져오기
		CNodeMapPtr pNodeMap(device->GetRemoteIStPort()->GetINodeMap());
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
		CNodeMapPtr pNodeMap(device->GetRemoteIStPort()->GetINodeMap());
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
