/*!
\file OpenCV.cpp
\brief 
 
 This sample shows how to use StApi with OpenCV for format conversion and display.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Acquire image data (with waiting in main thread)
 - Copy image data for OpenCV
 - Convert Bayer image format to RGB using OpenCV
 - Preview image using OpenCV
 
 Note: Please set the path of include files and libraries of OpenCV.
 For more information, please refer to the help document of StApi and OpenCV.

*/


// Include files for using StApi.
#include <StApi_TL.h>


// Namespace for using StApi.
using namespace StApi;

// Namespace for using cout
using namespace std;

// Count of images to be grabbed.
const uint64_t nCountOfImagesToGrab = 100;


// Include files for using OpenCV.
#include <opencv2/opencv.hpp>

// Library files for using OpenCV.
#ifdef _DEBUG
#define CV_LIBEXT "d.lib"
#else
#define CV_LIBEXT ".lib"
#endif
#define CV_LIB_FILE_NAME(MOD) MOD CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION) CV_LIBEXT
#if CV_MAJOR_VERSION < 3
#pragma comment(lib, CV_LIB_FILE_NAME("opencv_core"))
#pragma comment(lib, CV_LIB_FILE_NAME("opencv_imgproc"))
#pragma comment(lib, CV_LIB_FILE_NAME("opencv_highgui"))
#pragma comment(lib, "zlib" CV_LIBEXT)
#else
#pragma comment(lib, CV_LIB_FILE_NAME("opencv_world"))
#endif


// Namespace for using OpenCV.
using namespace cv;

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

		// Create a DataStream object for handling image stream data.
		CIStDataStreamPtr pIStDataStream(pIStDevice->CreateIStDataStream(0));

		// Start the image acquisition of the host side.
		pIStDataStream->StartAcquisition(nCountOfImagesToGrab);

		// Start the image acquisition of the camera side.
		pIStDevice->AcquisitionStart();

		// Image buffers for OpenCV.
		Mat inputMat;
		Mat displayMat;
		
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

				// Display the information of the acquired image data.
				cout << "BlockId=" << pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size:" << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight()
					<< " First byte =" << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pIStImage->GetImageBuffer())) << endl;

				// Check the pixelfomat of the input image.
				const StApi::EStPixelFormatNamingConvention_t ePFNC = pIStImage->GetImagePixelFormat();
				const StApi::IStPixelFormatInfo * const pIStPixelFormatInfo = StApi::GetIStPixelFormatInfo(ePFNC);
				if (pIStPixelFormatInfo->IsMono() || pIStPixelFormatInfo->IsBayer())
				{
					// Check the size of the image.
					const size_t nImageWidth = pIStImage->GetImageWidth();
					const size_t nImageHeight = pIStImage->GetImageHeight();
					int nInputType = CV_8UC1;
					if (8 < pIStPixelFormatInfo->GetEachComponentTotalBitCount())
					{
						nInputType = CV_16UC1;
					}

					// Create a OpenCV buffer for the input image.
					if ((inputMat.cols != nImageWidth) || (inputMat.rows != nImageHeight) || (inputMat.type() != nInputType))
					{
						inputMat.create(nImageHeight, nImageWidth, nInputType);
					}

					// Copy the input image data to the buffer for OpenCV.
					const size_t dwBufferSize = inputMat.rows * inputMat.cols * inputMat.elemSize() * inputMat.channels();
					memcpy(inputMat.ptr(0), pIStImage->GetImageBuffer(), dwBufferSize);

					// Convert the pixelformat if needed.
					Mat *pMat = &inputMat;
					if (pIStPixelFormatInfo->IsBayer())
					{
						int nConvert = 0;
						switch (pIStPixelFormatInfo->GetPixelColorFilter())
						{
						case(StPixelColorFilter_BayerRG) : nConvert = COLOR_BayerRG2RGB;	break;
						case(StPixelColorFilter_BayerGR) : nConvert = COLOR_BayerGR2RGB;	break;
						case(StPixelColorFilter_BayerGB) : nConvert = COLOR_BayerGB2RGB;	break;
						case(StPixelColorFilter_BayerBG) : nConvert = COLOR_BayerBG2RGB;	break;
						}
						if (nConvert != 0)
						{
							cvtColor(inputMat, displayMat, nConvert);
							pMat = &displayMat;
						}
					}

					// Show the image.
					imshow("Image1", *pMat);
					waitKey(1);
				}


			}
			else
			{
				// If the acquired data contains no image data.
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
