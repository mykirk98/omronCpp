#include "CameraManager.h"
#include <iostream>
#include <string>
#include <chrono>
#include "CameraStaff.h"

std::string saveDir = "/home/msis/Pictures/SentechExperiments/Experiments1/";
int main()
{
    int totalFrames = 1000; // 10���� 1000��� ȹ��
    CStApiAutoInit m_stApiAutoInit;
    CIStSystemPtr m_pSystem(CreateIStSystem());

	CameraWorker cameraWorker(totalFrames); // 10���� �̹��� ȹ��
	if (cameraWorker.Initialize(m_pSystem))
	{
        auto start = std::chrono::high_resolution_clock::now();

		cameraWorker.StartAcquisition();

        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end - start;
        double fps = totalFrames / elapsed.count();

        std::cout << "총 " << totalFrames << "장을 "
                  << elapsed.count() << "초 동안 캡처함. 평균 FPS = "
                  << fps << std::endl;

		// ... �̹��� ó�� ���� ...
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}