/*!
\file FrameStartTrigger.cpp
\brief 

 This sample shows how to use trigger mode of the camera
 The The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Set trigger mode and send trigger

 For more information, please refer to the help document of StApi.

*/

// If you want to use the GUI features, please remove the comment.
#define ENABLED_ST_GUI

// If you want to use a class method as a callback function, please remove the comment.
//#define ENABLED_CLASS_METHOD_TYPE_CALLBACK

// Include files for using StApi.
#include <StApi_TL.h>
#ifdef ENABLED_ST_GUI
#include <StApi_GUI.h>
#endif

//Namespace for using StApi.
using namespace StApi;

//Namespace for using cout
using namespace std;

//Namespace for using GenApi.
using namespace GenApi;

#ifdef ENABLED_ST_GUI
typedef IStImageDisplayWnd* UserParam_t;
#else
typedef void*	UserParam_t;
#endif

//Feature names
const char * TRIGGER_SELECTOR = "TriggerSelector";				//Standard
const char * TRIGGER_SELECTOR_FRAME_START = "FrameStart";		//Standard
const char * TRIGGER_SELECTOR_EXPOSURE_START = "ExposureStart";	//Standard
const char * TRIGGER_MODE = "TriggerMode";						//Standard
const char * TRIGGER_MODE_ON = "On";							//Standard
const char * TRIGGER_SOURCE = "TriggerSource";					//Standard
const char * TRIGGER_SOURCE_SOFTWARE = "Software";				//Standard
const char * TRIGGER_SOFTWARE = "TriggerSoftware";				//Standard

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OnCallback(IStCallbackParamBase *pIStCallbackParamBase, UserParam_t pvContext)
{
	if(pIStCallbackParamBase->GetCallbackType() == StCallbackType_GenTLEvent_DataStreamNewBuffer)
	{
		//Case of receiving a NewBuffer events

		IStCallbackParamGenTLEventNewBuffer *pIStCallbackParamGenTLEventNewBuffer = dynamic_cast<IStCallbackParamGenTLEventNewBuffer*>(pIStCallbackParamBase);
		
		try
		{
			// Get the IStDataStream interface pointer
			IStDataStream *pIStDataStream = pIStCallbackParamGenTLEventNewBuffer->GetIStDataStream();

			// Wait until the data is acquired.
			// If the data has been received, IStStreamBufferReleasable interface pointer is retrieved.
			// When the received data is no longer needed, immediately call the IStStreamBufferReleasable::Release(), please return the buffer to the streaming queue.
			// (In the destructor of CIStStreamBufferPtr, IStStreamBufferReleasable::Release() is automatically called.)
			CIStStreamBufferPtr pIStStreamBuffer(pIStDataStream->RetrieveBuffer(0));

			if (pIStStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				// If the image data is included in the acquired data.

				// Get the IStImage interface pointer to the acquired image data.
				IStImage *pIStImage = pIStStreamBuffer->GetIStImage();

#ifdef ENABLED_ST_GUI
				// Check if display window is visible.
				if (!pvContext->IsVisible())
				{
					// Sets the position and size of the window.
					pvContext->SetPosition(0, 0, static_cast<int32_t>(pIStImage->GetImageWidth()), static_cast<int32_t>(pIStImage->GetImageHeight()));

					// Create a new thread to display the window.
					pvContext->Show(NULL, StWindowMode_ModalessOnNewThread);
				}

				// Register the image to be displayed.
				// This will have a copy of the image data and original buffer can be released if necessary and original buffer can be released if necessary.
				pvContext->RegisterIStImage(pIStImage);
#else
				// Display the information of the acquired image data.
				cout << "BlockId=" << pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size:" << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight()
					<< " First byte =" << (uint32_t)*(uint8_t*)pIStImage->GetImageBuffer() << endl;
#endif
			}
			else
			{
				// If the acquired data contains no image data
				cout << "Image data does not exist" << endl;
			}
		}
		catch (const GenICam::GenericException &e)
		{
			// Display a description of the error.
			cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
		}
	}
}

#ifdef ENABLED_CLASS_METHOD_TYPE_CALLBACK
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CCallback
{
public:
	CCallback(){};
	~CCallback(){};


	void OnStCallbackClassMethod(IStCallbackParamBase *pIStCallbackParamBase, UserParam_t pvContext)
	{
		OnCallback(pIStCallbackParamBase, pvContext);
	};
};
#else
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void __stdcall OnStCallbackCFunction(IStCallbackParamBase *pIStCallbackParamBase, UserParam_t pvContext)
{
	OnCallback(pIStCallbackParamBase, pvContext);
}
#endif

//-----------------------------------------------------------------------------
//
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

		// Get the INodeMap interface pointer for the camera settings.
		CNodeMapPtr pINodeMapRemote(pIStDevice->GetRemoteIStPort()->GetINodeMap());

		// Set the TriggerSelector to FrameStart.
		try
		{
			SetEnumeration(pINodeMapRemote, TRIGGER_SELECTOR, TRIGGER_SELECTOR_FRAME_START);
		}
		catch (const  GenICam::GenericException &)
		{
			//If "FrameStart" is not supported, use "ExposureStart".
			SetEnumeration(pINodeMapRemote, TRIGGER_SELECTOR, TRIGGER_SELECTOR_EXPOSURE_START);
		}

		// Set the TriggerMode to On.
		SetEnumeration(pINodeMapRemote, TRIGGER_MODE, TRIGGER_MODE_ON);

		// Set the TriggerSource to Software.
		SetEnumeration(pINodeMapRemote, TRIGGER_SOURCE, TRIGGER_SOURCE_SOFTWARE);

		// Get the ICommand interface pointer for the TriggerSoftware node.
		CCommandPtr pICommandTriggerSoftware(pINodeMapRemote->GetNode(TRIGGER_SOFTWARE));

#ifdef ENABLED_ST_GUI
		// If using GUI for display, create a display window here.
		CIStImageDisplayWndPtr pIStImageDisplayWnd(CreateIStWnd(StWindowType_ImageDisplay));
#endif

		// Create a DataStream object for handling image stream data.
		CIStDataStreamPtr pIStDataStream(pIStDevice->CreateIStDataStream(0));

		// Register a callback function. When a Data stream event is triggered, the registered function will be called.
#ifdef ENABLED_ST_GUI
#ifdef ENABLED_CLASS_METHOD_TYPE_CALLBACK
		CCallback objCCallback;
		RegisterCallback(pIStDataStream, objCCallback, &CCallback::OnStCallbackClassMethod, (UserParam_t)pIStImageDisplayWnd);
#else
		RegisterCallback(pIStDataStream, &OnStCallbackCFunction, static_cast<UserParam_t>(pIStImageDisplayWnd));
#endif
#else
#ifdef ENABLED_CLASS_METHOD_TYPE_CALLBACK
		CCallback objCCallback;
		RegisterCallback(pIStDataStream, objCCallback, &CCallback::OnStCallbackClassMethod, (UserParam_t)NULL);
#else
		RegisterCallback(pIStDataStream, &OnStCallbackCFunction, (UserParam_t)NULL);
#endif
#endif

		// Start the image acquisition of the host side.
		pIStDataStream->StartAcquisition();

		// Start the image acquisition of the camera side.
		pIStDevice->AcquisitionStart();

#ifdef ENABLED_ST_GUI
		// Create a node map display window object, to get the IStWndReleasable interface pointer.
		// After the window object is no longer needed, call the IStWndReleasable::Release() to discard the window object.
		// (In the destructor of CIStNodeMapDisplayWndPtr, IStWndReleasable::Release() is called.)
		CIStNodeMapDisplayWndPtr pIStNodeMapDisplayWnd(CreateIStWnd(StWindowType_NodeMapDisplay));

		pIStNodeMapDisplayWnd->RegisterINode(pICommandTriggerSoftware->GetNode(), "Root");
		pIStNodeMapDisplayWnd->Show(NULL, StWindowMode_Modal);
#else	
		for (;;)
		{
			cout << "0 : Generate trigger" << endl;
			cout << "Else : Exit" << endl;
			cout << "Select : ";

			// Waiting for input
			size_t nIndex;
			cin >> nIndex;

			if (nIndex == 0)
			{
				pICommandTriggerSoftware->Execute();
			}
			else
			{
				break;
			}
		}
#endif

		// Stop the image acquisition of the camera side.
		pIStDevice->AcquisitionStop();

		// Stop the image acquisition of the host side.
		pIStDataStream->StopAcquisition();
	}
    catch (const GenICam::GenericException &e)
	{
		// Display a description of the error.
		cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
	}

	return(0);
}
