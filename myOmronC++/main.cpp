#include "CameraWorkerCB.h"

int main()
{
	CameraWorker cameraWorker(1); // 10개의 이미지 획득
	if (cameraWorker.initialize())
	{
		cameraWorker.StartAcquisition();

		// ... 이미지 처리 로직 ...
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}

//int main()
//{
//	CameraWorkerCB cameraWorker;
//	if (cameraWorker.initialize())
//	{
//		cameraWorker.startAcquisition();
//
//		while (true)
//		{
//			std::cout << "0: Generate trigger" << std::endl;
//			std::cout << "Else: Exit" << std::endl;
//			std::cout << "Select: ";
//
//			size_t nindex;
//			std::cin >> nindex;
//			if (nindex == 0)
//			{
//				cameraWorker.pICommandTriggerSoftware->Execute();
//			}
//			else
//			{
//				break;
//			}
//		}
//	}
//	else
//	{
//		std::cerr << "Camera initialization failed." << std::endl;
//	}
//	return 0;
//}