#include "CameraConfigurator.h"

void CameraConfigurator::DisplayNodes(CNodePtr pINode)
{
	try
	{
#ifdef ENABLED_ST_GUI
		// Create a NodeMap display window object.
		CIStNodeMapDisplayWndPtr pNodeMapDisplayWnd(CreateIStWnd(StWindowType_NodeMapDisplay));
		// Register the node to NodeMap window.
		pNodeMapDisplayWnd->RegisterINode(pINode, "Root");
		// Set the position and size of the window.
		pNodeMapDisplayWnd->SetPosition(0, 0, 480, 640);
		// Display the window.
		pNodeMapDisplayWnd->Show(NULL, StWindowMode_Modal);
#else
		if (IsImplemented(pINode))
		{
			// Display the name and interface type.
			std::cout << GetInterfaceName(pINode) << " : " << pINode->GetName() << std::endl;
			// Get the interface type.
			EInterfaceType eInterfaceType = pINode->GetPrincipalInterfaceType();
			if (eInterfaceType == intfICategory)
			{
				// In the case of Category type, display all of the features that belong to the category.
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
				// In the case of Enumeration type, display all of the entries.
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
		// Use INodeMap object to access current settings of the camera.
		CNodeMapPtr pNodeMap(device->GetRemoteIStPort()->GetINodeMap());
		// Create a FeatureBag object for acquiring/saving camera settings.
		CIStFeatureBagPtr pFeatureBag(CreateIStFeatureBag());
		// Acquire and save all current settings from INodeMap object to FeatureBag.
		pFeatureBag->StoreNodeMapToBag(pNodeMap);

		// Display all settings.
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
		// Use INodeMap object to access current settings of the camera.
		CNodeMapPtr pNodeMap(device->GetRemoteIStPort()->GetINodeMap());
		// Create a FeatureBag object for loading settings from file.
		CIStFeatureBagPtr pFeatureBag(CreateIStFeatureBag());
		// Load the settings from file to the FeatureBag.
		pFeatureBag->StoreFileToBag(filePath);

		// Load the settings from the FeatureBag to the camera
		std::wcout << std::endl << L"Loading " << filePath.c_str() << L"... ";
		pFeatureBag->Load(pNodeMap);
		std::cout << "done" << std::endl;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Loading configuration failed: " << e.GetDescription() << std::endl;
	}
}
