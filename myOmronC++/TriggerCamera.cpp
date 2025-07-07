#include "TriggerCamera.h"

//#define LOGGING
//#define SYNC_LOGGING

TriggerCamera::TriggerCamera()
	: pICommandTriggerSoftware(nullptr)
	, m_imageCaptured(false)
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
#ifdef LOGGING
		std::cout << "[TriggerCamera] " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << " : connected" << std::endl;
#endif // LOGGING
		// Get the INodeMap interface pointer for the camera settings.
		GenApi::CNodeMapPtr pINodeMap(m_pDevice->GetRemoteIStPort()->GetINodeMap());
		// Set the TriggerSelector to FrameStart.
		SetTriggerMode(pINodeMap, TRIGGER_SELECTOR_FRAME_START, TRIGGER_MODE_ON, TRIGGER_SOURCE_SOFTWARE);
		// Set the ICommand interface pointer for the TriggerSoftware node.
		pICommandTriggerSoftware = pINodeMap->GetNode(TRIGGER_SOFTWARE);
		
		// Create a DataStream object for handling image stream data.
		m_pDataStream = m_pDevice->CreateIStDataStream(0);
#ifdef LOGGING
		std::cout << "[TriggerCamera] # of available data stream : " << m_pDevice->GetDataStreamCount() << std::endl;
#endif // LOGGING
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

void TriggerCamera::StartAcquisition()
{
	try
	{
		// Start the image acquisition of the host(PC) side.
		m_pDataStream->StartAcquisition();
		// Start the image acquisition of the camera side.
		m_pDevice->AcquisitionStart();

#ifdef LOGGING
		std::cout << "[TriggerCamera] Acquisition started" << std::endl;
#endif // LOGGING
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[TriggerCamera] Start acquisition error: " << e.GetDescription() << std::endl;
	}
}

void TriggerCamera::StopAcquisition()
{
	try
	{
		// Stop the image acquisition of the camera side.
		m_pDevice->AcquisitionStop();
		// Stop the image acquisition of the host(PC) side.
		m_pDataStream->StopAcquisition();
#ifdef LOGGING
		std::cout << "[TriggerCamera] Acquisition stopped" << std::endl;
#endif // LOGGING

	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[TriggerCamera] Stop acqiusition error: " << e.GetDescription() << std::endl;
	}
}

bool TriggerCamera::TriggerAndWait(int timeoutMs)
{
	{	//NOTE: <--Critical section to ensure thread safety
		// Lock the mutex to ensure thread safety when accessing shared resources
#ifdef SYNC_LOGGING
		std::cout << "[TriggerCamera] Locking mutex and execute the trigger" << std::endl;
#endif // SYNC_LOGGING
		std::lock_guard<std::mutex> lock(m_mutex);
		// Reset the image captured flag to false before issuing a new trigger
		m_imageCaptured = false;
	}	//NOTE: <--end of critical section, mutex is automatically released

	// Issue a software trigger command to the camera
	std::chrono::steady_clock::time_point triggerTime = std::chrono::steady_clock::now();
	// print trigger time
	std::cout << "[TriggerCamera] Trigger time: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(triggerTime.time_since_epoch()).count()
		<< " ms" << std::endl;
	pICommandTriggerSoftware->Execute();
#ifdef SYNC_LOGGING
	std::cout << "[TriggerCamera] Trigger executed" << std::endl;
#endif // SYNC_LOGGING

	// Wait for the image to be captured or timeout
	std::unique_lock<std::mutex> lock(m_mutex);
#ifdef SYNC_LOGGING
	std::cout << "[TriggerCamera] Waiting for image capture with timeout: " << timeoutMs << " ms" << std::endl;
#endif // SYNC_LOGGING
	// Use condition variable to wait for the image to be captured or timeout
	bool success = m_cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() {
		return m_imageCaptured.load();	// Check if the image has been captured
		});
#ifdef SYNC_LOGGING
	std::cout << "[TriggerCamera] Wait completed" << std::endl;
#endif // SYNC_LOGGING
	//NOTE: if m_imageCaptured is true, it means the image has been captured successfully
	//NOTE: and the condition variable will notify the waiting thread to wake up.

	// true = image captured, false = timeout occurred
	return success;
}

void TriggerCamera::SetFrameQueue(std::shared_ptr<FrameQueue> queue)
{
	m_pFrameQueue = queue;
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
			std::chrono::steady_clock::time_point callbackTime = std::chrono::steady_clock::now();
			// print callback time
			std::cout << "[TriggerCamera] Callback received at: " 
				<< std::chrono::duration_cast<std::chrono::milliseconds>(callbackTime.time_since_epoch()).count() 
				<< " ms since epoch" << std::endl;
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

				if (m_pFrameQueue)
				{
					m_pFrameQueue->Push(pImage);
					std::cout << "[TriggerCamera] Image pushed. Queue size: " << m_pFrameQueue->Size() << std::endl;
				}
				std::chrono::steady_clock::time_point captureTime = std::chrono::steady_clock::now();
				// print capture time
				std::cout << "[TriggerCamera] Image captured at: " 
					<< std::chrono::duration_cast<std::chrono::milliseconds>(captureTime.time_since_epoch()).count() 
					<< " ms since epoch" << std::endl;
				//PrintFrameInfo(pStreamBuffer);

				
				{	//NOTE: <--Critical section to ensure thread safety
#ifdef SYNC_LOGGING
					std::cout << "[TriggerCamera] Image captured" << std::endl;
#endif // SYNC_LOGGING
					std::lock_guard<std::mutex> lock(m_mutex);
					m_imageCaptured = true;
#ifdef SYNC_LOGGING
					std::cout << "[TriggerCamera] Image captured flag set to true," 
						<< "notifying condition variable" << std::endl;
#endif // SYNC_LOGGING
				}	//NOTE: <--end of critical section, mutex is automatically released
				m_cv.notify_one();

#ifdef LOGGING
				std::cout << "[TriggerCamera] respond trigger" << std::endl;
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
#include <chrono>

int main()
{
	std::cout << "========== Trigger Camera with Wait Example ==========" << std::endl;

	CStApiAutoInit stApiAutoInit;
	CIStSystemPtr pSystem(CreateIStSystem());

	TriggerCamera camera;
	if (camera.Initialize(pSystem))
	{
		camera.StartAcquisition();
		int imageCount = 500;
		// calculate average FPS
		std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

		for (int i = 0; i < imageCount; ++i)
		{
			std::cout << "[Main] Triggering " << i << std::endl;
			if (camera.TriggerAndWait(100))
				std::cout << "[Main] Frame " << i << " captured." << std::endl;
			else
				std::cerr << "[Main] Frame " << i << " timed out." << std::endl;
		}

		std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
		double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
		double averageFPS = float(imageCount) / (elapsedTime / 1000.0); // 1000 frames
		std::cout << "[Main] Average FPS: " << averageFPS << std::endl;	// 65.6537

		camera.StopAcquisition();
	}

	return 0;
}
*/