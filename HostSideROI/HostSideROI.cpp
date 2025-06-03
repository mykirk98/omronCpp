/*!
\file HostSideROI.cpp
\brief

 This sample shows how to divide image data into multiple ROI images in local side and display them.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Acquire image data (with waiting in main thread)
 - Process image ROI in host side (local computer)
 
 For more information, please refer to the help document of StApi.

*/


// Include files for using StApi
#include <StApi_TL.h>
#include <StApi_GUI.h>
#include <iomanip>	//std::setprecision

// Namespace for using StApi.
using namespace StApi;

// Namespace for using cout
using namespace std;

// Namespace for using GenApi.
using namespace GenApi;

// Count of images to be grabbed.
const uint64_t nCountOfImagesToGrab = 2000;

// Count of regions of each direction.
const size_t nHorizontalRoiCount = 4;
const size_t nVerticalRoiCount = 2;


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

		// Create DisplayImageWindow (For display whole Image)
		CIStImageDisplayWndPtr pIStImageDisplayWnd;

		// Create DisplyaImageWindows (For display ROI Images)
		CIStImageDisplayWndPtrArray pIStWndList;

		// Create a DataStream object for handling image stream data..
		CIStDataStreamPtr pIStDataStream(pIStDevice->CreateIStDataStream(0));

		// Create INodeMap object to access current setting of the camera.
		CNodeMapPtr pINodeMapRemote(pIStDevice->GetRemoteIStPort()->GetINodeMap());

		// Get current setting of the image size.
		CIntegerPtr pIIntegerWidth(pINodeMapRemote->GetNode("Width"));
		CIntegerPtr pIIntegerHeight(pINodeMapRemote->GetNode("Height"));
		const int32_t pnImageSize[] = {
			(int32_t)pIIntegerWidth->GetValue(),
			(int32_t)pIIntegerHeight->GetValue()
		};

		// Get current pixel format information.
		EStPixelFormatNamingConvention_t nPFNC = (EStPixelFormatNamingConvention_t)dynamic_cast<IEnumeration*>(pINodeMapRemote->GetNode("PixelFormat"))->GetIntValue();
		const IStPixelFormatInfo *pIStPixelFormatInfo = GetIStPixelFormatInfo(nPFNC);

		// Get the minimum setting unit of both sides (X and Y).
		const size_t pnPixelIncrement[] = { pIStPixelFormatInfo->GetPixelIncrementX(), pIStPixelFormatInfo->GetPixelIncrementY() };

		// Calculate the size of the ROI.
		const size_t pnROIWindowCount[] = { nHorizontalRoiCount, nVerticalRoiCount };
		int32_t pnROIImageSize[2];
		for(size_t i = 0; i < 2; i++)
		{
			int32_t nSize = static_cast<int32_t>(pnImageSize[i] / pnROIWindowCount[i]);
			nSize -= static_cast<int32_t>(nSize % pnPixelIncrement[i]);
			pnROIImageSize[i] = nSize;
		}

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
				if (!pIStImageDisplayWnd.IsValid())
				{
					// Create image display window object for displaying whole image here.
					pIStImageDisplayWnd = CreateIStWnd(StWindowType_ImageDisplay);

					// Set the position and size of the window.
					pIStImageDisplayWnd->SetPosition(0, 0, static_cast<int32_t>(pIStImage->GetImageWidth()), static_cast<int32_t>(pIStImage->GetImageHeight()));
				}

				// Acquire detail information of received image and display it onto the status bar of the display window.
				stringstream ss;
				ss << pIStDevice->GetIStDeviceInfo()->GetDisplayName();
				ss << "  ";
				ss << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight();
				ss << "  ";
				ss << fixed << std::setprecision(2) << pIStDataStream->GetCurrentFPS();
				ss << "[fps]";
				GenICam::gcstring strText(ss.str().c_str());
				pIStImageDisplayWnd->SetUserStatusBarText(strText);

				// Check if the image display window is visible.
				if (!pIStImageDisplayWnd->IsVisible())
				{
					// Create a new thread to display the window.
					pIStImageDisplayWnd->Show(NULL, StWindowMode_ModalessOnNewThread);
				}

				// Register the image to be displayed.
				// This will have a copy of the image data and original buffer can be released if necessary and original buffer can be released if necessary.
				pIStImageDisplayWnd->RegisterIStImage(pIStImage);

				// Display each ROI image in different window.
				size_t nIndex = 0;
				for (size_t y = 0; y < pnROIWindowCount[1]; y++)
				{
					for (size_t x = 0; x < pnROIWindowCount[0]; x++)
					{
						// Get the ROI image with IStImage object.
						IStImage *pIStImageROI = pIStImage->GetROIImage(x * pnROIImageSize[0], y * pnROIImageSize[1], pnROIImageSize[0], pnROIImageSize[1]);


						IStImageDisplayWnd *pIStImageDisplayWndROI = NULL;
						if ((pIStWndList.GetSize() <= nIndex) || (pIStWndList[nIndex] == NULL))
						{
							// Create an image display window object, to get the IStWndReleasable interface pointer.
							pIStWndList.Register(CreateIStWnd(StWindowType_ImageDisplay));
							pIStImageDisplayWndROI = pIStWndList[nIndex];

							// Set the position and size of the window.
							pIStImageDisplayWndROI->SetPosition(static_cast<int32_t>(x * pnROIImageSize[0]), static_cast<int32_t>(y * pnROIImageSize[1]), pnROIImageSize[0], pnROIImageSize[1]);
						}
						else
						{
							pIStImageDisplayWndROI = pIStWndList[nIndex];
						}

						// Check if the image display window is visible.
						if (!pIStImageDisplayWndROI->IsVisible())
						{
							// Create a new thread to display the window.
							pIStImageDisplayWndROI->Show(NULL, StWindowMode_ModalessOnNewThread);
						}

						// Register the image to be displayed.
						// This will have a copy of the image data and original buffer can be released if necessary and original buffer can be released if necessary.
						pIStImageDisplayWndROI->RegisterIStImage(pIStImageROI);

						++nIndex;
					}
				}
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
