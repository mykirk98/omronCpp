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
