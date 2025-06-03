/*!
\file UserMemory.cpp
\brief

This sample shows how to use UserMemory function.
The following points will be demonstrated in this sample code:
- Initialize StApi
- Connect to camera
- How to read user data from the rom and to write user data to the rom.

For more information, please refer to the help document of StApi.
*/
#include <iomanip>	//setfill, setw
// Include files for using StApi.
#include <StApi_TL.h>

//Namespace for using GenTL.
using namespace GenTL;

//Namespace for using StApi.
using namespace StApi;

//Namespace for using cout
using namespace std;

//Namespace for using GenApi.
using namespace GenApi;

//Feature names
const char * DEVICE_USER_MEMORY = "DeviceUserMemory";								//Custom

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void PrintHexData(uint8_t *pbyteData, int64_t nLength, int64_t nAddOffset = 0)
{
	for (int64_t i = 0; i < nLength; i++)
	{
		if ((i & 0xF) == 0)
		{
			const int64_t nAddress = i + nAddOffset;
			cout << hex << uppercase << setfill('0') << setw(4) << nAddress << '\t';
		}

		cout << hex << uppercase << setfill('0') << setw(2) << (int)pbyteData[i] << ' ';

		if ((i & 0xF) == 0xF)
		{
			cout << endl;
		}
	}
	cout << endl;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DisplayRegister(IRegister *pIRegister)
{
	try
	{
		const int64_t nLength = pIRegister->GetLength();
		uint8_t *pbyteBuffer = new uint8_t[static_cast<size_t>(nLength)];
		pIRegister->Get(pbyteBuffer, nLength);
		PrintHexData(pbyteBuffer, nLength);
		delete[] pbyteBuffer;
	}
	catch (std::bad_alloc &)
	{
		throw BAD_ALLOC_EXCEPTION();
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int main(int /* argc */, char ** /* argv */)
{

	try
	{
		// Before using the library, perform the initialization. 
		// StApiInitialize() is called by the constructor of CStApiAutoInit. StApiTerminate() is called by the destructor.
		CStApiAutoInit objStApiAutoInit;

		// Create a system object, to get the IStSystemReleasable interface pointer.
		// After the system object is no longer needed, call the IStSystemReleasable::Release(), please discard the system object.
		// In the destructor of CIStSystemPtr, IStSystemReleasable::Release() is called.
		CIStSystemPtr pIStSystem(CreateIStSystem());

		// Create a device object of the first occurrence of the device to get the IStDeviceReleasable interface pointer.
		// After the device object is no longer needed, call the IStDeviceReleasable::Release(), please discard the device object.
		// In the destructor of CIStDevicePtr, IStDeviceReleasable::Release() is called.
		CIStDevicePtr pIStDevice(pIStSystem->CreateFirstIStDevice(DEVICE_ACCESS_EXCLUSIVE));

		// Displays the DisplayName of the device.
		cout << "Device=" << pIStDevice->GetIStDeviceInfo()->GetDisplayName() << endl;

		// Get the INodeMap interface pointer for the camera settings.
		CNodeMapPtr pINodeMapRemote(pIStDevice->GetRemoteIStPort()->GetINodeMap());

		//
		CRegisterPtr pIRegisterUserMemory(pINodeMapRemote->GetNode(DEVICE_USER_MEMORY));
		if(!pIRegisterUserMemory.IsValid())
		{
			cout << DEVICE_USER_MEMORY << " is not supported by this camera."<< endl;
		}
		else
		{
			DisplayRegister(pIRegisterUserMemory);
		}
	}
    catch (const GenICam::GenericException &e)
	{
		// Display a description of the error.
		cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
    }

	cin.ignore();
	// Wait until the Enter key is pressed.
	cout << endl << "Press Enter to exit." << endl;
	while(cin.get() != '\n');

	return(0);
}
