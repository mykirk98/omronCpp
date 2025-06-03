/*!
\file GrabChunkImage.cpp
\brief
 
 This sample shows the basic operation of using StApi and display chunk data of the received image.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to camera
 - Acquire and display chunk data.

 For more information, please refer to the help document of StApi.

*/
// Enable the following comment if you want to enable all of the chunks
//#define ENABLED_ALL_CHUNKS

// Include files for using StApi
#include <StApi_TL.h>

//Namespace for using StApi
using namespace StApi;

//Namespace for using cout
using namespace std;

//Namespace for using GenApi
using namespace GenApi;

//Count of images to be grabbed.
const uint64_t nCountOfImagesToGrab = 100;

//Feature names
const char * CHUNK_MODE_ACTIVE = "ChunkModeActive";			//Standard
const char * CHUNK_SELECTOR = "ChunkSelector";		//Standard
const char * CHUNK_ENABLE = "ChunkEnable";			//Standard

#ifdef  ENABLED_ALL_CHUNKS
#else
const char * TARGET_CHUNK_NAME = "ExposureTime";			//Standard
#endif


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DisplayNodeValues(vector<INode*> aChunkValueList)
{
	for (vector<INode*>::iterator itr = aChunkValueList.begin(); itr != aChunkValueList.end(); ++itr)
	{
		INode *pINode(*itr);
		stringstream ss;

		// Get the node name
		ss << " " << pINode->GetName();

		//Get the value of the node
		CValuePtr pValue(pINode);
		if (!IsReadable(pValue))
		{
			ss << " is not readable.";
		}
		else
		{
			ss << "=" << pValue->ToString();
		}
		ss << endl;


		cout << ss.str();
	}
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

		// Use INodeMap object to access current setting of the camera.
		CNodeMapPtr pINodeMapRemote(pIStDevice->GetRemoteIStPort()->GetINodeMap());

		// Get related INode to access and active the chunk mode.
		GenApi::CBooleanPtr pBooleanChunkModeActive(pINodeMapRemote->GetNode(CHUNK_MODE_ACTIVE));
		pBooleanChunkModeActive->SetValue(true);

		// Get the IEnumeration interface pointer to access ChunkSelector node.
		GenApi::CEnumerationPtr pEnumerationChunkSelector(pINodeMapRemote->GetNode(CHUNK_SELECTOR));

		// Get the IBoolean interface pointer to access the ChunkEnable node.
		GenApi::CBooleanPtr pBooleanChunkEnable(pINodeMapRemote->GetNode(CHUNK_ENABLE));

		// INode interface pointer list for the value of Chunk
		vector<INode*> aChunkValueList;
#ifdef ENABLED_ALL_CHUNKS
		// Get the Chunk entry list described in the XML file
		NodeList_t aChunkList;
		pEnumerationChunkSelector->GetEntries(aChunkList);
		for (NodeList_t::iterator itr = aChunkList.begin(); itr != aChunkList.end(); ++itr)
		{
			GenApi::CEnumEntryPtr pEnumEntry(*itr);
			if (IsAvailable(pEnumEntry))
			{
				//If Chunk is valid, set to the selector
				pEnumerationChunkSelector->SetIntValue(pEnumEntry->GetValue());
				if (IsWritable(pBooleanChunkEnable))
				{
					// If ChunkEnable settings that can be changed, enabled
					pBooleanChunkEnable->SetValue(true);

					// Get the node for the value of Chunk, be registered in the list
					CNodePtr pChunkValueNode(pINodeMapRemote->GetNode("Chunk" + pEnumEntry->GetSymbolic()));
					if (pChunkValueNode)
					{
						aChunkValueList.push_back(pChunkValueNode);
					}
				}
			}
		}
#else
		// Get the IEnumEntry interface pointer for the specified Chunk
		GenApi::CEnumEntryPtr pEnumEntry(pEnumerationChunkSelector->GetEntryByName(TARGET_CHUNK_NAME));
		if (IsAvailable(pEnumEntry))
		{
			//If Chunk is valid, set to the selector
			pEnumerationChunkSelector->SetIntValue(pEnumEntry->GetValue());
			if (IsWritable(pBooleanChunkEnable))
			{
				// If ChunkEnable settings that can be changed, enable it.
				pBooleanChunkEnable->SetValue(true);

				// Get the node for the value of Chunk, be registered in the list.
				GenICam::gcstring strChunkValueName("Chunk");
				strChunkValueName.append(TARGET_CHUNK_NAME);
				CNodePtr pChunkValueNode(pINodeMapRemote->GetNode(strChunkValueName));
				if (pChunkValueNode)
				{
					aChunkValueList.push_back(pChunkValueNode);
				}
			}
		}
		else
		{
			cout << TARGET_CHUNK_NAME << " is not implemented." << endl;
		}
#endif

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
					<< " Size:" << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight() << endl;

				// Get the INode interface pointer to Chunk data of acquired image buffer.
				//CNodeMapPtr pINodeMapChunk(pIStStreamBuffer->GetChunkINodeMap());

				// Display the Chunk data.
				DisplayNodeValues(aChunkValueList);
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
		std::cerr << std::endl << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
	}

	// Wait until the Enter key is pressed.
	cout << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');

	return(0);
}
