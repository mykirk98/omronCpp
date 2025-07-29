#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <string>
#include "PathQueue.h"
#include "../../GUI_ZeroMQ_Sync/include/gui_sync_manager.h"

// PathQueue에서 경로 메시지를 꺼내 GUISyncManager에 전달하는 어댑터
class PathToGuiAdapter
{
public:
    PathToGuiAdapter(std::shared_ptr<PathQueue> pathQueue, GUISyncManager* guiSyncManager);
    ~PathToGuiAdapter();

    void Start();  // 전달 스레드 시작
    void Stop();   // 스레드 종료

private:
    void ForwardLoop();  // 내부 메시지 처리 루프

private:
    std::shared_ptr<PathQueue> m_pathQueue;
    GUISyncManager* m_guiSyncManager;
    std::thread m_thread;
    std::atomic<bool> m_running;
};