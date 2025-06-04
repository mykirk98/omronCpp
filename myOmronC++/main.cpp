#include <StApi_TL.h>		// TL : Transport Layer
#include "CameraWorker.h"



int main(int /* args */, char** /* argv */)
{
	std::cout << "This is myOmronC++ application." << std::endl
		<< "It is designed to work with Omron cameras using StApi." << std::endl
		<< "Copyright by ChangSeong Yoo" << std::endl;

	CameraWorker cameraWorker(100);	// 100개의 이미지를 획득할 카메라 워커 객체 생성

	if (cameraWorker.initialize())
	{
		cameraWorker.StartAcquisition();
	}

	std::cout << "Press Enter to exit..." << std::endl;
	std::cin.get(); // Enter 키 입력 대기

	return 0;
}