#include "GigEConfigurator.h"


void GigEConfigurator::UpdateDeviceIPAddress(GenApi::INodeMap* pINodeMap, uint32_t deviceIndex, const GenICam::gcstring& serialNumber, std::string& cameraName)
{
	// Display the IP address of the host side.
	GenApi::CIntegerPtr pGevInterfaceSubnetIPAddress(pINodeMap->GetNode(GEV_INTERFACE_SUBNET_IP_ADDRESS));
	std::cout << "Interface IP Address=" << pGevInterfaceSubnetIPAddress->ToString() << std::endl;

	// Display the subnet mask of the host side.
	GenApi::CIntegerPtr pGevInterfaceSubnetMask(pINodeMap->GetNode(GEV_INTERFACE_SUBNET_MASK));
	std::cout << "Interface Subnet Mask=" << pGevInterfaceSubnetMask->ToString() << std::endl;
	std::cout << "---------------------------------------------------------" << std::endl;

	// Select the first camera.
	GenApi::CIntegerPtr pDeviceSelector(pINodeMap->GetNode(DEVICE_SELECTOR));
	pDeviceSelector->SetValue(deviceIndex);

	// Display the current IP address of the camera.
	GenApi::CIntegerPtr pGevDeviceIPAddress(pINodeMap->GetNode(GEV_DEVICE_IP_ADDRESS));
	std::cout << "Old Device IP Address=" << pGevDeviceIPAddress->ToString() << std::endl;

	// Display the current subnet mask of the camera.
	GenApi::CIntegerPtr pGevDeviceSubnetMask(pINodeMap->GetNode(GEV_DEVICE_SUBNET_MASK));
	std::cout << "Old Device Subnet Mask=" << pGevDeviceSubnetMask->ToString() << std::endl;
	std::cout << "---------------------------------------------------------" << std::endl;

	std::string strInput;

	std::unordered_map<std::string, std::pair<std::string, std::string>> cameraMap = {
		{"25C7812", {"192.168.0.31", "12MP_1"}},
		{"25E9151", {"192.168.0.21", "12MP_2"}},
		{"25C7667", {"192.168.0.41", "5MP_1"}},
		{"25A8829", {"192.168.0.42", "5MP_2"}},
		{"25C7669", {"192.168.0.43", "5MP_3"}}
	};

	std::unordered_map<std::string, std::pair<std::string, std::string>>::iterator it = cameraMap.find(serialNumber.c_str());
	if (it != cameraMap.end())
	{
		strInput = it->second.first;
		cameraName = it->second.second;
	}
	else
	{
		std::cerr << "Unknown serial number: " << serialNumber << std::endl;
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
		std::cout << "New Device IP Address=" << pGevDeviceForceIPAddress->ToString() << std::endl;

		// Specify the new subnet mask of the camera. At this point, the camera settings will not be updated.
		GenApi::CIntegerPtr pGevDeviceForceSubnetMask(pINodeMap->GetNode(GEV_DEVICE_FORCE_SUBNET_MASK));
		pGevDeviceForceSubnetMask->SetValue(nSubnetMask);
		std::cout << "New Device Subnet Mask=" << pGevDeviceForceSubnetMask->ToString() << std::endl;
		std::cout << "---------------------------------------------------------" << std::endl;

		// Update the camera settings.
		GenApi::CCommandPtr pGevDeviceForceIP(pINodeMap->GetNode(GEV_DEVICE_FORCE_IP));
		pGevDeviceForceIP->Execute();
		std::cout << "New IP address is set." << std::endl;
	}
	else
	{
		std::cout << "New IP address is not valid." << std::endl;
	}
}

void GigEConfigurator::UpdateHeartbeatTimeout(GenApi::INodeMap* pINodeMap, GenICam::gcstring heartBeatTimeOut)
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

IStDeviceReleasable* GigEConfigurator::CreateIStDeviceByIPAddress(IStInterface* pIStInterface, const int64_t nDeviceIPAddress)
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