#include "GigEManager.h"

GigEManager::GigEManager(std::string rootDir)
    : m_strRootDir(rootDir)
    , m_running(false)
{
    m_logger = std::make_shared<CamLogger>();
    m_pFrameQueue = std::make_shared<YCQueue<FrameData>>();
    m_pImageSaverThreadPool = std::make_shared<ImageSaverThreadPool>(5, m_strRootDir, m_pFrameQueue, m_pPathQueue, m_logger);
}

GigEManager::GigEManager(std::string saveRootDir, std::shared_ptr<YCQueue<std::string>> pathQueue)
    : m_strRootDir(saveRootDir)
    , m_running(false)
    , m_pPathQueue(pathQueue)
	, m_pSleeveACameraQueue(std::make_shared<YCQueue<cv::Mat>>())
	, m_pEndoscopeSideCameraQueue(std::make_shared<YCQueue<cv::Mat>>())
	, m_pEndoscopeRobotCameraQueue(std::make_shared<YCQueue<cv::Mat>>())
	, m_pEndoscopeRobotEndoscopeCameraQueue(std::make_shared<YCQueue<cv::Mat>>())
{
    m_logger = std::make_shared<CamLogger>();
    m_pFrameQueue = std::make_shared<YCQueue<FrameData>>();
    m_pImageSaverThreadPool = std::make_shared<ImageSaverThreadPool>(2, m_strRootDir, m_pFrameQueue, m_pPathQueue, m_logger);
    m_LCP24100SS = std::make_shared<LCP24100SS>();
    m_LCP100DC = std::make_shared<LCP100DC>();

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
		// Initialize all interfaces and cameras
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
                    const std::string& cameraName = camera->GetUserDefinedName();
                    camera->SetFrameQueue(m_pFrameQueue);

                    if (cameraName == "Sleeve_A_Camera")
                    {
						camera->SetCVMatQueue(m_pSleeveACameraQueue);
                    }
                    else if (cameraName == "Endoscope_Side_Camera")
                    {
                        camera->SetCVMatQueue(m_pEndoscopeSideCameraQueue);
                    }
                    else if (cameraName == "Endoscope_Robot_Camera")
                    {
                        camera->SetCVMatQueue(m_pEndoscopeRobotCameraQueue);
                    }
                    else if (cameraName == "Endoscope_Robot_Endoscope_Camera")
                    {
                        camera->SetCVMatQueue(m_pEndoscopeRobotEndoscopeCameraQueue);
                    }

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

		// Initialize LCP24100SS light controller
        if (m_LCP24100SS->open("/dev/ttyUSB0", 19200))
        {
            for (char ch = '1'; ch <= '6'; ++ch)
            {
                if (ch == '6') // Side camera
                {
                    m_LCP24100SS->setBrightness(ch, 120);
                    m_LCP24100SS->setStrobeTime_ms(ch, 10.00);
                    continue;
                }
                m_LCP24100SS->setBrightness(ch, 120);
                m_LCP24100SS->setStrobeTime_ms(ch, 100.00);
            }
            m_logger->Log("[GigEManager] LCP24100SS initialized.");
        }
        else
        {
            m_logger->Log("[GigEManager] Failed to open LCP24100SS on /dev/ttyUSB0");
        }

		// Initialize LCP100DC light controller
        if (m_LCP100DC->open("/dev/ttyUSB1", 19200))
        {
            m_LCP100DC->setBrightness('1', 50);
            m_LCP100DC->turnOff('1');
            m_logger->Log("[GigEManager] LCP100DC initialized.");
        }
        else
        {
            m_logger->Log("[GigEManager] Failed to open LCP100DC on /dev/ttyUSB1");
        }

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

    m_LCP100DC->turnOff('1');
    m_LCP24100SS->close();
    m_LCP100DC->close();

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

        if (cameraName == "Endoscope_Robot_Camera")
        {
            m_LCP24100SS->trigger('1');
            m_LCP24100SS->trigger('2');
            m_LCP24100SS->trigger('3');
            m_LCP24100SS->trigger('4');
            m_LCP24100SS->trigger('5');
        }
        else if (cameraName == "Endoscope_Side_Camera")
        {
            m_LCP24100SS->trigger('6');
        }
        else if (cameraName == "Sleeve_A_Camera")
        {
            m_LCP100DC->trigger_ms('1', 100); // Channel 1, ON for 100 ms
        }
    }
    else
    {
        m_logger->Log("[GigEManager] Camera not found: " + cameraName);
    }
}

void GigEManager::TriggerSingle(const std::string& camneraName, const std::string detailInfo)
{
    if (detailInfo != "TOP" && detailInfo != "BOTTOM" &&
        detailInfo != "1" && detailInfo != "2" && detailInfo != "3" && detailInfo != "4" && detailInfo != "5" &&
		detailInfo != "6" && detailInfo != "7" && detailInfo != "8" && detailInfo != "9" && detailInfo != "10")
    {
        m_logger->Log("[GigEManager] Invalid detail info: " + detailInfo);
        return;
	}

    std::map<std::string, std::shared_ptr<GigECamera>>::iterator it = m_cameraMap.find(camneraName);
    if (it != m_cameraMap.end())
    {
        it->second->ExecuteTrigger(detailInfo);
    }
}

std::shared_ptr<YCQueue<cv::Mat>> GigEManager::GetSleeveACameraQueue()
{
    return m_pSleeveACameraQueue;
}

std::shared_ptr<YCQueue<cv::Mat>> GigEManager::GetEndoscopeSideCameraQueue()
{
    return m_pEndoscopeSideCameraQueue;
}

std::shared_ptr<YCQueue<cv::Mat>> GigEManager::GetEndoscopeRobotCameraQueue()
{
    return m_pEndoscopeRobotCameraQueue;
}

std::shared_ptr<YCQueue<cv::Mat>> GigEManager::GetEndoscopeRobotEndoscopeCameraQueue()
{
    return m_pEndoscopeRobotEndoscopeCameraQueue;
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
    std::shared_ptr<YCQueue<std::string>> pathQueue = std::make_shared<YCQueue<std::string>>();
    //std::shared_ptr<PathQueue> pathQueue = std::make_shared<PathQueue>();
    //GigEManager manager(DEVELOPMENT_PC_DIRECTORY, pathQueue);
    GigEManager manager(DEVELOPMENT_PC_DIRECTORY);

    if (!manager.Initialize())
    {
        std::cerr << "Failed to initialize cameras.\n";
        return -1;
    }

    manager.StartAll();

    for (int i = 0; i < 10; ++i)
    {
        manager.TriggerSingle("5MP_1");
        manager.TriggerSingle("5MP_2");
        manager.TriggerSingle("5MP_3");
        manager.TriggerSingle("12MP_1");
        manager.TriggerSingle("12MP_2");
        Sleep(150);
    }
    Sleep(1000);
    manager.StopAll();
    return 0;
}
*/