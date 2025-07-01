//#include "CameraManager.h"
#include "CameraWorkerCB.h"
//#include <iostream>	// this header is used for input/output operations
//#include <string>	// this header is used for string manipulation
//#include <chrono>	// this header is used for time-related functions
//#include "CameraStaff.h"

int main()
{
	std::cout << "Trigger Camera Example" << std::endl;
	std::string directory = "C:\\Users\\mykir\\Work\\Experiments\\";	//NOTE: LAB PC DIRECTORY
	//std::string directory = "C:\\Users\\USER\\Pictures\\";//NOTE: HOME PC DIRECTORY
	CStApiAutoInit objStApiAutoInit;
	CIStSystemPtr pSystem(CreateIStSystem());

	CameraWorkerCB cameraWorker;
	if (cameraWorker.Initialize(pSystem))
	{
		cameraWorker.StartAcquisition();

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
				std::cout << "captured image and waiting for saving image..." << std::endl;
				Sleep(3000);
				cameraWorker.SaveImageToFile(directory);
				std::cout << "Image saved to " << directory << std::endl;

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