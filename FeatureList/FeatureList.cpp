/*!
\file FeatureList.cpp
\brief 
 
 This sample will list all support functions of connected camera.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Access Nodes of NodeMap for displaying camera's features

 For more information, please refer to the help document of StApi.

*/
// If you want to use the GUI features, please remove the comment.
#define ENABLED_ST_GUI

// Include files for using StApi.
#include <StApi_TL.h>
#ifdef ENABLED_ST_GUI
#include <StApi_GUI.h>
#include <iomanip>	//std::setprecision
#endif

//Namespace for using StApi.
using namespace StApi;

//Namespace for using GenApi.
using namespace GenApi;

//Namespace for using cout
using namespace std;


#ifdef ENABLED_ST_GUI
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DisplayNodes(CNodePtr pINode)
{
	// Create an NodeMap display window object.
	CIStNodeMapDisplayWndPtr pIStNodeMapDisplayWnd(CreateIStWnd(StWindowType_NodeMapDisplay));

	// Register the node to NodeMap window.
	pIStNodeMapDisplayWnd->RegisterINode(pINode, "Root");

	// Set the position and size of the window.
	pIStNodeMapDisplayWnd->SetPosition(0, 0, 480, 640);

	// Display the window.
	pIStNodeMapDisplayWnd->Show(NULL, StWindowMode_Modal);
}
#else

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DisplayNodes(CNodePtr pINode)
{
	if (IsImplemented(pINode))
	{
		// Display the names and interface type.
		cout << GetInterfaceName(pINode) << " : " << pINode->GetName() << endl;

		// Get the interface type.
		EInterfaceType eInterfaceType = pINode->GetPrincipalInterfaceType();
		if (eInterfaceType == intfICategory)
		{
			// In the case of Category type, display all of the features that belong to the category.
			GenApi::CCategoryPtr pICategory(pINode);
			FeatureList_t features;
			pICategory->GetFeatures(features);
			for (FeatureList_t::iterator itr = features.begin(); itr != features.end(); ++itr)
			{
				DisplayNodes(*itr);
			}
		}
		else if (eInterfaceType == intfIEnumeration)
		{
			// In the case of Enumeration type, display all of the entries.
			GenApi::CEnumerationPtr pIEnumeration(pINode);
			NodeList_t nodeList;
			pIEnumeration->GetEntries(nodeList);
			for (NodeList_t::iterator itr = nodeList.begin(); itr != nodeList.end(); ++itr)
			{
				DisplayNodes(*itr);
			}
		}
	}
}
#endif

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int main(int /* argc */, char ** /* argv */)
{
	try
	{
		// Initialize StApi before using.
		CStApiAutoInit objStApiAutoInit;

		// Create a system object for device scan and connection.
		CIStSystemPtr pIStSystem(CreateIStSystem());

		// Create a camera device object and connect to first detected device.
		CIStDevicePtr pIStDevice(pIStSystem->CreateFirstIStDevice());

		// Displays the DisplayName of the device.
		cout << "Device=" << pIStDevice->GetIStDeviceInfo()->GetDisplayName() << endl;

		// Display nodes.
		DisplayNodes(pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("Root"));
	}
	catch (const GenICam::GenericException &e)
	{
		// Display a description of the error.
		cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
	}

	// Wait until the Enter key is pressed.
	cout << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');

	return(0);
}
