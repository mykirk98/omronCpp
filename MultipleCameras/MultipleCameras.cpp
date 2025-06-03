/*!
\file MultipleCameras.cpp
\brief 
 
 This sample shows how to conect and get images from all available cameras.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to all available cameras
 - Acquire image from the list of camera
 You can see how to handle multiple cameras/stream objects in this sample.

 For more information, please refer to the help document of StApi.

*/

#include <iomanip>	//for setprecision
// Include files for using StApi.
#include <StApi_TL.h>

// Namespace for using StApi.
using namespace StApi;

// Namespace for using cout
using namespace std;

// Count of images to be grabbed.
const uint64_t nCountOfImagesToGrab = 10;

int main(int /* argc */, char ** /* argv */)
{
	try
	{
		// Initialize StApi before using.
		CStApiAutoInit objStApiAutoInit;

		// Create a system object for device scan and connection.
		CIStSystemPtr pIStSystem(CreateIStSystem());

		// Create a camera device list object to store all the cameras.
		CIStDevicePtrArray pIStDeviceList;

		// Create a DataStream list object to store all the data stream object related to the cameras.
		CIStDataStreamPtrArray pIStDataStreamList;

		// Here we try to connect to all possible device with a do-while loop.
		for (;;)
		{
			IStDeviceReleasable *pIStDeviceReleasable = NULL;
			
			try
			{
				// Create a camera device object and connect to first detected device.
				pIStDeviceReleasable = pIStSystem->CreateFirstIStDevice();
			}
			catch (...)
			{
				if (pIStDeviceList.GetSize() == 0)
				{
					throw;
				}
				else
				{
					break;
				}
			}

			// Add the camera into device object list for later usage.
			pIStDeviceList.Register(pIStDeviceReleasable);

			// Displays the DisplayName of the device.
			cout << "Device" << pIStDeviceList.GetSize() << "=" << pIStDeviceReleasable->GetIStDeviceInfo()->GetDisplayName() << endl;

			// Create a DataStream object for handling image stream data then add into DataStream list for later usage.
			pIStDataStreamList.Register(pIStDeviceReleasable->CreateIStDataStream(0));
		}

		// Start the image acquisition of the host side.
		pIStDataStreamList.StartAcquisition(nCountOfImagesToGrab);

		// Start the image acquisition of the camera side.
		pIStDeviceList.AcquisitionStart();
	

		// A while loop for acquiring data and checking status. 
		// Here we use DataStream list function to check if any cameras in the list is on grabbing.
		while (pIStDataStreamList.IsGrabbingAny())
		{
			// Retrieve data buffer pointer of image data from any camera with a timeout of 5000ms.
			CIStStreamBufferPtr pIStStreamBuffer(pIStDataStreamList.RetrieveBuffer(5000));

			// Check if the acquired data contains image data.
			if (pIStStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				cout
					<< pIStStreamBuffer->GetIStDataStream()->GetIStDevice()->GetIStDeviceInfo()->GetDisplayName()
					<< " : BlockId=" << dec << pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " " << setprecision(4) << pIStStreamBuffer->GetIStDataStream()->GetCurrentFPS() << "FPS" << endl;
			}
			else
			{
				// If the acquired data contains no image data.
				cout << "Image data does not exist" << endl;
			}
		}

		// Stop the image acquisition of the camera side.
		pIStDeviceList.AcquisitionStop();

		// Stop the image acquisition of the host side.
		pIStDataStreamList.StopAcquisition();
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
