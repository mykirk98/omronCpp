#pragma once

#include <StApi_TL.h>
#include <unordered_map>
#include "config.h"

//#if defined(_WIN32_WINNT_WIN8) && (_WIN32_WINNT_WIN8 <= WINVER)
//#include <WinSock2.h>	//AF_INET
//#include <ws2tcpip.h>	//inet_pton
//#else
//#include <WinSock2.h> // For inet_addr
//#endif
//#pragma comment(lib, "Ws2_32.lib")
#ifdef _WIN32
#include <WinSock2.h>     // AF_INET, inet_addr
#include <ws2tcpip.h>     // inet_pton, etc.
#pragma comment(lib, "Ws2_32.lib")  // Windows 전용 링커 지시문
#else
#include <arpa/inet.h>    // inet_addr, inet_ntoa, htons, etc.
#include <netinet/in.h>   // sockaddr_in
#include <sys/socket.h>   // socket, bind, etc.
#include <unistd.h>       // close()
#include <cstring>        // memset, memcpy 등
#endif

using namespace StApi;

class GigEConfigurator
{
public:
	static void UpdateDeviceIPAddress(GenApi::INodeMap* pINodeMap, uint32_t deviceIndex, const GenICam::gcstring& serialNumber, std::string& cameraName);

	static void UpdateHeartbeatTimeout(GenApi::INodeMap* pINodeMap, GenICam::gcstring heartBeatTimeOut);

	static IStDeviceReleasable* CreateIStDeviceByIPAddress(IStInterface* pIStInterface, const int64_t nDeviceIPAddress);

protected:

private:
};