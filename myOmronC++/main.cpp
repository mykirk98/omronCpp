#include "GigEManager.h"
#include <iostream>

int main()
{
	std::string saveRootDir = "C:\\Users\\mykir\\Work\\Experiments\\"; // NOTE: LAB WINDOWS PC DIRECTORY
	//std::string saveRootDir = "C:\\Users\\USER\\Pictures\\"; // NOTE: HOME PC DIRECTORY
	//std::string saveRootDir = "/home/msis/Pictures/SentechExperiments/Experiments1/"; // NOTE: LAB LINUX PC DIRECTORY
    GigEManager manager(saveRootDir);

    // Step 1: 카메라 초기화
    if (!manager.Initialize())
    {
        std::cerr << "[main] Failed to initialize GigEManager." << std::endl;
        return -1;
    }

    // Step 2: 모든 워커 스레드 시작 (acquisition 시작)
    manager.StartAll();
    std::cout << "[main] All camera workers started." << std::endl;

    // Step 3: 사용자 입력 루프 실행
    manager.RunInteractiveLoop();

    // Step 4: 모든 워커 종료 (acquisition 정지)
    manager.StopAll();
    std::cout << "[main] All camera workers stopped. Exiting..." << std::endl;

    return 0;
}