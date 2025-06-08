#include "CameraEventWorker.h"

int main()
{
	std::cout << "2025/06/08 - Camera Event C++ Code" << std::endl;

	CameraEventWorker camera(100);
	if (camera.initialize())
	{
		camera.startAcquisition();
	}

	/*std::cout << "Press Enter to stop acquisition..." << std::endl;
	std::cin.get();*/
}