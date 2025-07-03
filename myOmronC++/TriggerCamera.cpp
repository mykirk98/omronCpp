#include "TriggerCamera.h"

#define LOGGING

TriggerCamera::TriggerCamera()
	: pICommandTriggerSoftware(nullptr)
{
#ifdef LOGGING
	std::cout << "[TriggerCamera] constructed" << std::endl;
#endif // LOGGING
}

TriggerCamera::~TriggerCamera()
{
#ifdef LOGGING
	std::cout << "[TriggerCamera] destructed" << std::endl;
#endif // LOGGING
}

bool TriggerCamera::Initialize(const CIStSystemPtr& pSystem)
{
	try
	{
		// Create a camera device object and connect to the first detected device.
		m_pDevice = pSystem->CreateFirstIStDevice();
		std::cout << "[TriggerCamera] " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << " : connected" << std::endl;
		// Get the INodeMap interface pointer for the camera settings.
		GenApi::CNodeMapPtr pINodeMap(m_pDevice->GetRemoteIStPort()->GetINodeMap());
		// Set the TriggerSelector to FrameStart.
		SetTriggerMode(pINodeMap, TRIGGER_SELECTOR_FRAME_START, TRIGGER_MODE_ON, TRIGGER_SOURCE_SOFTWARE);
		// Set the ICommand interface pointer for the TriggerSoftware node.
		pICommandTriggerSoftware = pINodeMap->GetNode(TRIGGER_SOFTWARE);
		
		// Create a DataStream object for handling image stream data.
		m_pDataStream = m_pDevice->CreateIStDataStream(0);
		std::cout << "[TriggerCamera] # of available data stream : " << m_pDevice->GetDataStreamCount() << std::endl;

		// Register a callback function. When a Data stream event is triggered, the registered function will be called.
		RegisterCallback(m_pDataStream, &TriggerCamera::OnStCallbackMethod, this);
		//NOTE: this : means the current instance of TriggerCamera, allowing the callback to access instance variables and methods.

#ifdef LOGGING
		std::cout << "[TriggerCamera] Initialized" << std::endl;
#endif // LOGGING

		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[TriggerCamera] Initialization error: " << e.GetDescription() << std::endl;
		return false;
	}
}

void TriggerCamera::OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext)
{
	if (pvContext)
	{
		// pvContext is a pointer to the TriggerCamera instance
		static_cast<TriggerCamera*>(pvContext)->OnCallback(pIStCallbackParamBase);
		//NOTE: static_cast is used here to convert the void pointer back to TriggerCamera pointer
	}
}

void TriggerCamera::OnCallback(IStCallbackParamBase* pCallbackParam)
{
	try
	{
		// Check callback type. Only NewBuffer event is handled in here
		if (pCallbackParam->GetCallbackType() == StCallbackType_GenTLEvent_DataStreamNewBuffer)
		{
			IStCallbackParamGenTLEventNewBuffer* pNewBufferParam = dynamic_cast<IStCallbackParamGenTLEventNewBuffer*>(pCallbackParam);
			//NOTE: dynamic_cast is used to safely cast the base class pointer to the derived class pointer.
			//NOTE: static_cast is used when you are sure about the type of the object, while dynamic_cast is used for safe downcasting in class hierarchies.
			
			// Get the IStDataStream interface pointer from the received callback parameter.
			IStDataStream* pDataStream = pNewBufferParam->GetIStDataStream();
			
			// Retrieve the buffer pointer of image data for that callback indicated there is a buffer received.
			CIStStreamBufferPtr pStreamBuffer(pDataStream->RetrieveBuffer(0));
			
			// Check if the acquired data contains image data.
			if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				// If yes, we create a IStImage object for further image handling.
				IStImage* pImage = pStreamBuffer->GetIStImage();
				
				uint64_t frameID = pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID();
#ifdef LOGGING
				PrintFrameInfo(pImage, pStreamBuffer);
#endif // LOGGING
			}
			else
			{
				std::cout << "[TriggerCamera] No image present in the buffer." << std::endl;
			}
		}
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[TriggerCamera] Callback Exception: " << e.GetDescription() << std::endl;
	}
}

void TriggerCamera::SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName)
{
	try
	{
		// Get the IEnumeration interface pointer for the specified enumeration name.
		GenApi::CEnumerationPtr pIEnumeration(pInodeMap->GetNode(szEnumerationName));

		// Get the IEnumEntry interface pointer for the specified value name.
		GenApi::CEnumEntryPtr pIEnumEntry(pIEnumeration->GetEntryByName(szValueName));

		// Get the integer value corresponding to the set value name using the IEnumEntry interface pointer.
		// Update the settings using the IEnumeration interface pointer.
		pIEnumeration->SetIntValue(pIEnumEntry->GetValue());
#ifdef LOGGING
		std::cout << "[TriggerCamera] Set " << szEnumerationName << " to " << szValueName << std::endl;
#endif // LOGGING
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[TriggerCamera] Setting enumeration failed: " << e.GetDescription() << std::endl;
	}
}

void TriggerCamera::SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource)
{
	try
	{
		// Set the TriggerSelector to FrameStart.
		SetEnumeration(pINodeMap, TRIGGER_SELECTOR, triggerSelector);
		// Set the TriggerMode to On.
		SetEnumeration(pINodeMap, TRIGGER_MODE, triggerMode);
		// Set the TriggerSource to Software.
		SetEnumeration(pINodeMap, TRIGGER_SOURCE, triggerSource);
#ifdef LOGGING
		std::cout << "[TriggerCamera] Trigger mode set to " << triggerMode << " with source " << triggerSource << std::endl;
#endif // LOGGING
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[TriggerCamera] Setting trigger mode failed: " << e.GetDescription() << std::endl;
	}
}

// Example usage of TriggerCamera class
/*
#include "TriggerCamera.h"

int main()
{
	std::cout << "==========Trigger Camera Example==========" << std::endl;
	CStApiAutoInit objStApiAutoInit;
	CIStSystemPtr pSystem(CreateIStSystem());

	TriggerCamera cameraWorker;
	if (cameraWorker.Initialize(pSystem))
	{
		cameraWorker.StartAcquisition();

		std::cout << "0: Generate trigger" << std::endl;
		std::cout << "Else: Exit" << std::endl;
		std::cout << "Select: ";

		while (true)
		{
			size_t nindex;
			std::cin >> nindex;
			if (nindex == 0)
			{
				cameraWorker.pICommandTriggerSoftware->Execute();
			}
			else
			{
				break;
			}
		}
		cameraWorker.StopAcquisition();
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}
*/