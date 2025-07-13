#include "GigEWorker.h"

GigEWorker::GigEWorker(std::unique_ptr<GigECamera> camera)
    : m_camera(std::move(camera))
    , m_running(false)
{
}

GigEWorker::~GigEWorker()
{
}

void GigEWorker::Start()
{
    try
    {
        m_camera->StartAcquisition();
        m_running = true;
		// Start the worker thread
        m_thread = std::thread(&GigEWorker::WorkerLoop, this);
    }
    catch (const GenICam::GenericException& e)
    {
        std::cerr << "[GigEWorker] Failed to start thread: " << e.GetDescription() << std::endl;
    }
}

void GigEWorker::Stop()
{
    try
    {
	    // Stop the camera acquisition and join the thread
        m_running = false;
	    // Ensure the camera is stopped before joining the thread
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }
    catch (const GenICam::GenericException& e)
    {
		std::cerr << "[GigEWorker] Stop thread error: " << e.GetDescription() << std::endl;
    }
}

void GigEWorker::Trigger()
{
    try
    {
		// Trigger the camera to capture an image
        m_camera->pICommandTriggerSoftware->Execute();
    }
    catch (const GenICam::GenericException& e)
    {
        std::cerr << "[GigEWorker] Trigger error: " << e.GetDescription() << std::endl;
    }
}

bool GigEWorker::IsRunning() const
{
	// Check if the worker is currently running
    return m_running.load();
}

void GigEWorker::WorkerLoop()
{
    while (m_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    m_camera->StopAcquisition();
}


// Example usage of GigEWorker
/*
#include "GigEWorker.h"
#include <StApi_TL.h>

int main()
{
    CStApiAutoInit stApiAutoInit;

    CIStSystemPtr pSystem = CreateIStSystem(StSystemVendor_Default, StInterfaceType_GigEVision);
    IStInterface* pInterface = pSystem->GetIStInterface(1);

    std::unique_ptr<GigECamera> camera = std::make_unique<GigECamera>();
    if (camera->Initialize(pInterface, 0))
    {
        GigEWorker worker(std::move(camera));
        worker.Start();
        std::cout << "[main] Worker started. Type '0' to trigger, 'q' to quit." << std::endl;

        std::string input;
        while (true)
        {
            std::getline(std::cin, input);

            if (input == "q") break;
            if (input == "0") { worker.Trigger(); }
        }

        worker.Stop();
        std::cout << "[main] Worker stopped. Exiting..." << std::endl;
    }
    return 0;
}
*/