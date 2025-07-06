#include "TriggerCamera.h"
#include <chrono>

int main()
{
	std::cout << "========== Trigger Camera with Wait Example ==========" << std::endl;

	CStApiAutoInit stApiAutoInit;
	CIStSystemPtr pSystem(CreateIStSystem());

	TriggerCamera camera;
	if (camera.Initialize(pSystem))
	{
		camera.StartAcquisition();

		// calculate average FPS
		std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

		for (int i = 0; i < 1000; ++i)
		{
			std::cout << "[Main] Triggering " << i << std::endl;
			if (camera.TriggerAndWait(100))
				std::cout << "[Main] Frame " << i << " captured." << std::endl;
			else
				std::cerr << "[Main] Frame " << i << " timed out." << std::endl;
		}

		std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
		double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
		double averageFPS = 1000.0 / (elapsedTime / 1000.0); // 1000 frames
		std::cout << "[Main] Average FPS: " << averageFPS << std::endl;

		camera.StopAcquisition();
	}
	
	return 0;
}