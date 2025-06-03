/*!
\file GrabCameraEvent.cpp
\brief 

 This sample shows how to register an event callback with callback function.
 Here we register the callback function to "ExposureEnd" event (Defined as TARGET_EVENT_NAME) with a callback function to handle this event.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Acquire image data with callback function
 - Enable the event message sending function of camera
 - Register callback function of indicated event 
 
 For more information, please refer to the help document of StApi.

*/

// Include files for using StApi
#include <StApi_TL.h>

//Namespace for using StApi
using namespace StApi;

//Namespace for using cout
using namespace std;

//Namespace for using GenApi
using namespace GenApi;

//Count of images to be grabbed.
const uint64_t nCountOfImagesToGrab = 100;

// If you want to use a class method as a callback function, please remove the comment.
//#define ENABLED_CLASS_METHOD_TYPE_CALLBACK

typedef uint32_t	UserParam_t;

//Feature names
const char * EVENT_SELECTOR = "EventSelector";			//Standard
const char * EVENT_NOTIFICATION = "EventNotification";		//Standard
const char * EVENT_NOTIFICATION_ON = "On";			//Standard
const char * TARGET_EVENT_NAME = "ExposureEnd";			//Standard
const char * CALLBACK_NODE_NAME = "EventExposureEndTimestamp";	//Standard

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OnNodeCallback(INode *pINode, UserParam_t /*pParam*/)
{
	try
	{
		stringstream ss;
		ss << pINode->GetName();
			
		if(IsReadable(pINode))
		{
			CValuePtr pValue(pINode);
			if(pValue)
			{
				ss << " = " << pValue->ToString();
			}
		}
		else
		{
			ss << " is not readable.";
		}
		ss << endl;
		cout << ss.str();
	}
	catch (const GenICam::GenericException &e)
	{
		cerr << "An exception occurred." << endl << e.GetDescription() << endl;
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
// Main thread
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

		// Display the DisplayName of the device.
		cout << "Device=" << pIStDevice->GetIStDeviceInfo()->GetDisplayName() << endl;

		// Create a DataStream object for handling image stream data.
		CIStDataStreamPtr pIStDataStream(pIStDevice->CreateIStDataStream(0));

		// Use INodeMap object to access current setting of the camera.
		CNodeMapPtr pINodeMapRemote(pIStDevice->GetRemoteIStPort()->GetINodeMap());
		
		// Get INode interface pointer of "EventExposureEndTimestamp" for later registering use.
		CNodePtr pNodeCallback(pINodeMapRemote->GetNode(CALLBACK_NODE_NAME));
		if(!pNodeCallback.IsValid())
		{
			throw ACCESS_EXCEPTION("Not found a feature %s.", CALLBACK_NODE_NAME);
		}

#ifdef ENABLED_CLASS_METHOD_TYPE_CALLBACK
		// Register acquired INode interface pointer with callback function for calling.
		// When the event of passed-in INode indicated is triggered, the registered callback class method function will be called.
		CStNodeCallback objCStNodeCallback;
		RegisterCallback(pNodeCallback, objCStNodeCallback, &CStNodeCallback::OnNodeCallbackClassMethod, (UserParam_t)0, cbPostInsideLock);
#else
		// Register acquired INode interface pointer with callback function for calling.
		// When the event of passed-in INode indicated is triggered, the registered callback function will be called.
		RegisterCallback(pNodeCallback, &OnNodeCallbackCFunction, (UserParam_t)0, cbPostInsideLock);
#endif
		// Enabling the transmission of the target event.
		// For enabling the event sending function of camera, you need to select the event by EventSelector and switch EventNotification to ON.
		CEnumerationPtr pEnumerationEevntSelector(pINodeMapRemote->GetNode(EVENT_SELECTOR));
		CEnumEntryPtr pEnumEntryEventSelectorEntry(pEnumerationEevntSelector->GetEntryByName(TARGET_EVENT_NAME));
		pEnumerationEevntSelector->SetIntValue(pEnumEntryEventSelectorEntry->GetValue());
		
		CEnumerationPtr pEnumerationEevntNotification(pINodeMapRemote->GetNode(EVENT_NOTIFICATION));
		CEnumEntryPtr pEnumEntryEventNotificationEntry(pEnumerationEevntNotification->GetEntryByName(EVENT_NOTIFICATION_ON));
		pEnumerationEevntNotification->SetIntValue(pEnumEntryEventNotificationEntry->GetValue());

		// Start event handling thread for listening to the events.
		// You must start event handling thread in order to acquire the events.
		pIStDevice->StartEventAcquisitionThread();

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
				// If yes, we create a IStImage object for further image handling.
				IStImage *pIStImage = pIStStreamBuffer->GetIStImage();

				// Retrieve block ID, image size, and the data of first byte and display them.
				cout << "BlockId=" << pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size:" << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight()
					<< " First byte =" << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pIStImage->GetImageBuffer()))
					<< " Timestamp =" << pIStStreamBuffer->GetIStStreamBufferInfo()->GetTimestamp() << endl;
			}
			else
			{
				// If the acquired data contains no image data
				cout << "Image data does not exist" << endl;
			}
		}

		// Stop the image acquisition of the camera side.
		pIStDevice->AcquisitionStop();

		// Stop the image acquisition of the host side.
		pIStDataStream->StopAcquisition();

		// Stop the event acquisition thread
		pIStDevice->StopEventAcquisitionThread();
	}
    catch (const GenICam::GenericException &e)
	{
		// If any exception occurred, display the description of the error here.
		cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
	}

	// Wait until the Enter key is pressed.
	cout << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');

	return(0);
}
