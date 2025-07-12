/*!
\file GigEConfigurations.cpp
\brief 
 
 This sample shows how to setup the IP address and heartbeat timeout of GigE camera.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Acquire image data (with waiting in main thread)
 - Check and update IP address of GigE camera
 - Update heartbeat timeout of GigE camera

 For more information, please refer to the help document of StApi.
*/

// If you want to use the GUI features, please remove the comment.
//#define ENABLED_ST_GUI

// Include files for using StApi.
#include <StApi_TL.h>
#ifdef ENABLED_ST_GUI
#include <StApi_GUI.h>
#endif

#if defined(_WIN32_WINNT_WIN8) && (_WIN32_WINNT_WIN8 <= WINVER)
#include <WinSock2.h>	//AF_INET
#include <ws2tcpip.h>	//inet_pton
#else
#include <WinSock2.h> // For inet_addr
#endif
#pragma comment(lib, "Ws2_32.lib")

// Namespace for using StApi.
using namespace StApi;

// Namespace for using cout
using namespace std;

//Count of images to be grabbed.
const uint64_t nCountOfImagesToGrab = 100;

//Feature names
const char * GEV_INTERFACE_SUBNET_IP_ADDRESS = "GevInterfaceSubnetIPAddress";	//Standard
const char * GEV_INTERFACE_SUBNET_MASK = "GevInterfaceSubnetMask";				//Standard

const char * DEVICE_SELECTOR = "DeviceSelector";								//Standard
const char * GEV_DEVICE_IP_ADDRESS = "GevDeviceIPAddress";						//Standard
const char * GEV_DEVICE_SUBNET_MASK = "GevDeviceSubnetMask";					//Standard

const char * GEV_DEVICE_FORCE_IP_ADDRESS = "GevDeviceForceIPAddress";	//Standard
const char * GEV_DEVICE_FORCE_SUBNET_MASK = "GevDeviceForceSubnetMask";	//Standard
const char * GEV_DEVICE_FORCE_IP = "GevDeviceForceIP";							//Standard
const char * DEVICE_LINK_HEARTBEAT_TIMEOUT = "DeviceLinkHeartbeatTimeout";		//Standard[us]
const char * GEV_HEARTBEAT_TIMEOUT = "GevHeartbeatTimeout";						//Standard(Deprecated)[ms]

#ifdef ENABLED_ST_GUI
//-----------------------------------------------------------------------------
// GUI for checking and updating IP address of camera.
//-----------------------------------------------------------------------------
void UpdateDeviceIPAddress(GenApi::INodeMap *pINodeMap)
{
	// Create an NodeMap display window.
	CIStNodeMapDisplayWndPtr pIStNodeMapDisplayWnd(StApi::CreateIStWnd(StWindowType_NodeMapDisplay));

	// Register the node to NodeMap window.
	pIStNodeMapDisplayWnd->RegisterINode(pINodeMap->GetNode("Root"), "Interface");

	// Display the window.
	pIStNodeMapDisplayWnd->Show(NULL, StWindowMode_Modal);

}
#else //ENABLED_ST_GUI
//-----------------------------------------------------------------------------
// Console function for checking and updating IP address of camera.
//-----------------------------------------------------------------------------
void UpdateDeviceIPAddress(GenApi::INodeMap *pINodeMap)
{
	// Display the IP address of the host side.
	GenApi::CIntegerPtr pGevInterfaceSubnetIPAddress(pINodeMap->GetNode(GEV_INTERFACE_SUBNET_IP_ADDRESS));
	cout << "Interface IP Address=" << pGevInterfaceSubnetIPAddress->ToString() << endl;

	// Display the subnet mask of the host side.
	GenApi::CIntegerPtr pGevInterfaceSubnetMask(pINodeMap->GetNode(GEV_INTERFACE_SUBNET_MASK));
	cout << "Interface Subnet Mask=" << pGevInterfaceSubnetMask->ToString() << endl;

	bool fExit = false;
	for (;;)
	{
		// Select the first camera.
		const size_t nDeviceSelectorValue = 0;
		GenApi::CIntegerPtr pDeviceSelector(pINodeMap->GetNode(DEVICE_SELECTOR));
		pDeviceSelector->SetValue(nDeviceSelectorValue);

		// Display the current IP address of the camera.
		GenApi::CIntegerPtr pGevDeviceIPAddress(pINodeMap->GetNode(GEV_DEVICE_IP_ADDRESS));
		cout << "Device IP Address=" << pGevDeviceIPAddress->ToString() << endl;

		// Display the current subnet mask of the camera.
		GenApi::CIntegerPtr pGevDeviceSubnetMask(pINodeMap->GetNode(GEV_DEVICE_SUBNET_MASK));
		cout << "Device Subnet Mask=" << pGevDeviceSubnetMask->ToString() << endl;

		if (fExit)
		{
			break;
		}

		// Waiting for new IP address.
		cout << "Input new device IP address : ";
		string strInput;
		cin >> strInput;

		// Convert the new IP address string to a 32-bit number.
#if defined(_WIN32_WINNT_WIN8) && (_WIN32_WINNT_WIN8 <= WINVER)
		uint32_t nNewDeviceIPAddress;
		if (!inet_pton(AF_INET, strInput.c_str(), &nNewDeviceIPAddress))
		{
			nNewDeviceIPAddress = 0;
		}
		else
		{
			nNewDeviceIPAddress = ntohl(nNewDeviceIPAddress);
		}
#else
		const uint32_t nNewDeviceIPAddress = ntohl(inet_addr(strInput.c_str()));
#endif

		// Get the subnet mask of the host side.
		const uint32_t nSubnetMask = (uint32_t)pGevInterfaceSubnetMask->GetValue();

		// Get the IP address of the host side.
		const uint32_t nInterfaceIPAddress = (uint32_t)pGevInterfaceSubnetIPAddress->GetValue();

		// Ensure that the subnet address of the host and the camera are matched, and that the host and IP address of camera are different.
		if (((nInterfaceIPAddress & nSubnetMask) == (nNewDeviceIPAddress & nSubnetMask)) && (nInterfaceIPAddress != nNewDeviceIPAddress))
		{
			// Specify the new IP address of the camera. At this point, the camera settings will not be updated.
			GenApi::CIntegerPtr pGevDeviceForceIPAddress(pINodeMap->GetNode(GEV_DEVICE_FORCE_IP_ADDRESS));
			pGevDeviceForceIPAddress->SetValue(nNewDeviceIPAddress);

			// Specify the new subnet mask of the camera. At this point, the camera settings will not be updated.
			GenApi::CIntegerPtr pGevDeviceForceSubnetMask(pINodeMap->GetNode(GEV_DEVICE_FORCE_SUBNET_MASK));
			pGevDeviceForceSubnetMask->SetValue(nSubnetMask);
			
			// Update the camera settings.
			GenApi::CCommandPtr pGevDeviceForceIP(pINodeMap->GetNode(GEV_DEVICE_FORCE_IP));
			pGevDeviceForceIP->Execute();

			fExit = true;
		}
		else
		{
			cout << "New IP address is not valid." << endl;
		}
	}

}
#endif //ENABLED_ST_GUI

//-----------------------------------------------------------------------------
// Console function for reading and updating heartbeat timeout
//-----------------------------------------------------------------------------
void UpdateHeartbeatTimeout(GenApi::INodeMap *pINodeMap)
{
    std::string unit;
	bool fExit = false;

	for (;;)
	{
		// Display the current HeartbeatTimeout setting
		GenApi::CValuePtr pDeviceLinkHeartbeatTimeout(pINodeMap->GetNode(DEVICE_LINK_HEARTBEAT_TIMEOUT));
		cout.precision(12);

		if (pDeviceLinkHeartbeatTimeout.IsValid())
		{
            unit = "[us]";
		}
		else
		{
			pDeviceLinkHeartbeatTimeout = pINodeMap->GetNode(GEV_HEARTBEAT_TIMEOUT);
		    if (pDeviceLinkHeartbeatTimeout.IsValid())
            {
                unit = "[ms]";
            }
            else
            {
                cout << "Unable to get the current heartbeat value" << endl;
                fExit = true;
            }
		}
		
		if (fExit)
		{
			break;
		}

		// Waiting to enter a new HeartbeatTimeout setting.
        cout << "Warning: the heartbeat sending interval is fixed when the device is initialized (opened)." << endl;
        cout << "Thus, changing the heartbeat timeout smaller than the current value may cause timeout." << endl;
        cout << "In practical situation, please either set environment variable STGENTL_GIGE_HEARTBEAT before opening the device" << endl;
        cout << "or re-open the device after changing the heartbeat value without setting the environment variable and debugger.";

        cout << "Current Heartbeat Timeout" << unit << "=" << pDeviceLinkHeartbeatTimeout->ToString() << endl;
		cout << "Input new Heartbeat Timeout" << unit << " : ";
		GenICam::gcstring strValue;
		cin >> strValue;

		// Update the camera HeartbeatTimeout settings.
		pDeviceLinkHeartbeatTimeout->FromString(strValue);
		fExit = true;
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
IStDeviceReleasable *CreateIStDeviceByIPAddress(IStInterface *pIStInterface, const int64_t nDeviceIPAddress)
{
	pIStInterface->UpdateDeviceList();

	GenApi::CNodeMapPtr pINodeMap(pIStInterface->GetIStPort()->GetINodeMap());

	GenApi::CIntegerPtr pIIntegerDeviceSelector(pINodeMap->GetNode("DeviceSelector"));
	const int64_t nMaxIndex = pIIntegerDeviceSelector->GetMax();

	GenApi::CIntegerPtr pIntegerGevDeviceIPAddress(pINodeMap->GetNode("GevDeviceIPAddress"));
	for (int64_t i = 0; i <= nMaxIndex; ++i)
	{
		pIIntegerDeviceSelector->SetValue(i);
		if (GenApi::IsAvailable(pIntegerGevDeviceIPAddress))
		{
			if (pIntegerGevDeviceIPAddress->GetValue() == nDeviceIPAddress)
			{
				IStDeviceReleasable *pIStDeviceRelesable(pIStInterface->CreateIStDevice((size_t)i));
				return(pIStDeviceRelesable);
			}
		}
	}
	return(NULL);
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
		CIStSystemPtr pIStSystem(CreateIStSystem(StSystemVendor_Default, StInterfaceType_GigEVision));

		// Check GigE interface for devices.
		// If there is no camera, throw exception.
		IStInterface *pIStInterface = NULL;
		for (size_t i = 0; i < pIStSystem->GetInterfaceCount(); i++)
		{
			if (0 < pIStSystem->GetIStInterface(i)->GetDeviceCount())
			{
				pIStInterface = pIStSystem->GetIStInterface(i);
				break;
			}
		}
		if (pIStInterface == NULL)
		{
			throw RUNTIME_EXCEPTION("There is no device.");
		}

		// Update the IP address setting of the first camera on GigE interface.
		UpdateDeviceIPAddress(pIStInterface->GetIStPort()->GetINodeMap());

		// Get updated IP address.
		GenApi::CIntegerPtr pGevDeviceForceIPAddress(pIStInterface->GetIStPort()->GetINodeMap()->GetNode(GEV_DEVICE_FORCE_IP_ADDRESS));
		const int64_t nDeviceIPAddress = pGevDeviceForceIPAddress->GetValue();

		// Create a camera device object and connect to first detected device on GigE interface.
		// Note we use the previously created IStInterface rather than CIStSystemPtr for GigE device.
		//CIStDevicePtr pIStDevice(pIStInterface->CreateFirstIStDevice());
		CIStDevicePtr pIStDevice;
		for (size_t i = 0; i < 30; ++i)
		{
			Sleep(1000);
			IStDeviceReleasable *pIStDeviceReleasable(CreateIStDeviceByIPAddress(pIStInterface, nDeviceIPAddress));
			if (pIStDeviceReleasable != NULL)
			{
				pIStDevice.Reset(pIStDeviceReleasable);
				break;
			}
		}
		if (!pIStDevice.IsValid())
		{
			throw RUNTIME_EXCEPTION("A device with an IP address of %s could not be found.", pGevDeviceForceIPAddress->ToString().c_str());
		}

		// Display the DisplayName of the device.
		cout << "Device=" << pIStDevice->GetIStDeviceInfo()->GetDisplayName() << endl;
		
		// Update the camera HeartbeatTimeout settings.
		UpdateHeartbeatTimeout(pIStDevice->GetRemoteIStPort()->GetINodeMap());

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

				// Display the information of the acquired image data.
				cout << "BlockId=" << pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size:" << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight()
					<< " First byte =" << static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pIStImage->GetImageBuffer())) << endl;
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
	cin.ignore();
	cout << endl << "Press Enter to exit." << endl;
	while(cin.get() != '\n');

	return(0);
}
