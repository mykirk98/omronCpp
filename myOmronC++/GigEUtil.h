#pragma once

#include <StApi_TL.h>
#include <unordered_map>
#include "config.h"
#include "CamLogger.h"

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
#pragma comment(lib, "Ws2_32.lib")  // Windows Sockets library
#else
#include <arpa/inet.h>    // inet_addr, inet_ntoa, htons, etc.
#include <netinet/in.h>   // sockaddr_in
#include <sys/socket.h>   // socket, bind, etc.
#include <unistd.h>       // close()
#include <cstring>        // memset, memcpy etc.
#endif

using namespace StApi;

class GigEUtil
{
public:
	static void UpdateDeviceIPAddress(IStInterface* pInterface, uint32_t deviceIndex, std::string& userDefinedName, std::shared_ptr<CamLogger> logger);

	static void UpdateHeartbeatTimeout(GenApi::INodeMap* pINodeMap, GenICam::gcstring heartBeatTimeOut);

	static IStDeviceReleasable* CreateIStDeviceByIPAddress(IStInterface* pIStInterface, const int64_t nDeviceIPAddress);

protected:

private:

};