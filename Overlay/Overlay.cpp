/*!
\file Overlay.cpp
\brief 
 
 This sample shows how to use the callback functions to draw on top of the acquired image.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Acquire image data (with waiting in main thread)
 - Use callback function to draw on top of the acquired image.

 For more information, please refer to the help document of StApi.

*/



// Include files for using StApi.
#include <StApi_TL.h>
#include <StApi_GUI.h>
#include <iomanip>	//std::setprecision

//Namespace for using StApi.
using namespace StApi;

//Namespace for using cout
using namespace std;

//Count of images to be grabbed.
const uint64_t nCountOfImagesToGrab = 500;

typedef void*	UserParam_t;


//-----------------------------------------------------------------------------
// Function for drawing ellipse.
//-----------------------------------------------------------------------------
void DrawEllipse(HDC hDC, const RECT *pRect, COLORREF color)
{

	HPEN hPen = reinterpret_cast<HPEN>(CreatePen(PS_SOLID, 2, color));
	HPEN hOldPen = reinterpret_cast<HPEN>(SelectObject(hDC, hPen));
	HBRUSH hBrush = reinterpret_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
	HBRUSH hOldBrush = reinterpret_cast<HBRUSH>(SelectObject(hDC, hBrush));
	//MoveToEx(hDC, 0, 0, NULL);
	Ellipse(hDC, pRect->left, pRect->top, pRect->right, pRect->bottom);
	SelectObject(hDC, hOldBrush);
	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
}

//-----------------------------------------------------------------------------
// Function for drawing line.
//-----------------------------------------------------------------------------
void DrawLine(HDC hDC, const POINT *pStartPoint, const POINT *pEndPoint, COLORREF color)
{

	HPEN hPen = reinterpret_cast<HPEN>(CreatePen(PS_SOLID, 2, color));
	HPEN hOldPen = reinterpret_cast<HPEN>(SelectObject(hDC, hPen));
	MoveToEx(hDC, pStartPoint->x, pStartPoint->y, NULL);
	LineTo(hDC, pEndPoint->x, pEndPoint->y);
	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
}
//-----------------------------------------------------------------------------
// Function for drawing rectangle.
//-----------------------------------------------------------------------------
void DrawRect(HDC hDC, const RECT *pRect, COLORREF colorBrush, COLORREF colorPen)
{
	HPEN hPen = reinterpret_cast<HPEN>(CreatePen(PS_SOLID, 2, colorPen));
	HPEN hOldPen = reinterpret_cast<HPEN>(SelectObject(hDC, hPen));
	HBRUSH hBrush = reinterpret_cast<HBRUSH>(CreateSolidBrush(colorBrush));
	HBRUSH hOldBrush = reinterpret_cast<HBRUSH>(SelectObject(hDC, hBrush));
	//FillRect(hDC, pRect, hBrush);
	//FrameRect(hDC, pRect, hBrush);
	//InvertRect(hDC, pRect);
	Rectangle(hDC, pRect->left, pRect->top, pRect->right, pRect->bottom);
	SelectObject(hDC, hOldBrush);
	DeleteObject(hBrush);
	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
}
//-----------------------------------------------------------------------------
// Function for drawing polygon.
//-----------------------------------------------------------------------------
void DrawPolygon(HDC hDC, const POINT *pPoints, int nCount, COLORREF colorBrush, COLORREF colorPen)
{
	HPEN hPen = reinterpret_cast<HPEN>(CreatePen(PS_SOLID, 2, colorPen));
	HPEN hOldPen = reinterpret_cast<HPEN>(SelectObject(hDC, hPen));
	HBRUSH hBrush = reinterpret_cast<HBRUSH>(CreateSolidBrush(colorBrush));
	HBRUSH hOldBrush = reinterpret_cast<HBRUSH>(SelectObject(hDC, hBrush));
	Polygon(hDC, pPoints, nCount);
	SelectObject(hDC, hOldBrush);
	DeleteObject(hBrush);
	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
}
//-----------------------------------------------------------------------------
// Function for drawing text.
//-----------------------------------------------------------------------------
void DrawText(HDC hDC, const POINT *pPoint, LPCTSTR szText, COLORREF colorText, COLORREF colorBack)
{
	COLORREF oldTextColor = SetTextColor(hDC, colorText);
	COLORREF oldBkColor = SetBkColor(hDC, colorBack);
	
	RECT rect = { pPoint->x, pPoint->y, pPoint->x, pPoint->y };
	DrawText(hDC, szText, -1, &rect, DT_NOCLIP);

	SetBkColor(hDC, oldBkColor);
	SetTextColor(hDC, oldTextColor);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OnCallback(IStCallbackParamBase *pIStCallbackParamBase, UserParam_t /*pvContext*/)
{
	if (pIStCallbackParamBase->GetCallbackType() == StCallbackType_StApiGUIEvent_DisplayImageWndDrawing)
	{

		StApi::IStCallbackParamStApiGUIEventDrawing *pIStCallbackParamStApiGUIEventDrawing = dynamic_cast<StApi::IStCallbackParamStApiGUIEventDrawing*>(pIStCallbackParamBase);
		//StApi::IStImageDisplayWnd *pIStImageDisplayWnd = dynamic_cast<StApi::IStImageDisplayWnd*>(pIStCallbackParamStApiGUIEventDrawing->GetIStWnd());
		HDC hDC = pIStCallbackParamStApiGUIEventDrawing->GetDC();
		StApi::IStImage *pIStImage = pIStCallbackParamStApiGUIEventDrawing->GetIStImage();
		const size_t nImageWidth = pIStImage->GetImageWidth();
		const size_t nImageHeight = pIStImage->GetImageHeight();

		const size_t nROIOffsetX = pIStCallbackParamStApiGUIEventDrawing->GetROIOffsetX();
		const size_t nROIOffsetY = pIStCallbackParamStApiGUIEventDrawing->GetROIOffsetY();
		const size_t nROIWidth = pIStCallbackParamStApiGUIEventDrawing->GetROIWidth();
		const size_t nROIHeight = pIStCallbackParamStApiGUIEventDrawing->GetROIHeight();
		const size_t nDisplayWidth = pIStCallbackParamStApiGUIEventDrawing->GetDisplayWidth();
		const size_t nDisplayHeight = pIStCallbackParamStApiGUIEventDrawing->GetDisplayHeight();

		const double dblMagnificationH = nDisplayWidth / (double)nROIWidth;
		const double dblMagnificationV = nDisplayHeight / (double)nROIHeight;

		// Draw an ellipse inside the display image
		{
			const RECT rect = { static_cast<LONG>(-(nROIOffsetX * dblMagnificationH)), static_cast<LONG>(-(nROIOffsetY * dblMagnificationV)), static_cast<LONG>((nImageWidth - nROIOffsetX) * dblMagnificationH), static_cast<LONG>((nImageHeight - nROIOffsetY) * dblMagnificationV) };
			DrawEllipse(hDC, &rect, RGB(0, 255, 0));
		}
		// Draw an ellipse inside the display area.
		{
			const RECT rect = { 0, 0, static_cast<LONG>(nDisplayWidth), static_cast<LONG>(nDisplayHeight) };
			DrawEllipse(hDC, &rect, RGB(255, 0, 0));
		}

		// Center lines.
		{
			const POINT ptStart = { static_cast<LONG>(nImageWidth * dblMagnificationH / 2), 0 };
			const POINT ptEnd = { static_cast<LONG>(nImageWidth * dblMagnificationH / 2), static_cast<LONG>(nImageHeight * dblMagnificationV) };
			DrawLine(hDC, &ptStart, &ptEnd, RGB(255, 255, 0));
		}
		{
			const POINT ptStart = { 0, static_cast<LONG>(nImageHeight * dblMagnificationV / 2) };
			const POINT ptEnd = { static_cast<LONG>(nImageWidth * dblMagnificationH), static_cast<LONG>(nImageHeight * dblMagnificationV / 2) };
			DrawLine(hDC, &ptStart, &ptEnd, RGB(255, 255, 0));
		}

		// Draw rect
		{
			const RECT rect = { 10, 10, 100, 100 };
			DrawRect(hDC, &rect, RGB(255, 0, 255), RGB(0, 255, 255));
		}

		// Draw polygon
		{
			const int nOffsetX = 200;
			const int nOffsetY = 200;
			const POINT pPoint[] = 
			{
				{ nOffsetX + 0, nOffsetY + 0 },
				{ nOffsetX + 100, nOffsetY + 50 },
				{ nOffsetX + 0, nOffsetY + 100 },
			};
			DrawPolygon(hDC, pPoint, _countof(pPoint), RGB(255, 0, 255), RGB(0, 255, 255));
		}

		//Draw text
		{
			const POINT ptPos = { 300, 300 };
			DrawText(hDC, &ptPos, TEXT("Text"), RGB(255, 255, 0), RGB(0, 255, 255));
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

		// Create an image display window object for showing image.
		CIStImageDisplayWndPtr pIStImageDisplayWnd(CreateIStWnd(StWindowType_ImageDisplay));

		// Register a callback function. When an event occurs for ImageDisplayWnd, function registered is called.
#ifdef ENABLED_CLASS_METHOD_TYPE_CALLBACK
		CCallback objCCallback;
		RegisterCallback(pIStImageDisplayWnd, objCCallback, &CCallback::OnStCallbackClassMethod, (UserParam_t)NULL);
#else
		RegisterCallback(pIStImageDisplayWnd, &OnStCallbackCFunction, (UserParam_t)NULL);
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

			// Check if the acquired data contains image data.
			if (pIStStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				// If yes, we create a IStImage object for further image handling.
				IStImage *pIStImage = pIStStreamBuffer->GetIStImage();

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

				// Display window.
				if (!pIStImageDisplayWnd->IsVisible())
				{
					// Set the position and size of the window.
					pIStImageDisplayWnd->SetPosition(0, 0, static_cast<int32_t>(pIStImage->GetImageWidth()), static_cast<int32_t>(pIStImage->GetImageHeight()));

					// Create a new thread to display the window.
					pIStImageDisplayWnd->Show(NULL, StWindowMode_ModalessOnNewThread);
				}

				// Register the image to be displayed.
				// This will have a copy of the image data and original buffer can be released if necessary and original buffer can be released if necessary.
				pIStImageDisplayWnd->RegisterIStImage(pIStImage);
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
