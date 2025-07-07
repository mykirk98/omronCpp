#pragma once

#include "TriggerCamera.h"
#include <vector>
#include <memory>
#include <thread>

class CameraManager {
public:
    void AddCamera(std::unique_ptr<TriggerCamera> camera);
    void StartShooting(int imageCount); // capture loop per camera
    void JoinAll(); // join threads

private:
    std::vector<std::unique_ptr<TriggerCamera>> m_cameras;
    std::vector<std::thread> m_threads;
};
