/*!
\file CameraSideLUT.cpp
\brief

This sample shows how to use the camera side look up table.
The following points will be demonstrated in this sample code:
- Initialize StApi
- Connect to camera
- Acquire image data via callback function
- LUT function of the camera.
- Drawing graph function.

For more information, please refer to the help document of StApi.

*/

// Include files for using StApi.
#include <StApi_TL.h>
#include <StApi_IP.h>
#include <StApi_GUI.h>
#include <iomanip>	//std::setprecision
#include <math.h>

//Namespace for using StApi.
using namespace StApi;

//Namespace for using GenApi.
using namespace GenApi;

//Namespace for using cout
using namespace std;

typedef IStImageDisplayWnd* UserParam_t;

//-----------------------------------------------------------------------------
// Function for handling callback action
//-----------------------------------------------------------------------------
void __stdcall OnStCallbackCFunction(IStCallbackParamBase *pIStCallbackParamBase, UserParam_t pvContext)
{
	// Check callback type. Only NewBuffer event is handled in here
	if (pIStCallbackParamBase->GetCallbackType() == StCallbackType_GenTLEvent_DataStreamNewBuffer)
	{
		// In case of receiving a NewBuffer events:
		// Convert received callback parameter into IStCallbackParamGenTLEventNewBuffer for acquiring additional information.
		IStCallbackParamGenTLEventNewBuffer *pIStCallbackParamGenTLEventNewBuffer = dynamic_cast<IStCallbackParamGenTLEventNewBuffer*>(pIStCallbackParamBase);

		try
		{
			// Get the IStDataStream interface pointer from the received callback parameter.
			IStDataStream *pIStDataStream = pIStCallbackParamGenTLEventNewBuffer->GetIStDataStream();

			// Retrieve the buffer pointer of image data for that callback indicated there is a buffer received.
			CIStStreamBufferPtr pIStStreamBuffer(pIStDataStream->RetrieveBuffer(0));

			// Check if the acquired data contains image data.
			if (pIStStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{

				// If yes, we create a IStImage object for further image handling.
				IStImage *pIStImage = pIStStreamBuffer->GetIStImage();

				// Check if display window is visible.
				if (!pvContext->IsVisible())
				{
					// Set the position and size of the window.
					pvContext->SetPosition(0, 0, (int32_t)(pIStImage->GetImageWidth()), (int32_t)(pIStImage->GetImageHeight()));

					// Create a new thread to display the window.
					pvContext->Show(NULL, StWindowMode_ModalessOnNewThread);
				}

				// Register the image to be displayed.
				// This will have a copy of the image data and original buffer can be released if necessary.
				pvContext->RegisterIStImage(pIStImage);
			}
			else
			{
				// If the acquired data contains no image data...
				cout << "Image data does not exist." << endl;
			}
		}
		catch (const GenICam::GenericException &e)
		{
			// If any exception occurred, display the description of the error here.
			cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MakeGammaTable(double dblGammaCorrectionValue, vector<uint16_t> &vecGammaTable)
{
	if (0 < dblGammaCorrectionValue)
	{
		const double dblReciprocalNumber = 1 / dblGammaCorrectionValue;
		const size_t nMaxValue = vecGammaTable.size() - 1;
		for (size_t i = 0; i < vecGammaTable.size(); ++i)
		{
			const double dblInput = i / (double)nMaxValue;
			const double dblOutput = pow(dblInput, dblReciprocalNumber);
			vecGammaTable[i] = (uint16_t)((dblOutput * nMaxValue) + 0.5);
		}
	}
	else
	{
		memset(&vecGammaTable[0], 0, sizeof(vecGammaTable[0]) * vecGammaTable.size());
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SwapEndianness(uint16_t *pwBuffer, size_t nCount)
{
	uint8_t *pbyteBuffer = reinterpret_cast<uint8_t*>(pwBuffer);
	for (size_t i = 0; i < nCount; ++i)
	{
		const uint8_t byteTmp = pbyteBuffer[0];
		pbyteBuffer[0] = pbyteBuffer[1];
		pbyteBuffer[1] = byteTmp;
		pbyteBuffer += 2;
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SendGammaTable(GenApi::CNodeMapPtr pINodeMap, vector<uint16_t> &vecGammaTable, bool enabledSwapEndian)
{
	CIntegerPtr pIInteger_LUTIndex(pINodeMap->GetNode("LUTIndex"));
	const size_t nMaxValue = (size_t)(pIInteger_LUTIndex->GetMax());
	bool bUseValueAll = false;
	CRegisterPtr pIRegister_LUTValueAll(pINodeMap->GetNode("LUTValueAll"));

	// Checks availability of the 'LUTValueAll' node.
	if (IsAvailable(pIRegister_LUTValueAll))
	{
		if (pIRegister_LUTValueAll->GetLength() == (nMaxValue + 1) * sizeof(uint16_t))
		{
			bUseValueAll = true;
		}
	}

	if (bUseValueAll)
	{
		// If 'UseValueAll' node is available, it is used.
		if (enabledSwapEndian)
		{
			SwapEndianness(&vecGammaTable[0], vecGammaTable.size());
		}
		pIRegister_LUTValueAll->Set((uint8_t*)&vecGammaTable[0], vecGammaTable.size() * sizeof(vecGammaTable[0]));
	}
	else
	{
		//If 'UseValueAll' node isn't available, 'LUTValue' node is used.
		CIntegerPtr pIInteger_LUTValue(pINodeMap->GetNode("LUTValue"));
		for (size_t i = 0; i <= nMaxValue; ++i)
		{
			pIInteger_LUTIndex->SetValue(i);
			pIInteger_LUTValue->SetValue(vecGammaTable[i]);
		}
	}

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RcvGammaTable(CIntegerPtr pIInteger_LUTIndex, CRegisterPtr pIRegister_LUTValueAll, CIntegerPtr pIInteger_LUTValue, vector<uint16_t> &vecGammaTable, bool enabledSwapEndian)
{
	const size_t nMaxValue = (size_t)(pIInteger_LUTIndex->GetMax());

	bool bUseValueAll = false;

	// Checks availability of the 'LUTValueAll' node.
	if (IsAvailable(pIRegister_LUTValueAll))
	{
		if (pIRegister_LUTValueAll->GetLength() == (nMaxValue + 1) * sizeof(uint16_t))
		{
			bUseValueAll = true;
		}
	}

	if (bUseValueAll)
	{
		// If 'UseValueAll' node is available, it is used.
		pIRegister_LUTValueAll->Get((uint8_t*)&vecGammaTable[0], vecGammaTable.size() * sizeof(vecGammaTable[0]));
		if (enabledSwapEndian)
		{
			SwapEndianness(&vecGammaTable[0], vecGammaTable.size());
		}
	}
	else
	{
		// If 'UseValueAll' node is available, it is used.
		for (size_t i = 0; i <= nMaxValue; ++i)
		{
			pIInteger_LUTIndex->SetValue(i);
			vecGammaTable[i] = (uint16_t)(pIInteger_LUTValue->GetValue());
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SetGammaCorrection(GenApi::CNodeMapPtr pINodeMap, double dblGammaCorrectionValue, bool enabledSwapEndian)
{
	CBooleanPtr pIBoolean_LUTEnable(pINodeMap->GetNode("LUTEnable"));
	CIntegerPtr pIInteger_LUTIndex(pINodeMap->GetNode("LUTIndex"));

	//Enable LUT.
	pIBoolean_LUTEnable->SetValue(true);

	// LUT is made and sent to the camera.
	const size_t nMaxValue = (size_t)(pIInteger_LUTIndex->GetMax());
	vector<uint16_t> vecGammaTable;
	vecGammaTable.resize(nMaxValue + 1);
	MakeGammaTable(dblGammaCorrectionValue, vecGammaTable);
	SendGammaTable(pINodeMap, vecGammaTable, enabledSwapEndian);

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void UpdateLUTGraph(CBooleanPtr pIBoolean_LUTEnable, CIntegerPtr pIInteger_LUTIndex, CRegisterPtr pIRegister_LUTValueAll, CIntegerPtr pIInteger_LUTValue, IStGraphDisplayWnd *pIStGraphDisplayWnd, bool enabledSwapEndian)
{
	//Enable LUT.
	pIBoolean_LUTEnable->SetValue(true);

	// Receive LUT.
	const size_t nMaxValue = (size_t)(pIInteger_LUTIndex->GetMax());
	vector<uint16_t> vecGammaTable;
	vecGammaTable.resize(nMaxValue + 1);
	RcvGammaTable(pIInteger_LUTIndex, pIRegister_LUTValueAll, pIInteger_LUTValue, vecGammaTable, enabledSwapEndian);

	//Create buffer for graph.
	CIStGraphDataBufferListResizablePtr pIStGraphDataBufferListResizable(CreateIStGraphDataBufferListResizable());
	pIStGraphDataBufferListResizable->CreateBufferList(1);
	IStGraphDataBufferResizable *pIStGraphDataBufferResizable(pIStGraphDataBufferListResizable->GetIStGraphDataBuffer(0));
	pIStGraphDataBufferResizable->Resize(StNumberDataType_uint32, nMaxValue + 1);

	//Copy LUT values to the buffer for graph.
	IStGraphDataBuffer *pIStGraphDataBuffer(pIStGraphDataBufferListResizable->GetIStGraphDataBuffer(0));
	IStGraphData *pIStGraphData(pIStGraphDataBuffer->GetIStGraphData());
	uint32_t * const pdwBuffer = reinterpret_cast<uint32_t*>(pIStGraphData->GetDataPtr());
	for (size_t i = 0; i <= nMaxValue; ++i)
	{
		pdwBuffer[i] = vecGammaTable[i];
	}

	//Register the graph data to the graph display window.
	pIStGraphDisplayWnd->RegisterIStGraphDataBufferList(pIStGraphDataBufferListResizable);

	// Check if display window is visible.
	if (!pIStGraphDisplayWnd->IsVisible())
	{
		// Create a new thread to display the window.
		pIStGraphDisplayWnd->Show(NULL, StWindowMode_ModalessOnNewThread);
	}
}

//-----------------------------------------------------------------------------
// Main thread
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

		//Checks endianness of the host and the camera.
		const uint32_t dwValue1 = 1;
		const bool bLittleEndianHost = ((*(uint8_t*)&dwValue1) == 0x01);
		GenApi::CEnumerationPtr pIEnumeration_DeviceRegistersEndianness(pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("DeviceRegistersEndianness"));
		const bool bLittleEndianCamera = (pIEnumeration_DeviceRegistersEndianness->GetCurrentEntry()->GetSymbolic().compare("Little") == 0);
		const bool enabledSwapEndian = (!bLittleEndianHost && bLittleEndianCamera) || (bLittleEndianHost && !bLittleEndianCamera);

		// If using GUI for display, create a display window here.
		CIStImageDisplayWndPtr pIStImageDisplayWnd(CreateIStWnd(StWindowType_ImageDisplay));

		CIStGraphDisplayWndPtr pIStGraphDisplayWnd(CreateIStWnd(StWindowType_GraphDisplay));
		CNodeMapPtr pINodeMap_IStGraphDisplayWnd(pIStGraphDisplayWnd->GetINodeMap());
		CEnumerationPtr pIEnumeration_ScaleMode(pINodeMap_IStGraphDisplayWnd->GetNode("ScaleMode"));
		pIEnumeration_ScaleMode->SetIntValue(pIEnumeration_ScaleMode->GetEntryByName("Linear")->GetValue());

		//Sets the graph line to green.
		CEnumerationPtr pIEnumeration_LineInfoSelector(pINodeMap_IStGraphDisplayWnd->GetNode("LineInfoSelector"));
		CEnumerationPtr pIEnumeration_LineForegroundColorSelector(pINodeMap_IStGraphDisplayWnd->GetNode("LineForegroundColorSelector"));
		CIntegerPtr pIInteger_LineForegroundColor(pINodeMap_IStGraphDisplayWnd->GetNode("LineForegroundColor"));
		*pIEnumeration_LineInfoSelector = "GraphLineCustomStyle0";
		*pIEnumeration_LineForegroundColorSelector = "Red";
		pIInteger_LineForegroundColor->SetValue(0);
		*pIEnumeration_LineForegroundColorSelector = "Green";
		pIInteger_LineForegroundColor->SetValue(255);
		*pIEnumeration_LineForegroundColorSelector = "Blue";
		pIInteger_LineForegroundColor->SetValue(0);


		//Check if the camera implements the Lookup table feature.
		CNodeMapPtr pINodeMap_RemoteDevice(pIStDevice->GetRemoteIStPort()->GetINodeMap());
		CBooleanPtr pIBoolean_LUTEnable(pINodeMap_RemoteDevice->GetNode("LUTEnable"));
		CIntegerPtr pIInteger_LUTIndex(pINodeMap_RemoteDevice->GetNode("LUTIndex"));
		CRegisterPtr pIRegister_LUTValueAll(pINodeMap_RemoteDevice->GetNode("LUTValueAll"));
		CIntegerPtr pIInteger_LUTValue(pINodeMap_RemoteDevice->GetNode("LUTValue"));

		if (
			(!pIInteger_LUTIndex.IsValid()) ||
			((!pIRegister_LUTValueAll.IsValid()) && (!pIInteger_LUTValue.IsValid()))
			)
		{
			cout << "The camera does not have a Lookup table." << endl;
			return(0);
		}

		UpdateLUTGraph(pIBoolean_LUTEnable, pIInteger_LUTIndex, pIRegister_LUTValueAll, pIInteger_LUTValue, pIStGraphDisplayWnd, enabledSwapEndian);

		// Create a DataStream object for handling image stream data.
		CIStDataStreamPtr pIStDataStream(pIStDevice->CreateIStDataStream(0));

		// Register callback function. Note that by different usage, we pass different kinds/numbers of parameters in.
		RegisterCallback(pIStDataStream, &OnStCallbackCFunction, static_cast<UserParam_t>(pIStImageDisplayWnd));

		// Start the image acquisition of the host (local machine) side.
		pIStDataStream->StartAcquisition();

		const double pdblGammaCorrectonValues[] = { 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0, 1.0 };
		for (size_t i = 0; i < _countof(pdblGammaCorrectonValues); ++i)
		{

			//Update LUT by using new gamma correction value.
			SetGammaCorrection(pIStDevice->GetRemoteIStPort()->GetINodeMap(), pdblGammaCorrectonValues[i], enabledSwapEndian);

			//Update graph.
			UpdateLUTGraph(pIBoolean_LUTEnable, pIInteger_LUTIndex, pIRegister_LUTValueAll, pIInteger_LUTValue, pIStGraphDisplayWnd, enabledSwapEndian);

			char szTitle[64];
			sprintf_s(szTitle, _countof(szTitle), "Gamma=%.1f", pdblGammaCorrectonValues[i]);
			pIStGraphDisplayWnd->SetTitle(szTitle);

			cout << szTitle << endl;

			// Start the image acquisition of the camera side.
			pIStDevice->AcquisitionStart();

			// wait.
			Sleep(2500);

			// Stop the image acquisition of the camera side.
			pIStDevice->AcquisitionStop();
		}

		// Stop the image acquisition of the host side.
		pIStDataStream->StopAcquisition();
	}
	catch (const GenICam::GenericException &e)
	{
		// If any exception occurred, display the description of the error here.
		cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
	}

	// Keep getting image until Enter key pressed.
	cout << endl << "Press Enter to exit." << endl;
	for (;;)
	{
		if (cin.get() == '\n') break;
	}

	return(0);
}
