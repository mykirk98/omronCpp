#include "CameraWorker_Chunk.h"

int main()
{
	std::cout << "Camera Worker Class with Chunk Data" << std::endl;

	CameraWorker_Chunk camera(100);
	if (camera.initialize())
	{
		camera.startAcquisition();
	}
	return 0;
}