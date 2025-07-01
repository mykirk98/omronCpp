#include "CameraConfigurator.h"

void CameraConfigurator::DisplayNodes(CNodePtr pINode)
{
	try
	{
#ifdef ENABLED_ST_GUI
		// ���� ���÷��� ������ ��ü ����
		CIStNodeMapDisplayWndPtr pNodeMapDisplayWnd(CreateIStWnd(StWindowType_NodeMapDisplay));
		// ��带 ���� �����쿡 ���
		pNodeMapDisplayWnd->RegisterINode(pINode, "Root");
		// �������� ��ġ�� ũ�� ����
		pNodeMapDisplayWnd->SetPosition(0, 0, 480, 640);
		// ������ ���÷���
		pNodeMapDisplayWnd->Show(NULL, StWindowMode_Modal);
#else
		if (IsImplemented(pINode))
		{
			// ����� �̸��� �������̽� Ÿ�� ���
			std::cout << GetInterfaceName(pINode) << " : " << pINode->GetName() << std::endl;
			// ����� ���� ���
			EInterfaceType eInterfaceType = pINode->GetPrincipalInterfaceType();
			if (eInterfaceType == intfICategory)
			{
				// Category Ÿ���� ���, �ش� Category�� ���� ��� ����� ���
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
				// Enumeration Ÿ���� ���, �ش� Enumeration�� ��� �׸��� ���
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
		// ī�޶� ��� �� ��������
		CNodeMapPtr pNodeMap(device->GetRemoteIStPort()->GetINodeMap());
		// �������� �����ϱ� ���� FeatureBag ��ü ����
		CIStFeatureBagPtr pFeatureBag(CreateIStFeatureBag());
		// ��� ���� ��� �������� FeatureBag�� ����
		pFeatureBag->StoreNodeMapToBag(pNodeMap);

		// ����(.cfg)�� ����
		std::wcout << std::endl << L"Saving " << filePath.c_str() << L"... ";
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
		// ī�޶� ��� �� ��������
		CNodeMapPtr pNodeMap(device->GetRemoteIStPort()->GetINodeMap());
		// �������� �ҷ����� ���� FeatureBag ��ü ����
		CIStFeatureBagPtr pFeatureBag(CreateIStFeatureBag());
		// ����(.cfg)���� �������� �ҷ��� FeatureBag�� ����
		pFeatureBag->StoreFileToBag(filePath);

		// ���ʿ� FeatureBag�� ������ ����
		std::wcout << std::endl << L"Loading " << filePath.c_str() << L"... ";
		pFeatureBag->Load(pNodeMap);
		std::cout << "done" << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Loading configuration failed: " << e.GetDescription() << std::endl;
	}
}
