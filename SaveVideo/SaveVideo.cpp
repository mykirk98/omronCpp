/*!
\file SaveVideo.cpp
\brief 
 
 This sample shows how to save acquired image as AVI video file.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Create AVI video file

 For more information, please refer to the help document of StApi.

If the user name contains double-byte characters, incorrect characters might be displayed depending on your build or execution environment.
*/

// If you want to use the GUI features, please remove the comment.
//#define ENABLED_ST_GUI


// If you want to use a class method as a callback function, please remove the comment.
//#define ENABLED_CLASS_METHOD_TYPE_CALLBACK

// Include files for using StApi.
#include <StApi_TL.h>
#include <StApi_IP.h>
#ifdef ENABLED_ST_GUI
#include <StApi_GUI.h>
#endif
#include <iomanip>	//std::setprecision

// Namespace for using StApi.
using namespace StApi;

// Namespace for using cout
using namespace std;

// for SHGetFolderPath
#   include <Shlobj.h>

// Count of images to be grabbed.
const uint64_t nCountOfImagesToGrab = 500;

// The maximum number of images per file
const size_t iMaximumCountOfImagesPerFile = 200;

// Count of video files.
const size_t nCountOfVideoFiles = 3;

typedef void*	UserParam_t;
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OnCallback(IStCallbackParamBase *pIStCallbackParamBase, UserParam_t /*pvContext*/)
{
	EStCallbackType_t eCallbackType = pIStCallbackParamBase->GetCallbackType();
	if (eCallbackType == StCallbackType_StApiIPEvent_VideoFilerOpen)
	{
		IStCallbackParamStApiIPVideoFilerOpen *pIStCallbackParamStApiIPVideoFilerOpen = dynamic_cast<IStCallbackParamStApiIPVideoFilerOpen*>(pIStCallbackParamBase);
		wcout << L"Open:" << pIStCallbackParamStApiIPVideoFilerOpen->GetFileName().w_str().c_str() << endl;
	}
	else if (eCallbackType == StCallbackType_StApiIPEvent_VideoFilerClose)
	{
		IStCallbackParamStApiIPVideoFilerClose *pIStCallbackParamStApiIPVideoFilerClose = dynamic_cast<IStCallbackParamStApiIPVideoFilerClose*>(pIStCallbackParamBase);
		wcout << L"Close:" << pIStCallbackParamStApiIPVideoFilerClose->GetFileName().w_str().c_str() << endl;
	}
	else if (eCallbackType == StCallbackType_StApiIPEvent_VideoFilerError)
	{
		IStCallbackParamStApiIPVideoFilerError *pIStCallbackParamStApiIPVideoFilerError = dynamic_cast<IStCallbackParamStApiIPVideoFilerError*>(pIStCallbackParamBase);
		cout << "Error:" << pIStCallbackParamStApiIPVideoFilerError->GetException().what() << endl;
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
		
#ifdef ENABLED_ST_GUI
		// If using GUI for display, create a display window here.
		CIStImageDisplayWndPtr pIStImageDisplayWnd(CreateIStWnd(StWindowType_ImageDisplay));
#endif

		// Get the acquisition frame rate of the camera.
		double fps = 60.0;
		GenApi::CFloatPtr pIFloat_AcquisitionFrameRate(pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("AcquisitionFrameRate"));
		if (pIFloat_AcquisitionFrameRate)
		{
			fps = pIFloat_AcquisitionFrameRate->GetValue();
		}

		// Create a VideoFiler object for video file handling.
		CIStVideoFilerPtr pIStVideoFiler(CreateIStFiler(StFilerType_Video));

		// Register a callback function to IStVideoFiler interface pointer.
#ifdef ENABLED_CLASS_METHOD_TYPE_CALLBACK
		CCallback objCCallback;
		RegisterCallback(pIStVideoFiler, objCCallback, &CCallback::OnStCallbackClassMethod, (UserParam_t)NULL);
#else
		RegisterCallback(pIStVideoFiler, &OnStCallbackCFunction, (UserParam_t)NULL);
#endif

		// Configure the video file settings.
		pIStVideoFiler->SetMaximumFrameCountPerFile(iMaximumCountOfImagesPerFile);
		pIStVideoFiler->SetVideoFileFormat(StVideoFileFormat_AVI2);
		pIStVideoFiler->SetVideoFileCompression(StVideoFileCompression_MotionJPEG);
		pIStVideoFiler->SetFPS(fps);

		// Get the path of the video files.
		wchar_t szPath[MAX_PATH];
		SHGetFolderPathW(NULL, CSIDL_MYVIDEO, NULL, 0, szPath);

		// Register the file name of the video files
		for (size_t i = 0; i < nCountOfVideoFiles; i++)
		{
			GenICam::gcstring strFileName(szPath);
			strFileName.append("\\SaveVideo");
			
			stringstream ss;
			ss << i;
			strFileName.append(ss.str().c_str());
			strFileName.append(".avi");
			pIStVideoFiler->RegisterFileName(strFileName);
		}

		// Create a DataStream object for handling image stream data.
		CIStDataStreamPtr pIStDataStream(pIStDevice->CreateIStDataStream(0));

		// Start the image acquisition of the host side.
		pIStDataStream->StartAcquisition(nCountOfImagesToGrab);

		// Start the image acquisition of the camera side.
		pIStDevice->AcquisitionStart();

		bool fFirstFrame = true;
		uint64_t nFirstFrameTimestamp = 0;

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
				// This will have a copy of the image data and original buffer can be released if necessary and original buffer can be released if necessary.
				pIStImageDisplayWnd->RegisterIStImage(pIStImage);
#else
				// Display the information of the acquired image data.
				cout << "BlockId=" << pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size:" << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight()
					<< " " << setprecision(4) << pIStStreamBuffer->GetIStDataStream()->GetCurrentFPS() << "FPS" << endl;
#endif
				// Calculating the frame number in consideration of the frame drop.
				uint32_t nFrameNo = 0;
				const uint64_t nCurrentTimestamp = pIStStreamBuffer->GetIStStreamBufferInfo()->GetTimestamp();
				if (fFirstFrame)
				{
					fFirstFrame = false;
					nFirstFrameTimestamp = nCurrentTimestamp;
				}
				else
				{
					uint64_t nDelta = nCurrentTimestamp - nFirstFrameTimestamp;
					double dblTmp = nDelta * fps;
					dblTmp /= 1000000000;
					nFrameNo = (uint32_t)(dblTmp + 0.5);
				}

				// Add the image data to a video file.
				pIStVideoFiler->RegisterIStImage(pIStImage, nFrameNo);
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
	while(cin.get() != '\n');

	return(0);
}
