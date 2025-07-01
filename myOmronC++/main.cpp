//#include "CameraManager.h"
#include "CameraWorkerCB.h"
//#include <iostream>	// this header is used for input/output operations
//#include <string>	// this header is used for string manipulation
//#include <chrono>	// this header is used for time-related functions
//#include "CameraStaff.h"

int main()
{
	std::cout << "Basic Camera Worker Example" << std::endl;
	CStApiAutoInit objStApiAutoInit; // Initialize StApi
	CIStSystemPtr pSystem(CreateIStSystem()); // Create a system object for device scan and connection

	std::string targetDir = "C:\\Users\\mykir\\Work\\Experiments\\";	//NOTE: LAB PC DIRECTORY
	BasicCamera cameraWorker(10);
	if (cameraWorker.Initialize(pSystem))
	{
		cameraWorker.StartAcquisition();

		// image processing and saving logic can be added here...
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}