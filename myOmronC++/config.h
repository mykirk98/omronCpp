#pragma once
#include <unordered_map>
#include <string>



//----------------------------------------CAMERA CONFIGURATION---------------------------------------//
const std::unordered_map<std::string, std::pair<std::string, std::string>> cameraMap = {
//	{"camera serial number",	{"camera IP address",	"user defined(UDF) camera name"}}
	{"25J7712",					{"192.168.0.16",		"Endoscope_Side_Camera"}},		// LINE-B
	{"25A9284",					{"192.168.0.17",		"Endoscope_Robot_Endoscope_Camera"}},	// LINE-B
	{"25G8727",					{"192.168.0.15",		"Endoscope_Robot_Camera"}},		// LINE-B
	{"25J7723",					{"192.168.0.18",		"Sleeve_A_Camera"}},		// LINE-B
	//TODO: Add Sleeve_B_Camera later
};

const std::unordered_map<std::string, std::string> cameraConfigMap = {
//	{"user defined(UDF) camera name",		"camera configuration file path"}
	{"Endoscope_Side_Camera",				"/home/msis/cameraConfigs/NodeMaps/Endoscope_Side_Camera_config.cfg"},		// LINE-B
	{"Endoscope_Robot_Endoscope_Camera",	"/home/msis/cameraConfigs/NodeMaps/Endoscope_Robot_Endoscope_Camera_config.cfg"},	// LINE-B
	{"Endoscope_Robot_Camera",				"/home/msis/cameraConfigs/NodeMaps/Endoscope_Robot_Camera_config.cfg"},		// LINE-B
	{"Sleeve_A_Camera",						"/home/msis/cameraConfigs/NodeMaps/Sleeve_A_Camera_config.cfg"},		// LINE-B
	//TODO: Add Sleeve_B_Camera later
};

const std::unordered_map <std::string, std::pair<bool, bool>> cameraQueueMap = {
//	{"user defined(UDF) camera name",		{	enable saving image,	enable forwarding cvMat	}}
	{"Endoscope_Side_Camera",				{	true,					true	}},		// LINE-B
	{"Endoscope_Robot_Endoscope_Camera",	{	true,					true	}},		// LINE-B
	{"Endoscope_Robot_Camera",				{	true,					true	}},		// LINE-B
	{"Sleeve_A_Camera",						{	true,					true	}},		// LINE-B
	//TODO: Add Sleeve_B_Camera later
};

constexpr int IMAGE_SAVER_THREAD_POOL_SIZE = 2;	// 1 ~ 6
//---------------------------------------------------------------------------------------------------//





//----------------------------------------LIGHT CONFIGURATION----------------------------------------//
// rear cover top light
constexpr int REARCOVER_TOP_LIGHT_BRIGHTNESS = 120;		// 0 ~ 240
constexpr double REARCOVOER_TOP_LIGHT_STROBE_MS = 9.99;	// 0.01 ~ 9.99 ms
// rear cover side light
constexpr int REARCOVER_SIDE_LIGHT_BRIGHTNESS = 120;		// 0 ~ 240
constexpr double REARCOVER_SIDE_LIGHT_STROBE_MS = 9.99;	// 0.01 ~ 9.99 ms
// sleeve A light
constexpr int SLEEVE_A_LIGHT_BRIGHTNESS = 50;			// 0 ~ 100
constexpr double SLEEVE_A_LIGHT_STROBE_MS = 9.99;		// 0.01 ~ 9.99 ms
// sleeve B light
constexpr int SLEEVE_B_LIGHT_BRIGHTNESS = 50;			// 0 ~ 100
constexpr double SLEEVE_B_LIGHT_STROBE_MS = 9.99;		// 0.01 ~ 9.99 ms
//---------------------------------------------------------------------------------------------------//





//----------------------------DIRECTORY PATH WHICH YOU WANT TO SAVE IMAGES---------------------------//
constexpr const char* DEVELOPMENT_PC_DIRECTORY = "home/msis/Pictures/dataset/";
constexpr const char* LINE_A_SERVER = "home/msis/Pictures/dataset/";
constexpr const char* LINE_B_SERVER = "home/msis/Pictures/dataset/";
//---------------------------------------------------------------------------------------------------//
































//---------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------//
//-----------------------------------WARNING : DO NOT CHANGE BELOW-----------------------------------//
#define ENABLED_ST_GUI
constexpr const char* TRIGGER_SELECTOR = "TriggerSelector";
constexpr const char* TRIGGER_SELECTOR_FRAME_START = "FrameStart";
constexpr const char* TRIGGER_SELECTOR_EXPOSURE_START = "ExposureStart";
constexpr const char* TRIGGER_SELECTOR_EXPOSURE_END = "ExposureEnd";
constexpr const char* TRIGGER_MODE = "TriggerMode";
constexpr const char* TRIGGER_MODE_ON = "On";
constexpr const char* TRIGGER_MODE_OFF = "Off";
constexpr const char* TRIGGER_SOURCE = "TriggerSource";
constexpr const char* TRIGGER_SOURCE_SOFTWARE = "Software";
constexpr const char* TRIGGER_SOFTWARE = "TriggerSoftware";

constexpr const char* GEV_INTERFACE_SUBNET_IP_ADDRESS = "GevInterfaceSubnetIPAddress";
constexpr const char* GEV_INTERFACE_SUBNET_MASK = "GevInterfaceSubnetMask";

constexpr const char* DEVICE_SELECTOR = "DeviceSelector";
constexpr const char* GEV_DEVICE_IP_ADDRESS = "GevDeviceIPAddress";
constexpr const char* GEV_DEVICE_SUBNET_MASK = "GevDeviceSubnetMask";

constexpr const char* GEV_DEVICE_FORCE_IP_ADDRESS = "GevDeviceForceIPAddress";
constexpr const char* GEV_DEVICE_FORCE_SUBNET_MASK = "GevDeviceForceSubnetMask";
constexpr const char* GEV_DEVICE_FORCE_IP = "GevDeviceForceIP";
constexpr const char* DEVICE_LINK_HEARTBEAT_TIMEOUT = "DeviceLinkHeartbeatTimeout";
constexpr const char* GEV_HEARTBEAT_TIMEOUT = "GevHeartbeatTimeout";