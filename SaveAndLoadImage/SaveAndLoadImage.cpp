/*!
\file SaveAndLoadImage.cpp
\brief 

 This sample shows how to save a captured image into RAW file of StApi.
 After saving to RAW file, this sample will load the file, convert it to BGR8 image, and save as BMP/TIF/PNG/JPG files.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Acquire 1 image data (with waiting in main thread)
 - Save image to / Load image from file.
 - Apply Pixel format conversion
 - Create image buffer for processing the image

 For more information, please refer to the help document of StApi.

If the user name contains double-byte characters, incorrect characters might be displayed depending on your build or execution environment.
*/

// Include files for using StApi.
#include <StApi_TL.h>
#include <StApi_IP.h>

//Namespace for using StApi.
using namespace StApi;

//Namespace for using cout
using namespace std;

// Count of images to be grabbed. We'll only get 1 image to demo how to save image file.
const uint64_t nCountOfImagesToGrab = 1;

// Include file for SHGetFolderPath
#include <Shlobj.h>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int main(int /* argc */, char ** /* argv */)
{
	std::wcout.imbue(std::locale("", std::locale::ctype));

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

		// Start the image acquisition of the host (local machine) side.
		pIStDataStream->StartAcquisition(nCountOfImagesToGrab);

		// Start the image acquisition of the camera side.
		pIStDevice->AcquisitionStart();

		// Get the path of the image files.
		wchar_t szPath[MAX_PATH];
		SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, 0, szPath);
		GenICam::gcstring strFileNameHeader(szPath);
		strFileNameHeader.append("\\");
		strFileNameHeader.append(pIStDevice->GetIStDeviceInfo()->GetDisplayName());

		bool isImageSaved = false;

		// Get the file name of the image file of the StApiRaw file format
		GenICam::gcstring strFileNameRaw(strFileNameHeader);
		strFileNameRaw.append(".StApiRaw");

		// Retrieve the buffer pointer of image data with a timeout of 5000ms.
		// Note that we don't use a while loop here because we only retrieve one image for saving.
		CIStStreamBufferPtr pIStStreamBuffer(pIStDataStream->RetrieveBuffer(5000));

		// Check if the acquired data contains image data.
		if(pIStStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
		{
			// If yes, we create a IStImage object for further image handling.
			IStImage *pIStImage = pIStStreamBuffer->GetIStImage();

			// Display the information of the acquired image data.
			cout << "\r BlockId=" << pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
				<< " Size:" << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight()
				<< " First byte =" << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pIStImage->GetImageBuffer()));

			// Create a still image file handling class object (filer) for still image processing.
			CIStStillImageFilerPtr pIStStillImageFiler(CreateIStFiler(StFilerType_StillImage));
            
			// Save the image file as StApiRaw file format with using the filer we created.
			std::wcout << std::endl << L"Saving " << strFileNameRaw.w_str().c_str() << L"... ";
			pIStStillImageFiler->Save(pIStImage, StStillImageFileFormat_StApiRaw, strFileNameRaw);
			std::cout << "done" << std::endl;
			isImageSaved = true;
		}
		else
		{
			// If the acquired data contains no image data...
			cout << "Image data does not exist" << endl;
		}

		// Stop the image acquisition of the camera side.
		pIStDevice->AcquisitionStop();

		// Stop the image acquisition of the host side.
		pIStDataStream->StopAcquisition();

		// The following code shows how to load the saved StApiRaw and process it.
		if (isImageSaved)
		{
			// Create a buffer for storing the image data from StApiRaw file.
			CIStImageBufferPtr pIStImageBuffer(CreateIStImageBuffer());

			// Create a still image file handling class object (filer) for still image processing.
			CIStStillImageFilerPtr pIStStillImageFiler(CreateIStFiler(StFilerType_StillImage));

			// Load the image from the StApiRaw file into buffer.
			std::wcout << std::endl << L"Loading " << strFileNameRaw.w_str().c_str() << L"... ";
			pIStStillImageFiler->Load(pIStImageBuffer, strFileNameRaw);
			std::cout << "done" << std::endl;

			// Create a data converter object for pixel format conversion.
			CIStPixelFormatConverterPtr pIStPixelFormatConverter(CreateIStConverter(StConverterType_PixelFormat));

			// Convert the image data to StPFNC_BGR8 format
			pIStPixelFormatConverter->SetDestinationPixelFormat(StPFNC_BGR8);
			pIStPixelFormatConverter->Convert(pIStImageBuffer->GetIStImage(), pIStImageBuffer);

			// Get the IStImage interface pointer to the converted image data.
			IStImage *pIStImage = pIStImageBuffer->GetIStImage();

			// Save as Bitmap
			{
				// Bitmap file extension.
				GenICam::gcstring strImageFileName(strFileNameHeader);
				strImageFileName.append(".bmp");

				// Save the image file in Bitmap format.
				std::wcout << std::endl << L"Saving " << strImageFileName.w_str().c_str() << L"... ";
				pIStStillImageFiler->Save(pIStImage, StStillImageFileFormat_Bitmap, strImageFileName);
				std::cout << "done" << std::endl;
			}

			// Save as Tiff
			{
				// Tiff file extension.
				GenICam::gcstring strImageFileName(strFileNameHeader);
				strImageFileName.append(".tif");

				// Save the image file in Tiff format.
				std::wcout << std::endl << L"Saving " << strImageFileName.w_str().c_str() << L"... ";
				pIStStillImageFiler->Save(pIStImage, StStillImageFileFormat_TIFF, strImageFileName);
				std::cout << "done" << std::endl;
			}

			//Save as PNG
			{
				// PNG file extension.
				GenICam::gcstring strImageFileName(strFileNameHeader);
				strImageFileName.append(".png");

				// Save the image file in PNG format.
				std::wcout << std::endl << L"Saving " << strImageFileName.w_str().c_str() << L"... ";
				pIStStillImageFiler->Save(pIStImage, StStillImageFileFormat_PNG, strImageFileName);
				std::cout << "done" << std::endl;
			}

			//Save as JPEG
			{
				// JPEG file extension.
				GenICam::gcstring strImageFileName(strFileNameHeader);
				strImageFileName.append(".jpg");

				// Save the image file in JPEG format.
				pIStStillImageFiler->SetQuality(75);
				std::wcout << std::endl << L"Saving " << strImageFileName.w_str().c_str() << L"... ";
				pIStStillImageFiler->Save(pIStImage, StStillImageFileFormat_JPEG, strImageFileName);
				std::cout << "done" << std::endl;
			}

			// Save as CSV
			{
				// CSV file extension.
				GenICam::gcstring strImageFileName(strFileNameHeader);
				strImageFileName.append(".csv");

				// Save the image file in CSV format.
				std::wcout << std::endl << L"Saving " << strImageFileName.w_str().c_str() << L"... ";
				pIStStillImageFiler->Save(pIStImage, StStillImageFileFormat_CSV, strImageFileName);
				std::cout << "done" << std::endl;
			}
		}

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
