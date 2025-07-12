// GigEManager.h
#pragma once

#include <StApi_TL.h>
#include "GigEWorker.h"
#include <vector>
#include <memory>

class GigEManager {
public:
    GigEManager();
    ~GigEManager();

    bool Initialize();                // 전체 인터페이스와 디바이스 스캔 및 초기화
    void StartAll();                  // 모든 워커 스레드 시작
    void StopAll();                   // 모든 워커 스레드 종료
    void TriggerAll();                // 모든 카메라에 트리거 발행
    void TriggerSelected(const std::vector<int>& indices); // 선택된 인덱스 카메라만 트리거
    void RunInteractiveLoop();        // 사용자 입력 루프

private:
    CIStSystemPtr m_pSystem;
    std::vector<std::shared_ptr<GigEWorker>> m_workers;
};