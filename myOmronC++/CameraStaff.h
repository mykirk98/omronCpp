#pragma once

#include <thread>
#include <atomic>
#include <memory>
#include <string>
#include "TriggerCamera.h"

class CameraStaff
{
public:
    CameraStaff();
    ~CameraStaff();

    bool Initialize(const CIStSystemPtr& pSystem, const std::string& saveDir);
    void Start();
    void Stop();
    void Trigger();

private:
    void Run();

    std::unique_ptr<TriggerCamera> m_worker;
    std::thread m_thread;
    std::atomic<bool> m_running;
    std::atomic<bool> m_triggerRequested;
    std::string m_saveDir;
};

