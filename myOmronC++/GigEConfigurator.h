#pragma once

#include <StApi_TL.h>

#include "config.h"

#if defined(_WIN32_WINNT_WIN8) && (_WIN32_WINNT_WIN8 <= WINVER)
#include <WinSock2.h>	//AF_INET
#include <ws2tcpip.h>	//inet_pton
#else
#include <WinSock2.h> // For inet_addr
#endif
#pragma comment(lib, "Ws2_32.lib")

using namespace StApi;

class GigEConfigurator
{
public:
	static void UpdateDeviceIPAddress(GenApi::INodeMap* pINodeMap, uint32_t deviceIndex, const GenICam::gcstring& serialNumber);

	static void UpdateHeartbeatTimeout(GenApi::INodeMap* pINodeMap, GenICam::gcstring heartBeatTimeOut);

	static IStDeviceReleasable* CreateIStDeviceByIPAddress(IStInterface* pIStInterface, const int64_t nDeviceIPAddress);

protected:

private:
};