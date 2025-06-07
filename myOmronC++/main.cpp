#include <StApi_TL.h>		// TL : Transport Layer
#include "CameraWorker.h"
#include "CameraWorker_CB.h"



int main(int /* args */, char** /* argv */)
{
	CameraWorker_CB cameraWorker;
	if (cameraWorker.initialize())
	{
		cameraWorker.startAcquisition();

		// ... 이미지 처리 로직 ...

		std::cout << "Press Enter to stop acquisition..." << std::endl;
		std::cin.get(); // 사용자 입력 대기

		//cameraWorker.stopAcquisition();
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}