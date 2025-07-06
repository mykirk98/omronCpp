#include "CameraStaff.h"

//#define LOGGING

CameraStaff::CameraStaff()
    : m_camera(std::make_unique<TriggerCamera>())
    , m_running(false)
    , m_triggerRequested(false)
{
#ifdef LOGGING
    std::cout << "[CameraStaff] constructed" << std::endl;
#endif // LOGGING
}

CameraStaff::~CameraStaff()
{
    Stop();
#ifdef LOGGING
	std::cout << "[CameraStaff] destructed" << std::endl;
#endif // LOGGING
}

bool CameraStaff::Initialize(const CIStSystemPtr& pSystem, const std::string& saveDir)
{
    m_saveDir = saveDir;
#ifdef LOGGING
	std::cout << "[CameraStaff] Initialized" << std::endl;
#endif // LOGGING
    return m_camera->Initialize(pSystem);
}

void CameraStaff::Start()
{
    m_running = true;
    m_camera->StartAcquisition();

    m_thread = std::thread(&CameraStaff::Run, this);
#ifdef LOGGING
	std::cout << "[CameraStaff] thread started" << std::endl;
#endif // LOGGING
}

void CameraStaff::Stop()
{
    m_running = false;
    if (m_thread.joinable())
        m_thread.join();

    m_camera->StopAcquisition();
#ifdef LOGGING
	std::cout << "[CameraStaff] thread stopped" << std::endl;
#endif // LOGGING
}

void CameraStaff::Trigger()
{
    m_triggerRequested = true;
#ifdef LOGGING
	std::cout << "[CameraStaff] Trigger requested." << std::endl;
#endif // LOGGING
}

void CameraStaff::Run()
{
    while (m_running)
    {
        if (m_triggerRequested)
        {
            try
            {
                //m_camera->pICommandTriggerSoftware->Execute();
#ifdef LOGGING
				std::cout << "[CameraStaff] Trigger executed." << std::endl;
#endif // LOGGING
            }
            catch (const std::exception& e)
            {
                std::cerr << "[CameraStaff] Trigger error: " << e.what() << std::endl;
            }
            m_triggerRequested = false;
        }
        // Sleep to prevent busy waiting, if i don't do this, the CPU usage will be high
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

/*
#include "CameraStaff.h"

int main()
{
    std::cout << "==========Camera Staff Example==========" << std::endl;
    CStApiAutoInit objStApiAutoInit;
    CIStSystemPtr system = CreateIStSystem();
    std::string saveDir = "C:\\Users\\mykir\\Work\\Experiments\\";	//NOTE: LAB PC DIRECTORY

    CameraStaff staff;
    if (staff.Initialize(system, saveDir))
    {
        staff.Start();

        while (true)
        {
            std::cout << "0: Trigger image, Else: Quit\n> ";
            int cmd;
            std::cin >> cmd;

            if (cmd == 0)
                staff.Trigger();
            else
                break;
        }

        staff.Stop();
    }
}
*/