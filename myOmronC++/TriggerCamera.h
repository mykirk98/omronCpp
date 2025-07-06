#pragma once
#include "BasicCamera.h"
#include <mutex>
#include <condition_variable>
#include <atomic>

/* @brief This class extends BasicCamera to implement software trigger functionality. */
class TriggerCamera : public BasicCamera
{
public:
	/* @brief TriggerCamera constructor */
	TriggerCamera();
	/* @brief TriggerCamera destructor */
	~TriggerCamera();

	/*
	@brief camera initialization method
	@param pSystem : CIStSystemPtr object representing the camera system
	*/
	bool Initialize(const CIStSystemPtr& pSystem);
	/* @brief Start image acquisition method */
	void StartAcquisition();
	/* @brief Stop image acquisition method */
	void StopAcquisition();
	/*
	@brief Execute software trigger and wait for image capture
	@param timeoutMs : Timeout in milliseconds to wait for the image capture
	*/
	bool TriggerAndWait(int timeoutMs = 1000);

private:
	/*
	@brief Callback method for handling StApi events, static method to be used with RegisterCallback
	@param pIStCallbackParamBase : Pointer to the callback parameter base
	@param pvContext : User-defined context pointer
	*/
	static void OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext);
	/*
	@brief Callback method for handling StApi events
	@param pCallbackParam : Pointer to the callback parameter base
	*/
	void OnCallback(IStCallbackParamBase* pCallbackParam);
	/*
	@brief Set enumeration value in the INodeMap
	@param pInodeMap : INodeMap pointer to the camera settings
	@param szEnumerationName : Name of the enumeration to set
	@param szValueName : Name of the value to set in the enumeration
	*/
	void SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName);
	/*
	@brief Set trigger mode in the INodeMap
	@param pINodeMap : INodeMap pointer to the camera settings
	@param triggerSelector : Trigger selector to set
	@param triggerMode : Trigger mode to set
	@param triggerSource : Trigger source to set
	*/
	void SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource);

	/* @brief Pointer to the GenICam command node for executing a software trigger */
	GenApi::CCommandPtr pICommandTriggerSoftware;
	/* @brief Mutex to ensure thread safety when accessing shared resources (e.g., image capture flag) */
	std::mutex m_mutex;
	/* @brief Condition variable used to wait for and notify image capture events between threads */
	std::condition_variable m_cv;
	/* @brief Atomic flag indicating whether an image has been captured (used for thread-safe status checks) */
	std::atomic<bool> m_imageCaptured;
};