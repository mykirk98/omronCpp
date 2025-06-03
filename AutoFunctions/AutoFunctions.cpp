/*!
\file AutoFunctions.cpp
\brief 

 This sample demostrates how to set AWB, AGC, and AE function.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Acquire image via callback function
 - Set AWB, AGC, AE 

 For more information, please refer to the help document of StApi.

*/

// Include files for using StApi.
#include <StApi_TL.h>
#include <StApi_GUI.h>

//Namespace for using StApi.
using namespace StApi;

//Namespace for using cout
using namespace std;

//Namespace for using GenApi.
using namespace GenApi;

//User parameter for callback function.
typedef StApi::IStImageDisplayWnd*	UserParam_t;

//Feature names
const char * EXPOSURE_AUTO = "ExposureAuto";			//Standard
const char * GAIN_AUTO = "GainAuto";					//Standard
const char * BALANCE_WHITE_AUTO = "BalanceWhiteAuto";	//Standard

const char * AUTO_LIGHT_TARGET = "AutoLightTarget";		//Custom
const char * GAIN = "Gain";								//Standard
const char * GAIN_RAW = "GainRaw";						//Custom

const char * EXPOSURE_MODE = "ExposureMode";			//Standard
const char * EXPOSURE_TIME = "ExposureTime";			//Standard
const char * EXPOSURE_TIME_RAW = "ExposureTimeRaw";		//Custom

const char * BALANCE_RATIO_SELECTOR = "BalanceRatioSelector";	//Standard
const char * BALANCE_RATIO = "BalanceRatio";			//Standard


//-----------------------------------------------------------------------------
// Callback function for image display.
//-----------------------------------------------------------------------------
void __stdcall OnStCallbackCFunction(IStCallbackParamBase *pIStCallbackParamBase, UserParam_t pIStImageDisplayWnd)
{
	// Check callback type. We only handle a NewBuffer event here.
	if(pIStCallbackParamBase->GetCallbackType() == StCallbackType_GenTLEvent_DataStreamNewBuffer)
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
				if (!pIStImageDisplayWnd->IsVisible())
				{
					// Sets the position and size of the window.
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
		catch (const GenICam::GenericException &e)
		{
			// Display a description of the error.
			cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
		}
	}
}

//-----------------------------------------------------------------------------
// List up the contents of current enumeration node with current setting.
//-----------------------------------------------------------------------------
void Enumeration(INodeMap *pINodeMap, const char *szEnumerationName)
{
	// Get the IEnumeration interface pointer
	CEnumerationPtr pIEnumeration(pINodeMap->GetNode(szEnumerationName));
	if (IsWritable(pIEnumeration))
	{
		// Get the setting list.
		GenApi::NodeList_t sNodeList;
		pIEnumeration->GetEntries(sNodeList);
		for (;;)
		{
			// Display a configurable options.
			cout << szEnumerationName << endl;
			for (size_t i = 0; i < sNodeList.size(); i++)
			{
				if (IsAvailable(sNodeList[i]))
				{
					CEnumEntryPtr pIEnumEntry(sNodeList[i]);
					cout << i << " : " << pIEnumEntry->GetSymbolic();
					// Add "(Current)" for indicating current setting.
					if (pIEnumeration->GetIntValue() == pIEnumEntry->GetValue())
					{
						cout << "(Current)";
					}
					cout << endl;
				}
			}
			cout << "Select : ";

			// Waiting for input.
			size_t nIndex;
			cin >> nIndex;

			// Reflect the value entered.
			if (nIndex < sNodeList.size())
			{
				CEnumEntryPtr pIEnumEntry(sNodeList[nIndex]);
				pIEnumeration->SetIntValue(pIEnumEntry->GetValue());
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// List up the numeric value of the current setting that the node indicated.
//-----------------------------------------------------------------------------
template <class VALUE_TYPE, class NODE_TYPE>
void Numeric(INodeMap *pINodeMap, const char *szNodeName)
{
	// Get the IInteger or IFloat interface pointer
	NODE_TYPE pINumeric(pINodeMap->GetNode(szNodeName));
	if (IsWritable(pINumeric))
	{
		for (;;)
		{
			// Display the feature name, range, current value, and incremental value.
			cout << szNodeName;
			cout << " Min=" << pINumeric->GetMin();
			cout << " Max=" << pINumeric->GetMax();
			cout << " Current=" << pINumeric->GetValue();
			EIncMode eIncMode = pINumeric->GetIncMode();
			if (eIncMode == fixedIncrement)
			{
				VALUE_TYPE nValue = pINumeric->GetInc();
				cout << " Inc=" << nValue;
			}
			cout << endl;

			cout << "New value : ";

			//Waiting for input of new value.
			VALUE_TYPE nValue;
			cin >> nValue;

			// Reflect the value entered.
			if ((pINumeric->GetMin() <= nValue) && (nValue <= pINumeric->GetMax()))
			{
				pINumeric->SetValue(nValue);
				break;
			}
		}
	}
}
//-----------------------------------------------------------------------------
// List up the contents of current enumeration node with current numeric setting.
//-----------------------------------------------------------------------------
template <class VALUE_TYPE, class NODE_TYPE>
void EnumerationAndNumeric(INodeMap *pINodeMap, const char *szEnumerationName, const char *szNumericName)
{
	// Get the IEnumeration interface pointer
	CEnumerationPtr pIEnumeration(pINodeMap->GetNode(szEnumerationName));
	if (IsWritable(pIEnumeration))
	{
		// Get the setting list
		GenApi::NodeList_t sNodeList;
		pIEnumeration->GetEntries(sNodeList);

		for (GenApi::NodeList_t::iterator itr = sNodeList.begin(); itr != sNodeList.end(); ++itr)
		{
			if (IsAvailable(*itr))
			{
				if (IsVisible((*itr)->GetVisibility(), Guru))
				{
					// Switch the setting target using the IEnumeration interface pointer
					CEnumEntryPtr pIEnumEntry = *itr;
					pIEnumeration->SetIntValue(pIEnumEntry->GetValue());

					// Display the selected setting target
					cout << szEnumerationName << "=" << pIEnumEntry->GetSymbolic() << endl;

					// Configure a numerical value
					Numeric<VALUE_TYPE, NODE_TYPE>(pINodeMap, szNumericName);
				}
			}
		}

	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ExposureAuto(INodeMap *pINodeMap)
{
	// Configure the ExposureMode
	Enumeration(pINodeMap, EXPOSURE_MODE);

	// Configure the ExposureAuto
	Enumeration(pINodeMap, EXPOSURE_AUTO);

	// Configure the AutoLightTarget
	Numeric<int64_t, CIntegerPtr>(pINodeMap, AUTO_LIGHT_TARGET);

	if (pINodeMap->GetNode(EXPOSURE_TIME) != NULL)
	{
		// Configure the ExposureTime
		Numeric<double, CFloatPtr>(pINodeMap, EXPOSURE_TIME);
	}
	else
	{
		// Configure the ExposureTimeRaw if the ExposureTime function does not exist
		Numeric<int64_t, CIntegerPtr>(pINodeMap, EXPOSURE_TIME_RAW);
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GainAuto(INodeMap *pINodeMap)
{
	// Configure the GainAuto
	Enumeration(pINodeMap, GAIN_AUTO);

	// Configure the AutoLightTarget
	Numeric<int64_t, CIntegerPtr>(pINodeMap, AUTO_LIGHT_TARGET);

	if (pINodeMap->GetNode(GAIN) != NULL)
	{
		// Configure the Gain
		Numeric<double, CFloatPtr>(pINodeMap, GAIN);
	}
	else
	{
		// Configure the GainRaw if the Gain function does not exist
		Numeric<int64_t, CIntegerPtr>(pINodeMap, GAIN_RAW);
	}

}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void BalanceWhiteAuto(INodeMap *pINodeMap)
{
	// Configure the BalanceWhiteAuto
	Enumeration(pINodeMap, BALANCE_WHITE_AUTO);

	// While switching the BalanceRatioSelector, configure the BalanceRatio
	EnumerationAndNumeric<double, CFloatPtr>(pINodeMap, BALANCE_RATIO_SELECTOR, BALANCE_RATIO);
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
		
		// Create an image display window object for display.
		CIStImageDisplayWndPtr pIStImageDisplayWnd(CreateIStWnd(StWindowType_ImageDisplay));

		// Create a DataStream object for handling image stream data.
		CIStDataStreamPtr pIStDataStream(pIStDevice->CreateIStDataStream(0));

		// Register callback function for display.
		CIStRegisteredCallbackPtr pIStRegisteredCallback(RegisterCallback(pIStDataStream, &OnStCallbackCFunction, (UserParam_t)pIStImageDisplayWnd));

		// Start the image acquisition of the host side.
		pIStDataStream->StartAcquisition();

		// Start the image acquisition of the camera side.
		pIStDevice->AcquisitionStart();

		// Create INodeMap object to access current setting of the camera.
		CNodeMapPtr pINodeMapRemote(pIStDevice->GetRemoteIStPort()->GetINodeMap());

		// Check if the camera has AE, AWB and AWB functions.
		const char* pszAutoFunctionNames[] = { EXPOSURE_AUTO, GAIN_AUTO, BALANCE_WHITE_AUTO};
		bool pisWritable[] = { false, false, false };
		for (size_t i = 0; i < sizeof(pisWritable) / sizeof(bool); i++)
		{
			// User INode interface to acquire the setting of camera with function name.
			CNodePtr pINode(pINodeMapRemote->GetNode(pszAutoFunctionNames[i]));
			if (IsWritable(pINode))
			{
				pisWritable[i] = true;
			}
		}

		for (;;)
		{
            // Display menu of setting function
			cout << "Auto Functions" << endl;
			for (size_t i = 0; i < sizeof(pisWritable) / sizeof(bool); i++)
			{
				if (pisWritable[i])
				{
					cout << i << " : " << pszAutoFunctionNames[i] << endl;
				}
			}
			cout << "Else : Exit" << endl;
			cout << "Select : ";

			// Waiting for input.
			size_t nInput;
			cin >> nInput;

			if ((nInput < sizeof(pisWritable) / sizeof(bool)) && pisWritable[nInput])
			{
				// Call the function for setting according to the input.
				switch (nInput)
				{
				case(0) : ExposureAuto(pINodeMapRemote); break;
				case(1) : GainAuto(pINodeMapRemote); break;
				case(2) : BalanceWhiteAuto(pINodeMapRemote); break;
				}
				
			}
			else
			{
				// Exit
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

	return(0);
}
