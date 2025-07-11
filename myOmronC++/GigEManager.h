// GigEManager.h
#pragma once

#include <StApi_TL.h>
#include "GigECamera.h"
#include <vector>
#include <memory>
#include <thread>

class GigEManager {
public:
    GigEManager();
    ~GigEManager();

    bool Initialize();
    void StartAll();
    void StopAll();
    void TriggerAll();
    void TriggerSelected(const std::vector<int>& indices);
    void RunInteractiveLoop();

private:
    CIStSystemPtr m_pSystem;
    std::vector<std::shared_ptr<GigECamera>> m_cameras;
};
