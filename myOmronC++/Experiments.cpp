#include "BasicCamera.h"

void FreeRunPlainExperiment()
{
	int numImages = 100;
	CStApiAutoInit objStApiAutoInit; // Initialize StApi
	CIStSystemPtr pSystem(CreateIStSystem());

	BasicCamera camera(HOME_PC_DIRECTORY);
	
	if (camera.Initialize(pSystem))
	{
		camera.StartAcquisition(numImages);
		camera.FreeRunCapture0();
		camera.StopAcquisition();
	}
}

/* 프레임 획득->이미지 저장->OpenCV Mat 변환 순서를 직렬로 처리하여
각 과정이 걸리는 시간을 측정하는 실험*/
void FreeRunExperiment()
{
	int numImages = 100;
	CStApiAutoInit objStApiAutoInit; // Initialize StApi
	CIStSystemPtr pSystem(CreateIStSystem());

	BasicCamera camera(HOME_PC_DIRECTORY);

	if (camera.Initialize(pSystem))
	{
		camera.StartAcquisition(numImages);
		camera.FreeRunCapture1();

		camera.StopAcquisition();
	}
}