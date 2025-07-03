#pragma once
#include "BasicCamera.h"
#include "config.h"

/*
@brief camera worker with callback class
@brief This class handles camera operations using callbacks for image acquisition and processing.
*/
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

	/* @brief Command interface pointer for software trigger */
	GenApi::CCommandPtr pICommandTriggerSoftware;
	
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
};