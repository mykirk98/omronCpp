#include "CameraManager.h"
#include <iostream>

void CameraManager::AddCamera(std::unique_ptr<TriggerCamera> camera)
{
    m_cameras.push_back(std::move(camera));
}

void CameraManager::StartShooting(int imageCount)
{
    for (auto& camera : m_cameras)
    {
        m_threads.emplace_back([camera = camera.get(), imageCount](){
            camera->StartAcquisition();
            for (int i = 0; i < imageCount; ++i)
            {
                bool success = camera->TriggerAndWait(5000);
                if (success)
                    std::cout << "[Camera] Frame " << i << " captured." << std::endl;
                else
                    std::cerr << "[Camera] Frame " << i << " timeout!" << std::endl;
            }
            camera->StopAcquisition();
            });
    }
}

void CameraManager::JoinAll()
{
    for (auto& t : m_threads)
    {
        if (t.joinable())
            t.join();
    }
}

// Example usage of CameraManager class
/*
#include "CameraManager.h"
#include "FrameQueue.h"
#include <StApi_TL.h>

int main()
{
    CStApiAutoInit stApiInit;
    CIStSystemPtr pSystem(CreateIStSystem());

    //auto sharedQueue = std::make_shared<FrameQueue>();
    std::shared_ptr<FrameQueue> sharedQueue = std::make_shared<FrameQueue>();

    CameraManager manager;
    int numCameras = 1;
    int numImages = 10;
    // 예시: 2대의 카메라 생성
    for (int i = 0; i < numCameras; ++i)
    {
        auto camera = std::make_unique<TriggerCamera>();
        if (camera->Initialize(pSystem)) {
            camera->SetFrameQueue(sharedQueue);
            manager.AddCamera(std::move(camera));
        }
    }

    manager.StartShooting(numImages); // 카메라마다 100장 촬영
    manager.JoinAll();

    for (int i = 0; i < numCameras * numImages; ++i)
    {
        IStImage* image = sharedQueue->Pop();
        std::cout << "[Main] Popped frame, queue size: " << sharedQueue->Size() << std::endl;
    }

    return 0;
}
*/


//int main(int /* argc */, char** /* argv */)
//{
//	try
//	{
//		// Initialize StApi before using.
//		CStApiAutoInit objStApiAutoInit;
//
//		// Create a system object for device scan and connection.
//		CIStSystemPtr pIStSystem(CreateIStSystem(StSystemVendor_Default, StInterfaceType_GigEVision));
//		//CIStSystemPtr pIStSystem(CreateIStSystem());
//
//		// Check GigE interface for devices.
//		// If there is no camera, throw exception.
//		IStInterface* pIStInterface = NULL;
//
//		for (uint32_t i = 0; i < pIStSystem->GetInterfaceCount(); i++)
//		{
//			std::cout << "Interface " << i << ": " << pIStSystem->GetIStInterface(i)->GetIStInterfaceInfo()->GetDisplayName() << std::endl;
//			std::cout << "DeviceCount=" << pIStSystem->GetIStInterface(i)->GetDeviceCount() << std::endl;
//			pIStInterface = pIStSystem->GetIStInterface(i);
//
//			for (uint32_t j = 0; j < pIStSystem->GetIStInterface(i)->GetDeviceCount(); j++)
//			{
//				std::cout << "-------------------------------------------" << std::endl;
//				std::cout << "Device " << j << ": " << pIStInterface->GetIStDeviceInfo(j)->GetDisplayName() << std::endl;
//				std::cout << "SerialNumber " << pIStInterface->GetIStDeviceInfo(j)->GetSerialNumber() << std::endl;
//				//UpdateDeviceIPAddress(pIStInterface->GetIStPort()->GetINodeMap(), j, pIStInterface->GetIStDeviceInfo(j)->GetSerialNumber());
//				GigEConfigurator::UpdateDeviceIPAddress(pIStInterface->GetIStPort()->GetINodeMap(), j, pIStInterface->GetIStDeviceInfo(j)->GetSerialNumber());
//
//				GenApi::CIntegerPtr pGevDeviceForceIPAddress(pIStInterface->GetIStPort()->GetINodeMap()->GetNode(GEV_DEVICE_FORCE_IP_ADDRESS));
//				const int64_t nDeviceIPAddress = pGevDeviceForceIPAddress->GetValue();
//
//				CIStDevicePtr pIStDevice;
//				for (size_t i = 0; i < 30; ++i)
//				{
//					Sleep(1000);
//					//IStDeviceReleasable* pIStDeviceReleasable(CreateIStDeviceByIPAddress(pIStInterface, nDeviceIPAddress));
//					IStDeviceReleasable* pIStDeviceReleasable(GigEConfigurator::CreateIStDeviceByIPAddress(pIStInterface, nDeviceIPAddress));
//					if (pIStDeviceReleasable != NULL)
//					{
//						pIStDevice.Reset(pIStDeviceReleasable);
//						break;
//					}
//				}
//				if (!pIStDevice.IsValid())
//				{
//					throw RUNTIME_EXCEPTION("A device with an IP address of %s could not be found.", pGevDeviceForceIPAddress->ToString().c_str());
//				}
//
//				// Display the DisplayName of the device.
//				cout << "Device=" << pIStDevice->GetIStDeviceInfo()->GetDisplayName() << endl;
//
//				// Update the camera HeartbeatTimeout settings.
//				//UpdateHeartbeatTimeout(pIStDevice->GetRemoteIStPort()->GetINodeMap());
//				GigEConfigurator::UpdateHeartbeatTimeout(pIStDevice->GetRemoteIStPort()->GetINodeMap(), "3000000");
//
//			}
//		}