#pragma once

//#define ENABLED_ST_GUI

constexpr const char* TRIGGER_SELECTOR = "TriggerSelector";					//Standard
constexpr const char* TRIGGER_SELECTOR_FRAME_START = "FrameStart";			//Standard
constexpr const char* TRIGGER_SELECTOR_EXPOSURE_START = "ExposureStart";	//Standard
constexpr const char* TRIGGER_SELECTOR_EXPOSURE_END = "ExposureEnd";		//Standard
constexpr const char* TRIGGER_MODE = "TriggerMode";							//Standard
constexpr const char* TRIGGER_MODE_ON = "On";								//Standard
constexpr const char* TRIGGER_MODE_OFF = "Off";								//Standard
constexpr const char* TRIGGER_SOURCE = "TriggerSource";						//Standard
constexpr const char* TRIGGER_SOURCE_SOFTWARE = "Software";					//Standard
constexpr const char* TRIGGER_SOFTWARE = "TriggerSoftware";					//Standard

constexpr const char* GEV_INTERFACE_SUBNET_IP_ADDRESS = "GevInterfaceSubnetIPAddress";	//Standard
constexpr const char* GEV_INTERFACE_SUBNET_MASK = "GevInterfaceSubnetMask";				//Standard

constexpr const char* DEVICE_SELECTOR = "DeviceSelector";								//Standard
constexpr const char* GEV_DEVICE_IP_ADDRESS = "GevDeviceIPAddress";						//Standard
constexpr const char* GEV_DEVICE_SUBNET_MASK = "GevDeviceSubnetMask";					//Standard

constexpr const char* GEV_DEVICE_FORCE_IP_ADDRESS = "GevDeviceForceIPAddress";	//Standard
constexpr const char* GEV_DEVICE_FORCE_SUBNET_MASK = "GevDeviceForceSubnetMask";	//Standard
constexpr const char* GEV_DEVICE_FORCE_IP = "GevDeviceForceIP";							//Standard
constexpr const char* DEVICE_LINK_HEARTBEAT_TIMEOUT = "DeviceLinkHeartbeatTimeout";		//Standard[us]
constexpr const char* GEV_HEARTBEAT_TIMEOUT = "GevHeartbeatTimeout";						//Standard(Deprecated)[ms]