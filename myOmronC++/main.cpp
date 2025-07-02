#include "BasicCamera.h"

int main()
{
	std::cout << "==========Basic Camera Example==========" << std::endl;
	CStApiAutoInit objStApiAutoInit; // Initialize StApi
	CIStSystemPtr pSystem(CreateIStSystem()); // Create a system object for device scan and connection

	//std::string targetDir = "C:\\Users\\mykir\\Work\\Experiments\\";	//NOTE: LAB PC DIRECTORY
	BasicCamera basicCamera(10);
	if (basicCamera.Initialize(pSystem))
	{
		basicCamera.StartAcquisition();

		// image processing and saving logic can be added here...
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}