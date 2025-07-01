#include "CameraStaff.h"
#include <chrono>
#include <iostream>

CameraStaff::CameraStaff()
    : m_worker(std::make_unique<CameraWorkerCB>())
    , m_running(false)
    , m_triggerRequested(false)
{
}

CameraStaff::~CameraStaff()
{
    Stop();
}

bool CameraStaff::Initialize(const CIStSystemPtr& pSystem, const std::string& saveDir)
{
    m_saveDir = saveDir;
    return m_worker->Initialize(pSystem);
}

void CameraStaff::Start()
{
    m_running = true;
    m_worker->StartAcquisition();

    m_thread = std::thread(&CameraStaff::Run, this);
}

void CameraStaff::Stop()
{
    m_running = false;
    if (m_thread.joinable())
        m_thread.join();

    m_worker->StopAcquisition();
}

void CameraStaff::Trigger()
{
    m_triggerRequested = true;
}

void CameraStaff::Run()
{
    while (m_running)
    {
        if (m_triggerRequested)
        {
            try
            {
                m_worker->pICommandTriggerSoftware->Execute();
                std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 이미지 저장 대기
                m_worker->SaveImageToFile(m_saveDir);
                std::cout << "[CameraStaff] Image saved to " << m_saveDir << std::endl;
            }
            catch (const std::exception& e)
            {
                std::cerr << "[CameraStaff] Trigger error: " << e.what() << std::endl;
            }
            m_triggerRequested = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // CPU 낭비 방지용 대기
    }
}
