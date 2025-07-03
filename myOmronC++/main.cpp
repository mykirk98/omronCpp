#include "BasicCamera.h"

int main()
{
	std::cout << "==========Basic Camera Example==========" << std::endl;
	CStApiAutoInit objStApiAutoInit; // Initialize StApi
	CIStSystemPtr pSystem(CreateIStSystem()); // Create a system object for device scan and connection

	BasicCamera basicCamera(10);
	if (basicCamera.Initialize(pSystem))
	{
		basicCamera.StartAcquisition();

		basicCamera.SequentialCapture();

		// image processing and saving logic can be added here...
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}