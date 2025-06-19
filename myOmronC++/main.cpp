#include "CameraWorker.h"

int main()
{
	CameraWorker cameraWorker(10);
	
	if (cameraWorker.initialize())
	{
		cameraWorker.StartAcquisition();
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	
	return 0;
}