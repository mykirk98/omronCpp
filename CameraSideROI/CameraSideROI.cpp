/*!
\file CameraSideROI.cpp
\brief 

 This sample shows how to set ROI in camera side and handle the image data.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Set image ROI parameter 
 - Acquire image data (with waiting in main thread)
 - Process the acquired ROI images 

 For more information, please refer to the help document of StApi.
*/


// If you want to use the GUI features, please remove the comment.
//#define ENABLED_ST_GUI

// Include files for using StApi.
#include <StApi_TL.h>
#ifdef ENABLED_ST_GUI
#include <StApi_GUI.h>
#include <iomanip>	//std::setprecision
#endif 

// Namespace for using StApi.
using namespace StApi;

// Namespace for using cout
using namespace std;

// Namespace for using GenApi.
using namespace GenApi;

// Counts of images to be grabbed.
const uint64_t nCountOfImagesToGrab = 100;

//Feature names
const char * PIXEL_FORMAT = "PixelFormat";			//Standard
const char * REGION_SELECTOR = "RegionSelector";	//Standard
const char * REGION_MODE = "RegionMode";			//Standard
const char * OFFSET_X = "OffsetX";					//Standard
const char * OFFSET_Y = "OffsetY";					//Standard
const char * WIDTH = "Width";						//Standard
const char * HEIGHT = "Height";						//Standard

#ifdef ENABLED_ST_GUI

#else
//-----------------------------------------------------------------------------
// List up the contents of current enumeration node with current setting.
//-----------------------------------------------------------------------------
bool Enumeration(INodeMap *pINodeMap, const char *szEnumerationName)
{
	bool isUpdated = false;

	// Get the IEnumeration interface pointer.
	CEnumerationPtr pIEnumeration = pINodeMap->GetNode(szEnumerationName);
	if (IsWritable(pIEnumeration))
	{
		// Get the setting list
		GenApi::NodeList_t sNodeList;
		pIEnumeration->GetEntries(sNodeList);

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
		cout << "Else : Exit" << endl;
		cout << "Select : ";

		// Waiting for input
		size_t nInput;
		cin >> nInput;

		// Reflect the value entered
		if ((nInput < sNodeList.size()) && IsAvailable(sNodeList[nInput]))
		{
			CEnumEntryPtr pIEnumEntry(sNodeList[nInput]);
			pIEnumeration->SetIntValue(pIEnumEntry->GetValue());
			isUpdated = true;
		}
	
	}
	return(isUpdated);
}

//-----------------------------------------------------------------------------
// List up the numeric value of the current setting that the node indicated.
//-----------------------------------------------------------------------------
template <class VALUE_TYPE, class NODE_TYPE>
void Numeric(INodeMap *pINodeMap, const char *szNodeName)
{
	// Get the IInteger or IFloat interface pointer.
	NODE_TYPE pINumeric(pINodeMap->GetNode(szNodeName));
	if (IsWritable(pINumeric))
	{
		for (;;)
		{
			// Display the feature name, the range, the current value and the incremental value.
			cout << szNodeName;
			cout << " Min=" << pINumeric->GetMin();
			cout << " Max=" << pINumeric->GetMax();
			cout << " Current=" << pINumeric->GetValue();
			EIncMode eIncMode = pINumeric->GetIncMode();
			if (eIncMode == fixedIncrement)
			{
				VALUE_TYPE nValue = pINumeric->GetInc();
				cout << " Inc=" << nValue;
			}
			cout << endl;
			
			cout << "New value : ";

			//Waiting for input
			VALUE_TYPE nValue;
			cin >> nValue;

			// Reflect the value entered
			if ((pINumeric->GetMin() <= nValue) && (nValue <= pINumeric->GetMax()))
			{
				pINumeric->SetValue(nValue);
				break;
			}
		}
	}
}
//-----------------------------------------------------------------------------
// Use Numeric() to check and set the ROI region detail setting.
//-----------------------------------------------------------------------------
void EachRegion(INodeMap *pINodeMap)
{
	// Configure the OffsetX.
	Numeric<int64_t, CIntegerPtr>(pINodeMap, OFFSET_X);

	// Configure the Width.
	Numeric<int64_t, CIntegerPtr>(pINodeMap, WIDTH);

	// Configure the OffsetY.
	Numeric<int64_t, CIntegerPtr>(pINodeMap, OFFSET_Y);

	// Configure the Height.
	Numeric<int64_t, CIntegerPtr>(pINodeMap, HEIGHT);
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CameraSideROI(INodeMap *pINodeMap)
{
	// Get the IEnumeration interface pointer for RegionSelector.
	CEnumerationPtr pIEnumeration(pINodeMap->GetNode(REGION_SELECTOR));
	// Check if target camera (the node represented) supports multi-ROI.
	if (IsWritable(pIEnumeration))
	{
		// Multi ROI.
		for (;;)
		{
			if (Enumeration(pINodeMap, REGION_SELECTOR))
			{
				// Configure the RegionMode.
				Enumeration(pINodeMap, REGION_MODE);

				// Check whether the area is valid.
				CEnumerationPtr pIEnumeration_RM(pINodeMap->GetNode(REGION_MODE));
				if (pIEnumeration_RM->GetCurrentEntry() != pIEnumeration_RM->GetEntryByName("Off"))
				{
					// Configure the position and size of the region.
					EachRegion(pINodeMap);
				}
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		// Single ROI.
		// Configure the position and size of the region.
		EachRegion(pINodeMap);
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

#ifdef ENABLED_ST_GUI
		// If using GUI for display, create a display window here.
		CIStImageDisplayWndPtr pIStImageDisplayWnd(CreateIStWnd(StWindowType_ImageDisplay));
#endif

		// Create a DataStream object for handling image stream data.
		CIStDataStreamPtr pIStDataStream(pIStDevice->CreateIStDataStream(0));

		// Create INodeMap object to access current setting of the camera.
		CNodeMapPtr pINodeMapRemote(pIStDevice->GetRemoteIStPort()->GetINodeMap());

#ifdef ENABLED_ST_GUI
		// Create a node map display window object, to get the IStWndReleasable interface pointer.
		// After the window object is no longer needed, call the IStWndReleasable::Release() to discard the window object.
		// (In the destructor of CIStNodeMapDisplayWndPtr, IStWndReleasable::Release() is automatically called.)
		CIStNodeMapDisplayWndPtr pIStNodeMapDisplayWnd(CreateIStWnd(StWindowType_NodeMapDisplay));

		pIStNodeMapDisplayWnd->RegisterINode(pINodeMapRemote->GetNode(PIXEL_FORMAT), "CameraSideROI");
		pIStNodeMapDisplayWnd->RegisterINode(pINodeMapRemote->GetNode(REGION_SELECTOR), "CameraSideROI");
		pIStNodeMapDisplayWnd->RegisterINode(pINodeMapRemote->GetNode(REGION_MODE), "CameraSideROI");
		pIStNodeMapDisplayWnd->RegisterINode(pINodeMapRemote->GetNode(OFFSET_X), "CameraSideROI");
		pIStNodeMapDisplayWnd->RegisterINode(pINodeMapRemote->GetNode(OFFSET_Y), "CameraSideROI");
		pIStNodeMapDisplayWnd->RegisterINode(pINodeMapRemote->GetNode(WIDTH), "CameraSideROI");
		pIStNodeMapDisplayWnd->RegisterINode(pINodeMapRemote->GetNode(HEIGHT), "CameraSideROI");
		pIStNodeMapDisplayWnd->Show(NULL, StWindowMode_Modal);
#else
		// Check and set PixelFormat.
		Enumeration(pINodeMapRemote, PIXEL_FORMAT);

		// Check and set CameraSideROI.
		CameraSideROI(pINodeMapRemote);

		cin.ignore();
#endif

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

#ifdef ENABLED_ST_GUI
				// Acquire detail information of received image and display it onto the status bar of the display window.
				stringstream ss;
				ss << pIStDevice->GetIStDeviceInfo()->GetDisplayName();
				ss << "  ";
				ss << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight();
				ss << "  ";
				ss << fixed << std::setprecision(2) << pIStDataStream->GetCurrentFPS();
				ss << "[fps]";
				gcstring strText(ss.str().c_str());
				pIStImageDisplayWnd->SetUserStatusBarText(strText);

				// Check if display window is visible.
				if (!pIStImageDisplayWnd->IsVisible())
				{
					// Sets the position and size of the window.
					pIStImageDisplayWnd->SetPosition(0, 0, pIStImage->GetImageWidth(), pIStImage->GetImageHeight());

					// Create a new thread to display the window.
					pIStImageDisplayWnd->Show(NULL, StWindowMode_ModalessOnNewThread);
				}

				// Register the image to be displayed.
				// This will have a copy of the image data and original buffer can be released if necessary and original buffer can be released if necessary.
				pIStImageDisplayWnd->RegisterIStImage(pIStImage);
#else
				// Display the information of the acquired image data.
				cout << "BlockId=" << pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size:" << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight()
					<< " First byte =" << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pIStImage->GetImageBuffer())) << endl;
#endif
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
