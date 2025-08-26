#pragma once
#include "config.h"

#include <StApi_TL.h>
#ifdef ENABLED_ST_GUI
#include <StApi_GUI.h>
#endif // ENABLED_ST_GUI

using namespace StApi;
using namespace GenApi;

class NodeMapUtil
{
public:
	
	/* @brief Display all nodes in the node map
	@param pINode : Pointer to the node map */
	static void DisplayNodes(GenApi::CNodePtr pINode);
	/* @brief Save the current state of the node map to a directory
	@param device : Pointer to the device whose node map is to be saved
	@param dstDir : Directory where the node map will be saved */
	static void Save(const CIStDevicePtr& device, const std::string& dstDir);
	/* @brief Load the node map from a directory
	@param device : Pointer to the device whose node map is to be loaded
	@param srcDir : Directory from which the node map will be loaded */
	static void Load(const CIStDevicePtr& device, const std::string& srcDir);
	/* @brief Set the value of a node in the node map
	@param pInodeMap : Pointer to the node map
	@param szEnumerationName : Name of the enumeration node
	@param szValueName : Name of the value to be set */
	static void SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName);
	/* @brief Set the trigger mode for the camera
	@param pINodeMap : Pointer to the node map
	@param triggerSelector : Selector for the trigger type (e.g., FrameStart)
	@param triggerMode : Mode of the trigger (e.g., On, Off)
	@param triggerSource : Source of the trigger (e.g., Software, Action0, Line0) */
	static void SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource);
protected:

private:

};

