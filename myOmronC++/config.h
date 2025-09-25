#pragma once
#include <unordered_map>
#include <string>



//----------------------------------------CAMERA CONFIGURATION---------------------------------------//
//				data standardization
//				{"camera serial number", {"camera IP address", "user defined(UDF) camera name"}}
const std::unordered_map<std::string, std::pair<std::string, std::string>> cameraMap = {
	{"25G8727", {"192.168.5.15", "rearcover_top_camera"}},
	{"25J7712", {"192.168.6.16", "rearcover_top_camera"}},
	{"25A9284", {"192.168.7.17", "endoscope_camera"}},
	{"25J7723", {"192.168.5.18", "sleeve_A_camera"}},
	//{"25J7712", {"192.168.0.16", "Endoscope_Side_Camera"}},
	//{"25A9284", {"192.168.0.17", "Endoscope_Robot_Endoscope_Camera"}},
	//{"25G8727", {"192.168.0.15", "Endoscope_Robot_Camera"}},
	//{"25J7723", {"192.168.0.18", "Sleeve_A_Camera"}},
	{"25C7812", {"192.168.0.31", "12MP_1"}},
	{"25E9151", {"192.168.0.21", "12MP_2"}},
	{"25C7667", {"192.168.0.41", "5MP_1"}},
	{"25A8829", {"192.168.0.42", "5MP_2"}},
	{"25C7669", {"192.168.0.43", "5MP_3"}},
	{"25AA120", {"192.168.0.44", "5MP_4"}},
	{"23F8382", {"192.168.0.51", "2MP_1"}},
	{"25A9284", {"192.168.0.52", "2MP_2"}}
};
//---------------------------------------------------------------------------------------------------//





//----------------------------DIRECTORY PATH WHICH YOU WANT TO SAVE IMAGES---------------------------//
constexpr const char* HOME_PC_DIRECTORY = "C:\\Users\\USER\\Pictures\\";
constexpr const char* LAB_WINDOW_PC_DIRECTORY = "C:\\Users\\mykir\\Work\\Experiments\\";
constexpr const char* LAB_LINUX_PC_DIRECTORY = "/home/msis/Pictures/SentechExperiments/Experiments1/";
constexpr const char* DEVELOPMENT_PC_DIRECTORY = "home/msis/Pictures/dataset/";
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