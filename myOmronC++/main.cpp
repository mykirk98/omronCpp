#include "EventCamera.h"

int main()
{
	EventCamera camera(100);
	if (camera.initialize())
	{
		camera.startAcquisition();
	}

	std::cout << "Press Enter to stop acquisition..." << std::endl;
	std::cin.get();
}