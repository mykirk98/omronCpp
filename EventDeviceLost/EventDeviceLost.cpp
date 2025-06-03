/*!
\file EventDeviceLost.cpp
\brief 

 This sample shows how to setup and detect device connection lost.
 The following points will be demonstrated in this sample code:
 - Initialize StApi 
 - Connect to camera
 - Detect the disconnection of camera 

 For more information, please refer to the help document of StApi.

*/

// Include files for using StApi.
#include <StApi_TL.h>

//Namespace for using StApi.
using namespace StApi;

//Namespace for using cout
using namespace std;

//Namespace for using GenApi
using namespace GenApi;

//Count of images to be grabbed.
const uint64_t nCountOfImagesToGrab = GENTL_INFINITE;

//#define ENABLED_CLASS_METHOD_TYPE_CALLBACK
typedef IStDevice*	UserParam_t;

//Feature names
const char * EVENT_SELECTOR = "EventSelector";				//Standard
const char * EVENT_NOTIFICATION = "EventNotification";			//Standard
const char * EVENT_NOTIFICATION_ON = "On";						//Standard
const char * TARGET_EVENT_NAME = "DeviceLost";				//Standard
const char * CALLBACK_NODE_NAME = "EventDeviceLost";	//Standard

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OnNodeCallback(GenApi::INode *pINode, UserParam_t pIStDevice)
{
	if(GenApi::IsAvailable(pINode))
	{
		// Node event will be triggered when it is invalidated. 
        // Check if DeviceLost occurred.
		if (pIStDevice->IsDeviceLost())
		{
			cout << "OnNodeEvent:" << pINode->GetDisplayName() << " : DeviceLost" << endl;
		}
		else
		{
			cout << "OnNodeEvent:" << pINode->GetDisplayName() << " : Invalidated" << endl;
		}
	}
}
#ifdef ENABLED_CLASS_METHOD_TYPE_CALLBACK
//-----------------------------------------------------------------------------
// Class method callback function
//-----------------------------------------------------------------------------
class CStNodeCallback
{
public:
	void OnNodeCallbackClassMethod(INode* pINode, UserParam_t pParam)
	{
		OnNodeCallback(pINode, pParam);
	};
protected:
};
#else

//-----------------------------------------------------------------------------
// Flat function callback
//-----------------------------------------------------------------------------
void __stdcall OnNodeCallbackCFunction(INode* pINode, UserParam_t pParam)
{
	OnNodeCallback(pINode, pParam);
};
#endif //ENABLED_CLASS_METHOD_TYPE_CALLBACK
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GrabLoop(IStDevice * pIStDevice)
{

	// Display the DisplayName of the device.
	cout << "Device=" << pIStDevice->GetIStDeviceInfo()->GetDisplayName() << endl;

	// Get the INodeMap interface pointer for the host side device settings
	GenApi::CNodeMapPtr pINodeMapLocal(pIStDevice->GetLocalIStPort()->GetINodeMap());

	// Get the INode interface pointer for the EventDeviceLost node
	GenApi::CNodePtr pNodeCallback(pINodeMapLocal->GetNode(CALLBACK_NODE_NAME));
	if (!pNodeCallback.IsValid())
	{
		throw ACCESS_EXCEPTION("%s node does not exist.", CALLBACK_NODE_NAME);
	}
	else
	{
#ifdef ENABLED_CLASS_METHOD_TYPE_CALLBACK
		// Register a callback function. When a DataStream event is triggered, the registered function will be called.
		CStNodeCallback objCStNodeCallback;
		//CIStRegisteredCallbackPtr pIStRegisteredCallbackNodeEvent(RegisterCallback(pNodeCallback, objCStNodeCallback, &CStNodeCallback::OnNodeCallbackClassMethod, pIStDevice, cbPostOutsideLock));
		RegisterCallback(pNodeCallback, objCStNodeCallback, &CStNodeCallback::OnNodeCallbackClassMethod, pIStDevice, cbPostOutsideLock);
#else
		// Register a callback function. When an event occurs for Data strem, function registered will be called.
		//CIStRegisteredCallbackPtr pIStRegisteredCallbackNodeEvent(RegisterCallback(pNodeCallback, &OnNodeCallbackCFunction, pIStDevice, cbPostOutsideLock));
		RegisterCallback(pNodeCallback, &OnNodeCallbackCFunction, pIStDevice, cbPostOutsideLock);
#endif
		// Enabling the transmission of the target event.
		CEnumerationPtr pEnumerationEventSelector(pINodeMapLocal->GetNode(EVENT_SELECTOR));
		CEnumEntryPtr pEnumEntryEventSelectorEntry(pEnumerationEventSelector->GetEntryByName(TARGET_EVENT_NAME));
		pEnumerationEventSelector->SetIntValue(pEnumEntryEventSelectorEntry->GetValue());

		CEnumerationPtr pEnumerationEventNotification(pINodeMapLocal->GetNode(EVENT_NOTIFICATION));
		CEnumEntryPtr pEnumEntryEventNotificationEntry(pEnumerationEventNotification->GetEntryByName(EVENT_NOTIFICATION_ON));
		pEnumerationEventNotification->SetIntValue(pEnumEntryEventNotificationEntry->GetValue());

		// Start the event acquisition thread for listening to event.
		pIStDevice->StartEventAcquisitionThread();

		// Create a DataStream object for handling image stream data.
		CIStDataStreamPtr pIStDataStream(pIStDevice->CreateIStDataStream(0));

		// Start the image acquisition of the host side.
		pIStDataStream->StartAcquisition(nCountOfImagesToGrab);

		// Start the image acquisition of the camera side.
		pIStDevice->AcquisitionStart();

		// A while loop for acquiring data and checking status. 
		// Here, the acquisition runs until it reaches the assigned numbers of frames.
		while (pIStDataStream->IsGrabbing())
		{
			// Retrieve the buffer pointer of image data with a timeout of 5000ms.
			CIStStreamBufferPtr pIStStreamBuffer(pIStDataStream->RetrieveBuffer(5000));

			// Check if the acquired data contains image data.
			if (pIStStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				// If yes, get the IStImage interface pointer to the acquired image data for further operation.
				IStImage *pIStImage = pIStStreamBuffer->GetIStImage();

				// Display the information of the acquired image data.
				cout << "BlockId=" << pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size:" << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight()
					<< " First byte =" << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pIStImage->GetImageBuffer())) << endl;
			}
			else
			{
				// If the acquired data contains no image data.
				cout << "Image data does not exist" << endl;
			}

#ifdef _DEBUG
			//Invalidate node for checking a dummy device lost event.
			pNodeCallback->InvalidateNode();
#endif
		}

		// Stop the image acquisition of the camera side.
		pIStDevice->AcquisitionStop();

		// Stop the image acquisition of the host side.
		pIStDataStream->StopAcquisition();

		// Stop the event acquisition thread.
		pIStDevice->StopEventAcquisitionThread();
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

		GenICam::gcstring strDeviceId;

		for (;;)
		{
			CIStDevicePtr pIStDevice;
	
			if (strDeviceId.empty())
			{
				// Create a device object of the first occurrence of the device and get the IStDeviceReleasable interface pointer.
				pIStDevice.Reset(pIStSystem->CreateFirstIStDevice());

				// Hold the device ID for re-open
				strDeviceId = pIStDevice->GetIStDeviceInfo()->GetID();
			}
			else
			{
				// Get the number of interfaces.
				const size_t iInterfaceCount = pIStSystem->GetInterfaceCount();


				for (size_t iInterface = 0; iInterface < iInterfaceCount; ++iInterface)
				{
					// Get the IStInterface interface pointer.
					IStInterface *pIStInterface(pIStSystem->GetIStInterface(iInterface));
					try
					{
						// When the camera of the specified ID exists, get the IStDeviceRelesable interface pointer
						pIStDevice.Reset(pIStInterface->CreateIStDevice(strDeviceId));
						break;
					}
					catch (...)
					{
					}

				}
			}

			if (pIStDevice)
			{
				try
				{
					// Repeat the acquisition of the image until the device is disconnected.
					GrabLoop(pIStDevice);
				}
				catch (const GenICam::GenericException &)
				{
					if (!pIStDevice->IsDeviceLost())
					{
						throw;
					}
				}
			}

			//Display choices
			cout << "0 : Reopen the same Device." << endl;
            cout << "    Warning: for GigEVision device, you may need to set the camera IP to persistent before running" << endl;
            cout << "             this example." << endl;
			cout << "Else : Exit" << endl;
			cout << "Selection : ";


			//Waiting for input
			int iInput;
			cin >> iInput;
			if (iInput != 0) break;
		}
	}
    catch (const GenICam::GenericException &e)
    {
		// Display a description of the error.
		cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
    }

	// Wait until the Enter key is pressed.
	cin.ignore();
	cout << endl << "Press Enter to exit." << endl;
	while(cin.get() != '\n');

	return(0);
}
