#include "GigEManager.h"

GigEManager::GigEManager(std::string rootDir)
    : m_strRootDir(rootDir)
    , m_running(false)
{
	m_logger = std::make_shared<Logger>();
    m_pFrameQueue = std::make_shared<ThreadSafeQueue<FrameData>>();
    m_pCVMatQueue = std::make_shared<ThreadSafeQueue<cv::Mat>>();
	m_pImageSaverThreadPool = std::make_shared<ImageSaverThreadPool>(5, m_strRootDir, m_pFrameQueue, m_pPathQueue, m_logger);
}

GigEManager::GigEManager(std::string saveRootDir, std::shared_ptr<ThreadSafeQueue<std::string>> pathQueue)
    : m_strRootDir(saveRootDir)
	, m_running(false)
	, m_pPathQueue(pathQueue)
{
    m_pFrameQueue = std::make_shared<ThreadSafeQueue<FrameData>>();
    m_pCVMatQueue = std::make_shared<ThreadSafeQueue<cv::Mat>>();
    m_pImageSaverThreadPool = std::make_shared<ImageSaverThreadPool>(5, m_strRootDir, m_pFrameQueue, m_pPathQueue, m_logger);
}

GigEManager::~GigEManager()
{
    StopAll();
}

bool GigEManager::Initialize()
{
    try {
        m_logger->Start();
		m_pImageSaverThreadPool->Start();

        m_pSystem = CreateIStSystem(StSystemVendor_Default, StInterfaceType_GigEVision);
        for (uint32_t ifaceIdx = 0; ifaceIdx < m_pSystem->GetInterfaceCount(); ++ifaceIdx)
        {
            IStInterface* pInterface = m_pSystem->GetIStInterface(ifaceIdx);
            m_logger->Log("Interface " + std::to_string(ifaceIdx) + "(" + std::string(pInterface->GetIStInterfaceInfo()->GetDisplayName()) + ")" + " initialized, " + std::to_string(pInterface->GetDeviceCount()) + " devices detected.");
            m_logger->Log("---------------------------------------------------------------------------------------");
            for (uint32_t deviceIdx = 0; deviceIdx < pInterface->GetDeviceCount(); ++deviceIdx)
            {
                std::shared_ptr<GigECamera> camera = std::make_shared<GigECamera>(m_strRootDir, m_logger);
                if (camera->Initialize(pInterface, deviceIdx))
                {
				    const std::string& cameraName = camera->GetUserDefinedName();    //TODO: 이 시점에서 cameraName이 어떻게 결정된 것인지 확인 필요
                    camera->SetFrameQueue(m_pFrameQueue);
                    //TODO: opencv mat 큐 설정
					camera->SetCVMatQueue(m_pCVMatQueue);
                    m_cameras.push_back(camera);
                    m_cameraMap[cameraName] = camera;
                }
                else
                {
					m_logger->Log("[GigEManager] Failed to initialize camera " + std::to_string(deviceIdx));
                }
                m_logger->Log("---------------------------------------------------------------------------------------");
            }
        }
		m_logger->Log("[GigEManager] Total " + std::to_string(m_cameras.size()) + " cameras initialized.");
        return !m_cameras.empty();
    }
    catch (const GenICam::GenericException& e)
    {
		m_logger->Log("[GigEManager] Initialization error: " + std::string(e.GetDescription()));
        return false;
    }
}

void GigEManager::StartAll()
{
    m_running = true;
    for (std::shared_ptr<GigECamera>& camera : m_cameras)
    {
        try
        {
            camera->StartAcquisition();
			m_threads.emplace_back(&GigEManager::CameraLoop, this, camera);
        }
        catch (const std::exception& e)
        {
			m_logger->Log("[GigEManager] Worker failed to start: " + std::string(e.what()));
        }
    }
	m_logger->Log("[GigEManager] All cameras started successfully.");
}

void GigEManager::StopAll()
{
    m_running = false;
    for (std::thread& thread : m_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    for (std::shared_ptr<GigECamera>& camera : m_cameras)
    {
        camera->StopAcquisition();
    }
	m_pImageSaverThreadPool->Stop();
	m_logger->Log("[GigEManager] All cameras stopped successfully.");
	m_logger->Stop();
    m_threads.clear();

}

//void GigEManager::TriggerAll()
//{
//    for (std::shared_ptr<GigEWorker>& worker : m_workers)
//    {
//        worker->Trigger();
//    }
//}

void GigEManager::TriggerSingle(int index)
{
    if (index >= 0 && index < static_cast<int>(m_cameras.size()))
    {
        m_cameras[index]->ExecuteTrigger();
    }
    else
    {
		m_logger->Log("[GigEManager] Invalid worker index: " + std::to_string(index));
    }
}

void GigEManager::TriggerSingle(const std::string& cameraName)
{
	std::map<std::string, std::shared_ptr<GigECamera>>::iterator it = m_cameraMap.find(cameraName);
    if (it != m_cameraMap.end())
    {
        it->second->ExecuteTrigger();
    }
    else
    {
		m_logger->Log("[GigEManager] Camera not found: " + cameraName);
	}
}

void GigEManager::CameraLoop(std::shared_ptr<GigECamera> camera)
{
    while (m_running)
    {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // Here you can add any additional logic for the camera loop if needed;
		// e.g., checking camera status, temperature, heartbeat, etc.
	}
}

// Example usage of GigEManager class
/*
#include "GigEManager.h"

int main()
{
    std::shared_ptr<ThreadSafeQueue<std::string>> pathQueue = std::make_shared<ThreadSafeQueue<std::string>>();
    //std::shared_ptr<PathQueue> pathQueue = std::make_shared<PathQueue>();
    //GigEManager manager(saveRootDir, pathQueue);
    GigEManager manager(LAB_WINDOW_PC_DIRECTORY);

    if (!manager.Initialize())
    {
        std::cerr << "Failed to initialize cameras.\n";
        return -1;
    }

    manager.StartAll();

    for (int i = 0; i < 5; ++i)
    {
        manager.TriggerSingle("5MP_1");
        manager.TriggerSingle("5MP_2");
        //manager.TriggerSingle("5MP_3");
        //manager.TriggerSingle("5MP_4");
        //manager.TriggerSingle("12MP_1");
        //manager.TriggerSingle("12MP_2");
        manager.TriggerSingle("2MP_1");
        //manager.TriggerSingle("2MP_2");
#ifdef _WIN32
        Sleep(150);
#else
        usleep(150 * 1000);  // 150 ms
#endif // _WIN32

    }
    Sleep(3000);
    manager.StopAll();
    return 0;
}
*/