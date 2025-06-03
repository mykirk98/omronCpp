/*!
\file UserSetCtrl.cpp
\brief 
 
 This sample shows how to use UserSet to load/save setting from/into camera ROM.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Load/Save UserSet with FeatureBag

 For more information, please refer to the help document of StApi.

*/

// Include files for using StApi.
#include <StApi_TL.h>

//Namespace for using GenTL.
using namespace GenTL;

//Namespace for using StApi.
using namespace StApi;

//Namespace for using cout
using namespace std;

//Namespace for using GenApi.
using namespace GenApi;


//Feature names
const char * USER_SET_SELECTOR = "UserSetSelector";						//Standard
const char * USER_SET_TARGET = "UserSet1";								//Standard
const char * USER_SET_LOAD = "UserSetLoad";								//Standard
const char * USER_SET_SAVE = "UserSetSave";								//Standard
const char * USER_SET_DEFAULT = "UserSetDefault";						//Standard
const char * USER_SET_DEFAULT_SELECTOR = "UserSetDefaultSelector";		//Standard(Deprecated)
const char * PIXEL_FORMAT = "PixelFormat";								//Standard

//-----------------------------------------------------------------------------
// Execute command of indicated node map.
//-----------------------------------------------------------------------------
void Execute(INodeMap *pINodeMap, const char *szCommandName)
{
	// Get the ICommand interface pointer, and call the Execute method
	CCommandPtr pICommand(pINodeMap->GetNode(szCommandName));
	pICommand->Execute();
}

//-----------------------------------------------------------------------------
// Set the setting of indicated enumeration of the node map.
//-----------------------------------------------------------------------------
void SetEnumeration(INodeMap *pINodeMap, const char *szEnumerationName, const char *szValueName)
{
	// Get the IEnumeration interface pointer.
	CEnumerationPtr pIEnumeration(pINodeMap->GetNode(szEnumerationName));
	
	// Get the IEnumEntry interface pointer for the specified name.
	CEnumEntryPtr pIEnumEntry(pIEnumeration->GetEntryByName(szValueName));

	// Get the integer value corresponding to the set value name using the IEnumEntry interface pointer.
	// Update the settings using the IEnumeration interface pointer.
	pIEnumeration->SetIntValue(pIEnumEntry->GetValue());
}
//-----------------------------------------------------------------------------
// Display current setting of indicated enumeration of the node map.
//-----------------------------------------------------------------------------
void DisplayEnumeration(INodeMap *pINodeMap, const char *szEnumerationName)
{
	// Get the IEnumeration interface pointer.
	CEnumerationPtr pIEnumeration(pINodeMap->GetNode(szEnumerationName));

	// Display the current settings.
	cout << "Current " << szEnumerationName << "=" << pIEnumeration->GetCurrentEntry()->GetSymbolic() << endl;
}


//-----------------------------------------------------------------------------
// List up the contents of current enumeration node with current setting.
//-----------------------------------------------------------------------------
void Enumeration(INodeMap *pINodeMap, const char *szEnumerationName)
{
	// Get the IEnumeration interface pointer.
	CEnumerationPtr pIEnumeration(pINodeMap->GetNode(szEnumerationName));
	if (IsWritable(pIEnumeration))
	{
		// Get the setting list.
		GenApi::NodeList_t sNodeList;
		pIEnumeration->GetEntries(sNodeList);
		for (;;)
		{
			// Display a configurable option
			cout << szEnumerationName << endl;
			for (size_t i = 0; i < sNodeList.size(); i++)
			{
				if (IsAvailable(sNodeList[i]))
				{
					CEnumEntryPtr pIEnumEntry(sNodeList[i]);
					cout << i << " : " << pIEnumEntry->GetSymbolic();
					if (pIEnumeration->GetIntValue() == pIEnumEntry->GetValue())
					{
						cout << "(Current)";
					}
					cout << endl;
				}
			}
			cout << "Select : ";

			// Waiting for input
			size_t nIndex;
			cin >> nIndex;

			// Reflect the value entered.
			if (nIndex < sNodeList.size())
			{
				CEnumEntryPtr pIEnumEntry(sNodeList[nIndex]);
				pIEnumeration->SetIntValue(pIEnumEntry->GetValue());
				break;
			}
		}
	}
}

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
		CIStDevicePtr pIStDevice(pIStSystem->CreateFirstIStDevice(DEVICE_ACCESS_EXCLUSIVE));

		// Displays the DisplayName of the device.
		cout << "Device=" << pIStDevice->GetIStDeviceInfo()->GetDisplayName() << endl;

		// Get the INodeMap interface pointer for the camera settings.
		CNodeMapPtr pINodeMapRemote(pIStDevice->GetRemoteIStPort()->GetINodeMap());

		// Set the UserSet to be used to UserSetSelector.
		SetEnumeration(pINodeMapRemote, USER_SET_SELECTOR, USER_SET_TARGET);

		// Load the UserSet that is stored in the ROM reflected in the camera.
		Execute(pINodeMapRemote, USER_SET_LOAD);

		cout << "Loaded : " << USER_SET_TARGET << endl;

		// Create a FeatureBag object for acquiring/saving camera settings.
		CIStFeatureBagPtr pIStFeatureBagPtr(CreateIStFeatureBag());

		// Save the current settings to FeatureBag.
		pIStFeatureBagPtr->StoreNodeMapToBag(pINodeMapRemote);

		// Set the pixel format.
		Enumeration(pINodeMapRemote, PIXEL_FORMAT);

		// Display the current pixel format setting.
		DisplayEnumeration(pINodeMapRemote, PIXEL_FORMAT);

		// Save current settings to UserSet.
		Execute(pINodeMapRemote, USER_SET_SAVE);
		cout << "Saved : " << USER_SET_TARGET << endl;

		// Setting the pixel format.
		Enumeration(pINodeMapRemote, PIXEL_FORMAT);

		// Display current pixel format setting.
		DisplayEnumeration(pINodeMapRemote, PIXEL_FORMAT);

		// Load the UserSet that are stored in the ROM reflected in the camera.
		Execute(pINodeMapRemote, USER_SET_LOAD);
		cout << "Loaded : " << USER_SET_TARGET << endl;

		// Display current pixel format setting.
		DisplayEnumeration(pINodeMapRemote, PIXEL_FORMAT);

		// Load the settings in the FeatureBag to the camera.
		pIStFeatureBagPtr->Load(pINodeMapRemote);

		// Set the UserSet for UserSetSelector.
		SetEnumeration(pINodeMapRemote, USER_SET_SELECTOR, USER_SET_TARGET);

		// Save the current settings to UserSet.
		Execute(pINodeMapRemote, USER_SET_SAVE);
		cout << "Saved : " << USER_SET_TARGET << endl;

		if (pINodeMapRemote->GetNode(USER_SET_DEFAULT) != NULL)
		{
			// Display the UserSetDefault setting.
			DisplayEnumeration(pINodeMapRemote, USER_SET_DEFAULT);
		}
		else
		{
			// Display the UserSetDefaultSelector setting.
			DisplayEnumeration(pINodeMapRemote, USER_SET_DEFAULT_SELECTOR);
		}

	}
    catch (const GenICam::GenericException &e)
	{
		// Display the description of the error of exception.
		cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
    }

	cin.ignore();
	// Wait until the Enter key is pressed.
	cout << endl << "Press Enter to exit." << endl;
	while(cin.get() != '\n');

	return(0);
}
