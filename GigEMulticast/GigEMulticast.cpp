/*!
\file GigEMulticast.cpp
\brief

 This sample shows how to use the multicast function of GigE camera for multiple receivers.
 The monitor clients must connect after any one client connect to camera in control mode.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Acquire image data (with waiting in main thread)
 - Connect to camera in control mode / monitor mode
 - Multicast the image data 
 - Broadcast the image data 

 Note: If the firewall is enabled, you may not be able to get the image data.
 For more information, please refer to the help document of StApi.

*/


// If you want to use the GUI features, please remove the comment.
//#define ENABLED_ST_GUI
// Include files for using StApi
#include <StApi_TL.h>
#ifdef ENABLED_ST_GUI
#include <StApi_GUI.h>
#include <iomanip>	//std::setprecision
#endif

// Namespace for using GenTL.
using namespace GenTL;

// Namespace for using StApi.
using namespace StApi;

// Namespace for using cout
using namespace std;

// Counts of images going to grab.
const uint64_t nCountOfImagesToGrab = 100;

// Feature names
const char * DESTINATION_IP_ADDRESS	= "DestinationIPAddress";	//Custom
const char * DESTINATION_PORT = "DestinationPort";	//Custom
const char * TRANSMISSION_TYPE = "TransmissionType";	//Custom
const char * TRANSMISSION_TYPE_USE_CAMERA_CONFIGURATION = "UseCameraConfiguration";	//Custom

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int main(int /* argc */, char ** /* argv */)
{
	try
	{
		// Initialize StApi before using.
		CStApiAutoInit objStApiAutoInit;

		// Select the connecting mode (control/monitor) of the camera.
		// You can connect to a camera in monitor mode if it has already connected by other host with control mode.
		// Note if you connect to a camera in monitor mode, you cannot modify the camera settings.
		bool isMonitor = false;
		for (;;)
		{
			cout << "C : Control mode" << endl;
			cout << "M : Monitor mode" << endl;
			cout << "Select a mode : ";
			char nKey;
			cin >> nKey;

			if ((nKey == 'C') || (nKey == 'c') || (nKey == 'M') || (nKey == 'm'))
			{
				isMonitor = (nKey == 'M') || (nKey == 'm');
				break;
			}
		}

		// Create a system object for device scan and connection.
		CIStSystemPtr pIStSystem(CreateIStSystem(StSystemVendor_Default, StInterfaceType_GigEVision));

		// Create a camera device object and connect to first detected device.
		CIStDevicePtr pIStDevice(pIStSystem->CreateFirstIStDevice(isMonitor ? DEVICE_ACCESS_READONLY : DEVICE_ACCESS_CONTROL));

		// Displays the DisplayName of the device.
		cout << "Device=" << pIStDevice->GetIStDeviceInfo()->GetDisplayName() << endl;

#ifdef ENABLED_ST_GUI
		// Create an image display window object for display.
		CIStImageDisplayWndPtr pIStImageDisplayWnd(CreateIStWnd(StWindowType_ImageDisplay));
#endif

		// Create a DataStream object for handling image stream data.
		CIStDataStreamPtr pIStDataStream(pIStDevice->CreateIStDataStream(0));

		// Get the IEnumeration interface pointer of TRANSMISSION_TYPE node.
		GenApi::CEnumerationPtr pIEnumerationTransmissionType(pIStDataStream->GetIStPort()->GetINodeMap()->GetNode(TRANSMISSION_TYPE));
		int64_t nTransmissionType = 0;

		// Get the setting obeying to the connection type.
		if (isMonitor)
		{
			// Get the setting that represents the transmission type to accept the current camera settings.
			GenApi::CEnumEntryPtr pIEnumEntryUseCameraConfiguration(pIEnumerationTransmissionType->GetEntryByName(TRANSMISSION_TYPE_USE_CAMERA_CONFIGURATION));
			nTransmissionType = pIEnumEntryUseCameraConfiguration->GetValue();
		}
		else
		{
			// Get a list of the transmission type
			GenApi::NodeList_t sTransmissionTypeNodeList;
			pIEnumerationTransmissionType->GetEntries(sTransmissionTypeNodeList);
			for (;;)
			{
				// Display a list of the transmission type.
				cout << "Supported transmission types is as follows." << endl;
				for (size_t i = 0; i < sTransmissionTypeNodeList.size(); i++)
				{
					GenApi::CEnumEntryPtr pIEnumEntryTransmissionType(sTransmissionTypeNodeList[i]);
					cout << i << " : " << pIEnumEntryTransmissionType->GetSymbolic() << endl;
				}
				cout << "Select a transmission type : ";

				// Waiting for input.
				size_t nIndex;
				cin >> nIndex;

				if (nIndex < sTransmissionTypeNodeList.size())
				{
					// Get the setting of the selected transmission type.
					GenApi::CEnumEntryPtr pIEnumEntryTransmissionType(sTransmissionTypeNodeList[nIndex]);
					nTransmissionType = pIEnumEntryTransmissionType->GetValue();
					break;
				}
			}
		}

		// Configure the selected transmission type.
		pIEnumerationTransmissionType->SetIntValue(nTransmissionType);

		// Get the destination IP address of the image data.
		GenApi::CIntegerPtr pIIntegerDestinationIPAddress(pIStDataStream->GetIStPort()->GetINodeMap()->GetNode(DESTINATION_IP_ADDRESS));

		// Display the destination IP address of the image data.
		cout << "Destination IP Address=" << pIIntegerDestinationIPAddress->ToString() << endl;

		// Start the image acquisition of the host side.
		pIStDataStream->StartAcquisition(nCountOfImagesToGrab);

		if (!isMonitor)
		{
			// If in control mode, start the image acquisition of the camera side.
			pIStDevice->AcquisitionStart();
		}

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

		if (!isMonitor)
		{
			// Stop the image acquisition of the camera side.
			pIStDevice->AcquisitionStop();
		}

		// Stop the image acquisition of the host side.
		pIStDataStream->StopAcquisition();
	}
    catch (const GenICam::GenericException &e)
	{
		// Display a description of the error.
		cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
    }

	// Wait until the Enter key is pressed.
	cin.ignore();
	cout << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');

	return(0);
}
