/*!
\file MultipleSystems.cpp
\brief 

  This sample shows how to use multiple GenTL modules (cti files) for acquiring image data.
  The following points will be demonstrated in this sample code:
  - Initialize StApi
  - Connect to the first detected camera of all system
  - Acquire image data (with waiting in main thread) 
  - Use multiple GenTL module.

  For more information, please refer to the help document of StApi.
  
*/
// If you want to use the GUI features, please remove the comment below.
//#define ENABLED_ST_GUI

// Include files for using StApi.
#include <StApi_TL.h>
#ifdef ENABLED_ST_GUI
#include <iomanip>	//for setprecision
#include <StApi_GUI.h>
#endif

// Namespace for using GenTL.
using namespace GenTL;

// Namespace for using StApi.
using namespace StApi;

// Namespace for using cout
using namespace std;

// Count of images to be grabbed.
const uint64_t nCountOfImagesToGrab = 100;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int main(int /* argc */, char ** /* argv */)
{
	try
	{
		// Initialize StApi before using.
		CStApiAutoInit objStApiAutoInit;

		// Create a system object list for store system object.
		// Then we try to create objects of all available systems.
		CIStSystemPtrArray pIStSystemList;
		for (EStSystemVendor_t eSystemVendor = StSystemVendor_Default; eSystemVendor < StSystemVendor_Count; eSystemVendor = (EStSystemVendor_t)(eSystemVendor + 1))
		{
			try
			{
				// For each available system, try to create object for it then register it into system list for further usage.
				pIStSystemList.Register(CreateIStSystem(eSystemVendor, StInterfaceType_All));
			}
			catch (const GenICam::GenericException &e)
			{
				// Display a description of error if any happens.
				cerr << "An exception occurred." << endl << e.GetDescription() << endl;
			}
		}

		// Create a device object of the first detected device to connect.
		CIStDevicePtr pIStDevice(pIStSystemList.CreateFirstIStDevice(DEVICE_ACCESS_EXCLUSIVE));

		// Displays the DisplayName of the device.
		cout << "Device=" << pIStDevice->GetIStDeviceInfo()->GetDisplayName() << endl;
		
#ifdef ENABLED_ST_GUI
		// If using GUI for display, create a display window here.
		CIStImageDisplayWndPtr pIStImageDisplayWnd(CreateIStWnd(StWindowType_ImageDisplay));
#endif

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
			if (pIStStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				// If the image data is included in the acquired data.
				
				// Get the IStImage interface pointer to the acquired image data.
				IStImage *pIStImage = pIStStreamBuffer->GetIStImage();

#ifdef ENABLED_ST_GUI
				// Create a string to be displayed on the status bar
				stringstream ss;
				ss << pIStDevice->GetIStDeviceInfo()->GetDisplayName();
				ss << "  ";
				ss << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight();
				ss << "  ";
				ss << fixed << std::setprecision(2) << pIStDataStream->GetCurrentFPS();
				ss << "[fps]";
				gcstring strText(ss.str().c_str());
				pIStImageDisplayWnd->SetUserStatusBarText(strText);

				// Display window
				if (!pIStImageDisplayWnd->IsVisible())
				{
					// Sets the position and size of the window.
					pIStImageDisplayWnd->SetPosition(0, 0, pIStImage->GetImageWidth(), pIStImage->GetImageHeight());

					// Create a new thread to display the window.
					pIStImageDisplayWnd->Show(NULL, StWindowMode_ModalessOnNewThread);
				}

				// Register the image to be displayed.
				// Registered image is to be copied, the original data is no longer needed.
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
