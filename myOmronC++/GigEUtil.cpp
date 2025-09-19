#include "GigEUtil.h"

void GigEUtil::UpdateDeviceIPAddress(IStInterface* pInterface, uint32_t deviceIndex, std::string& userDefinedName, std::shared_ptr<CamLogger> logger)
{
	GenApi::CNodeMapPtr pINodeMap(pInterface->GetIStPort()->GetINodeMap());
	std::string serialNumber = pInterface->GetIStDeviceInfo(deviceIndex)->GetSerialNumber().c_str();

	// Display the IP address of the host side.
	GenApi::CIntegerPtr pGevInterfaceSubnetIPAddress(pINodeMap->GetNode(GEV_INTERFACE_SUBNET_IP_ADDRESS));
	logger->Log("Interface IP Address : " + std::string(pGevInterfaceSubnetIPAddress->ToString()));

	// Display the subnet mask of the host side.
	GenApi::CIntegerPtr pGevInterfaceSubnetMask(pINodeMap->GetNode(GEV_INTERFACE_SUBNET_MASK));
	logger->Log("Interface Subnet Mask : " + std::string(pGevInterfaceSubnetMask->ToString()) + "\n");

	// Select the first camera.
	GenApi::CIntegerPtr pDeviceSelector(pINodeMap->GetNode(DEVICE_SELECTOR));
	pDeviceSelector->SetValue(deviceIndex);

	// Display the current IP address of the camera.
	GenApi::CIntegerPtr pGevDeviceIPAddress(pINodeMap->GetNode(GEV_DEVICE_IP_ADDRESS));
	logger->Log("Old Device IP Address : " + std::string(pGevDeviceIPAddress->ToString()));

	// Display the current subnet mask of the camera.
	GenApi::CIntegerPtr pGevDeviceSubnetMask(pINodeMap->GetNode(GEV_DEVICE_SUBNET_MASK));
	logger->Log("Old Device Subnet Mask : " + std::string(pGevDeviceSubnetMask->ToString()) + "\n");

	std::string strInput;

	std::unordered_map<std::string, std::pair<std::string, std::string>> cameraMap = {
		{"24K9080", {"192.168.6.16", "Endoscope_Side_Camera"}},
		{"23F8383", {"192.168.7.17", "Endoscope_Robot_Endoscope_Camera"}},
		{"25E9152", {"192.168.5.15", "Endoscope_Robot_Camera"}},
		{"25C7812", {"192.168.0.31", "12MP_1"}},
		{"25E9151", {"192.168.0.21", "12MP_2"}},
		{"25C7667", {"192.168.0.41", "5MP_1"}},
		{"25A8829", {"192.168.0.42", "5MP_2"}},
		{"25C7669", {"192.168.0.43", "5MP_3"}},
		{"25AA120", {"192.168.0.44", "5MP_4"}},
		{"23F8382", {"192.168.0.51", "2MP_1"}},
		{"25A9284", {"192.168.0.52", "2MP_2"}}
	};

	std::unordered_map<std::string, std::pair<std::string, std::string>>::iterator it = cameraMap.find(serialNumber.c_str());
	if (it != cameraMap.end())
	{
		strInput = it->second.first;
		userDefinedName = it->second.second;
	}
	else
	{
		logger->Log("Unknown serial number : " + std::string(serialNumber));
	}

	// Convert the new IP address string to a 32-bit number.
#ifdef _WIN32
	uint32_t nNewDeviceIPAddress;
	if (!inet_pton(AF_INET, strInput.c_str(), &nNewDeviceIPAddress))
	{
		nNewDeviceIPAddress = 0;
	}
	else
	{
		nNewDeviceIPAddress = ntohl(nNewDeviceIPAddress);
	}
#else
	// POSIX 시스템 (Linux 등)에서는 inet_addr() + ntohl() 사용
	uint32_t nNewDeviceIPAddress = ntohl(inet_addr(strInput.c_str()));
#endif


	// Get the subnet mask of the host side.
	const uint32_t nSubnetMask = (uint32_t)pGevInterfaceSubnetMask->GetValue();

	// Get the IP address of the host side.
	const uint32_t nInterfaceIPAddress = (uint32_t)pGevInterfaceSubnetIPAddress->GetValue();

	// Ensure that the subnet address of the host and the camera are matched, and that the host and IP address of camera are different.
	if (((nInterfaceIPAddress & nSubnetMask) == (nNewDeviceIPAddress & nSubnetMask)) && (nInterfaceIPAddress != nNewDeviceIPAddress))
	{
		// Specify the new IP address of the camera. At this point, the camera settings will not be updated.
		GenApi::CIntegerPtr pGevDeviceForceIPAddress(pINodeMap->GetNode(GEV_DEVICE_FORCE_IP_ADDRESS));
		pGevDeviceForceIPAddress->SetValue(nNewDeviceIPAddress);
		logger->Log("New Device IP Address : " + std::string(pGevDeviceForceIPAddress->ToString()));

		// Specify the new subnet mask of the camera. At this point, the camera settings will not be updated.
		GenApi::CIntegerPtr pGevDeviceForceSubnetMask(pINodeMap->GetNode(GEV_DEVICE_FORCE_SUBNET_MASK));
		pGevDeviceForceSubnetMask->SetValue(nSubnetMask);
		logger->Log("New Device Subnet Mask : " + std::string(pGevDeviceForceSubnetMask->ToString()) + "\n");

		// Update the camera settings.
		GenApi::CCommandPtr pGevDeviceForceIP(pINodeMap->GetNode(GEV_DEVICE_FORCE_IP));
		pGevDeviceForceIP->Execute();
	}
	else
	{
		logger->Log("New IP address is not valid for camera : " + userDefinedName);
	}
}

void GigEUtil::UpdateHeartbeatTimeout(GenApi::INodeMap* pINodeMap, GenICam::gcstring heartBeatTimeOut)
{
	std::string unit;

	GenApi::CValuePtr pDeviceLinkHeartbeatTimeout(pINodeMap->GetNode(DEVICE_LINK_HEARTBEAT_TIMEOUT));
	std::cout.precision(12);

	if (pDeviceLinkHeartbeatTimeout.IsValid())
	{
		unit = "[us]";
	}
	else
	{
		pDeviceLinkHeartbeatTimeout = pINodeMap->GetNode(GEV_HEARTBEAT_TIMEOUT);
		if (pDeviceLinkHeartbeatTimeout.IsValid())
		{
			unit = "[ms]";
		}
		else
		{
			std::cout << "Unable to get the current heartbeat value" << std::endl;
		}
	}

	// Waiting to enter a new HeartbeatTimeout setting.
	//std::cout << "Warning: the heartbeat sending interval is fixed when the device is initialized (opened)." << std::endl;
	//std::cout << "Thus, changing the heartbeat timeout smaller than the current value may cause timeout." << std::endl;
	//std::cout << "In practical situation, please either set environment variable STGENTL_GIGE_HEARTBEAT before opening the device" << std::endl;
	//std::cout << "or re-open the device after changing the heartbeat value without setting the environment variable and debugger.";

	std::cout << "Current Heartbeat Timeout" << unit << "=" << pDeviceLinkHeartbeatTimeout->ToString() << std::endl;

	// Update the camera HeartbeatTimeout settings.
	pDeviceLinkHeartbeatTimeout->FromString(heartBeatTimeOut);
	std::cout << "New Heartbeat Timeout" << unit << "=" << pDeviceLinkHeartbeatTimeout->ToString() << std::endl;
}

IStDeviceReleasable* GigEUtil::CreateIStDeviceByIPAddress(IStInterface* pIStInterface, const int64_t nDeviceIPAddress)
{
	pIStInterface->UpdateDeviceList();

	GenApi::CNodeMapPtr pINodeMap(pIStInterface->GetIStPort()->GetINodeMap());

	GenApi::CIntegerPtr pIIntegerDeviceSelector(pINodeMap->GetNode("DeviceSelector"));
	const int64_t nMaxIndex = pIIntegerDeviceSelector->GetMax();

	GenApi::CIntegerPtr pIntegerGevDeviceIPAddress(pINodeMap->GetNode("GevDeviceIPAddress"));
	for (int64_t i = 0; i <= nMaxIndex; ++i)
	{
		pIIntegerDeviceSelector->SetValue(i);
		if (GenApi::IsAvailable(pIntegerGevDeviceIPAddress))
		{
			if (pIntegerGevDeviceIPAddress->GetValue() == nDeviceIPAddress)
			{
				IStDeviceReleasable* pIStDeviceRelesable(pIStInterface->CreateIStDevice((size_t)i));
				return(pIStDeviceRelesable);
			}
		}
	}
	return(NULL);
}