#include "CameraWorkerCB.h"


// 사용 예시 (main.cpp에서 호출)

int main()
{
	CameraWorkerCB cameraWorker;
	if (cameraWorker.initialize())
	{
		cameraWorker.startAcquisition();

		while (true)
		{
			std::cout << "0: Generate trigger" << std::endl;
			std::cout << "Else: Exit" << std::endl;
			std::cout << "Select: ";

			size_t nindex;
			std::cin >> nindex;
			if (nindex == 0)
			{
				cameraWorker.pICommandTriggerSoftware->Execute();
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}