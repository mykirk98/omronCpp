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
 - Decode QR code by OpenCV
 
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
const uint64_t nCountOfImagesToGrab = INFINITE;


// Include files for using OpenCV.
#include <opencv2/opencv.hpp>

// Library files for using OpenCV.
#ifdef _DEBUG
#define CV_LIBEXT "d.lib"
#else
#define CV_LIBEXT ".lib"
#endif
#define CV_LIB_FILE_NAME(MOD) MOD CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION) CV_LIBEXT
#if CV_MAJOR_VERSION < 4
#pragma message( "Notice: This program requires OpenCV version 4.0 or later to build and run." )
#endif

#pragma comment(lib, CV_LIB_FILE_NAME("opencv_world"))

// Namespace for using OpenCV.
using namespace cv;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool ConvertIStImageToMat(const IStImage *pIStImage, Mat &mat)
{
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
		
		Mat matTmp;
		Mat *pmatInput = pIStPixelFormatInfo->IsMono() ? &mat : &matTmp;
		
		// Create a OpenCV buffer for the input image.
		if ((pmatInput->cols != nImageWidth) || (pmatInput->rows != nImageHeight) || (pmatInput->type() != nInputType))
		{
			pmatInput->create(nImageHeight, nImageWidth, nInputType);
		}

		// Copy the input image data to the buffer for OpenCV.
		const size_t dwBufferSize = pmatInput->rows * pmatInput->cols * pmatInput->elemSize() * pmatInput->channels();
		memcpy(pmatInput->ptr(0), pIStImage->GetImageBuffer(), dwBufferSize);

		// Convert the pixelformat if needed.
		if (pIStPixelFormatInfo->IsBayer())
		{
			Mat *pmatOutput = &mat;
			int nConvert = 0;
			switch (pIStPixelFormatInfo->GetPixelColorFilter())
			{
			case(StPixelColorFilter_BayerRG): nConvert = COLOR_BayerRG2RGB;	break;
			case(StPixelColorFilter_BayerGR): nConvert = COLOR_BayerGR2RGB;	break;
			case(StPixelColorFilter_BayerGB): nConvert = COLOR_BayerGB2RGB;	break;
			case(StPixelColorFilter_BayerBG): nConvert = COLOR_BayerBG2RGB;	break;
			//case(StPixelColorFilter_BayerRG) : nConvert = COLOR_BayerRGGB2GRAY;	break;
			//case(StPixelColorFilter_BayerGR) : nConvert = COLOR_BayerGRBG2GRAY;	break;
			//case(StPixelColorFilter_BayerGB) : nConvert = COLOR_BayerGBRG2GRAY;	break;
			//case(StPixelColorFilter_BayerBG) : nConvert = COLOR_BayerBGGR2GRAY;	break;
			}
			if (nConvert != 0)
			{
				cvtColor(*pmatInput, *pmatOutput, nConvert);
			}
		}
		return(true);
	}
	else
	{
		cout << "This pixel format is not supported." << endl;

	}
	return(false);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DrawAreaAndText(InputOutputArray matDisplay, const String& text, Point *pPoints)
{
	const Scalar colorLine = Scalar(0, 0, 255);
	const int nThicknessLine = 4;
	const int pnNumPoints[] = { 4 };
	const Point *pcurPoints[] = { pPoints };
	polylines(matDisplay, pcurPoints, pnNumPoints, 1, true, colorLine, nThicknessLine);

	const Scalar colorText = Scalar(0, 0, 255);
	const int nFontFace = 0;
	const double dblFontScale = 0.5;
	const int nThicknessText = 1;
	putText(matDisplay, text, pcurPoints[0][2], nFontFace, dblFontScale, colorText);

	cout << "QR Code=" << text.c_str() << endl;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GenApi::ICommand *GetSoftwareTriggerICommand(IStPort *pIStPort)
{

	// Get the INodeMap interface pointer for the camera settings.
	GenApi::CNodeMapPtr pINodeMap(pIStPort->GetINodeMap());

	// Set the TriggerSelector to FrameStart.
	GenApi::CEnumerationPtr pIEnumeration_TriggerSelector(pINodeMap->GetNode("TriggerSelector"));
	*pIEnumeration_TriggerSelector = "FrameStart";

	// Set the TriggerMode to On.
	GenApi::CEnumerationPtr pIEnumeration_TriggerMode(pINodeMap->GetNode("TriggerMode"));
	*pIEnumeration_TriggerMode = "On";

	// Set the TriggerSource to Software.
	GenApi::CEnumerationPtr pIEnumeration_TriggerSource(pINodeMap->GetNode("TriggerSource"));
	*pIEnumeration_TriggerSource = "Software";

	// Get the ICommand interface pointer for the TriggerSoftware node.
	return(dynamic_cast<GenApi::ICommand*>(pINodeMap->GetNode("TriggerSoftware")));
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

		// Create a DataStream object for handling image stream data.
		CIStDataStreamPtr pIStDataStream(pIStDevice->CreateIStDataStream(0));

		GenApi::CCommandPtr pICommand_SoftwareTrigger(GetSoftwareTriggerICommand(pIStDevice->GetRemoteIStPort()));

		// Start the image acquisition of the host side.
		pIStDataStream->StartAcquisition(nCountOfImagesToGrab);

		// Start the image acquisition of the camera side.
		pIStDevice->AcquisitionStart();

		// Image buffers for OpenCV.
		Mat matDisplay;
		QRCodeDetector detector;

		// A while loop for acquiring data and checking status. 
		// Here, the acquisition runs until it reaches the assigned numbers of frames.
		while (pIStDataStream->IsGrabbing())
		{
			// Generate a software trigger.
			pICommand_SoftwareTrigger->Execute();

			// Retrieve the buffer pointer of image data with a timeout of 5000ms.
			CIStStreamBufferPtr pIStStreamBuffer(pIStDataStream->RetrieveBuffer(5000));

			// Check if the acquired data contains image data.
			if (pIStStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				// If yes, we create a IStImage object for further image handling.
				IStImage *pIStImage = pIStStreamBuffer->GetIStImage();

				// Display the information of the acquired image data.
				cout << "BlockId=" << pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID() << " : 0x" << pIStStreamBuffer->GetIStStreamBufferInfo()->GetBaseAddress() << endl;

				// Check the pixelfomat of the input image.
				if(ConvertIStImageToMat(pIStImage, matDisplay))
				{
#if 1
					//Search and decode multiple QR codes.
					vector<cv::String> vecDataStrings;
					vector<cv::Point> points;
					if (detector.detectAndDecodeMulti(matDisplay, vecDataStrings, points))
					{
						for (size_t t = 0; t < vecDataStrings.size(); ++t)
						{
							DrawAreaAndText(matDisplay, vecDataStrings[t], &points[t * 4]);
						}
					}
#else
					//Search and decode single QR code.
					vector<cv::Point> points;
					cv::String strData = detector.detectAndDecode(matDisplay, points);
					if (0 < strData.size())
					{
						DrawAreaAndText(matDisplay, strData, &points[0]);
					}
#endif
					// Show the image.
					imshow("Image1", matDisplay);
				}
			}
			else
			{
				// If the acquired data contains no image data.
				cout << "Image data does not exist" << endl;
			}

			// When you press ESC on the preview window, you exit through the loop.
			const int keyESC = 27;
			if (waitKey(1) == keyESC)
			{
				break;
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
	catch (const cv::Exception &e)
	{
		// Display a description of the error.
		cerr << endl << "An exception occurred." << endl << e.msg.c_str() << endl;
	}

	return(0);
}
