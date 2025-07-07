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
        m_threads.emplace_back([camera = camera.get(), imageCount]() {
            camera->StartAcquisition();
            for (int i = 0; i < imageCount; ++i)
            {
                bool success = camera->TriggerAndWait(100);
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

    auto sharedQueue = std::make_shared<FrameQueue>();

    CameraManager manager;
    int numCameras = 2;
    int numImages = 4;
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