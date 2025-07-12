#include "GigEWorker.h"
#include <iostream>
#include <chrono>

GigEWorker::GigEWorker(std::unique_ptr<GigECamera> camera)
    : m_camera(std::move(camera))
    , m_running(false)
{
}

GigEWorker::~GigEWorker()
{
    Stop();
}

void GigEWorker::Start()
{
    if (m_running)
        return;
    m_running = true;
    m_thread = std::thread(&GigEWorker::WorkerLoop, this);
}

void GigEWorker::Stop()
{
    if (!m_running)
        return;
    m_running = false;
    
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void GigEWorker::Trigger()
{
    if (m_camera && m_camera->pICommandTriggerSoftware)
    {
        try
        {
            m_camera->pICommandTriggerSoftware->Execute();
        }
        catch (const GenICam::GenericException& e)
        {
            std::cerr << "[GigEWorker] Trigger error: " << e.GetDescription() << std::endl;
        }
    }
}

bool GigEWorker::IsRunning() const
{
    return m_running.load();
}

void GigEWorker::WorkerLoop()
{
    // Start acquisition before entering loop
    m_camera->StartAcquisition();

    while (m_running)
    {
        // In a real application, you could monitor frame queue, handle logic, etc.
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Stop acquisition when exiting loop
    m_camera->StopAcquisition();
}

// Example usage of GigEWorker
/*
#include "GigEWorker.h"
#include <StApi_TL.h>
#include <iostream>
#include <string>

int main() {
    try {
        CStApiAutoInit stApiInit;

        CIStSystemPtr system = CreateIStSystem(StSystemVendor_Default, StInterfaceType_GigEVision);
        IStInterface* pInterface = system->GetIStInterface(1);

        std::unique_ptr<GigECamera> camera = std::make_unique<GigECamera>();
        if (!camera->Initialize(pInterface, 0))
        {
            std::cerr << "Failed to initialize GigECamera." << std::endl;
            return -1;
        }

        GigEWorker worker(std::move(camera));
        worker.Start();
        std::cout << "[main] Worker started. Type '0' to trigger, 'q' to quit." << std::endl;

        std::string input;
        while (true)
        {
            //std::cout << "> ";
            std::getline(std::cin, input);

            if (input == "q") break;
            if (input == "0")   {   worker.Trigger();   }
        }

        worker.Stop();
        std::cout << "[main] Worker stopped. Exiting..." << std::endl;

    }
    catch (const GenICam::GenericException& e)
    {
        std::cerr << "GenICam exception: " << e.GetDescription() << std::endl;
        return -1;
    }

    return 0;
}
*/