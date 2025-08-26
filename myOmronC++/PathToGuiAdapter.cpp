#include "PathToGuiAdapter.h"
#include <chrono>

PathToGuiAdapter::PathToGuiAdapter(std::shared_ptr<PathQueue> pathQueue, GUISyncManager* guiSyncManager)
    : m_pathQueue(pathQueue), m_guiSyncManager(guiSyncManager), m_running(false)
{
}

PathToGuiAdapter::~PathToGuiAdapter()
{
    Stop();
}

void PathToGuiAdapter::Start()
{
    m_running = true;
    m_thread = std::thread(&PathToGuiAdapter::ForwardLoop, this);
}

void PathToGuiAdapter::Stop()
{
    m_running = false;
    if (m_thread.joinable())
        m_thread.join();
}

void PathToGuiAdapter::ForwardLoop()
{
    while (m_running)
    {
        std::string message;
        if (m_pathQueue && m_pathQueue->PopWithTimeout(message, std::chrono::milliseconds(100)))
        {
            if (m_guiSyncManager)
            {
                m_guiSyncManager->sendMessage(message);  // 핵심 전달 지점
            }
        }
    }
}