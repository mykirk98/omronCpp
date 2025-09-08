#include "GigEManager.h"
#include "BasicCamera.h"

int main()
{
int numImages = 100;
	CStApiAutoInit objStApiAutoInit; // Initialize StApi
	CIStSystemPtr pSystem(CreateIStSystem());

	BasicCamera camera(HOME_PC_DIRECTORY);
	
	if (camera.Initialize(pSystem))
	{
		camera.StartAcquisition(numImages);
		//camera.FreeRunCapture0();
		camera.FreeRunCapture1();
		camera.StopAcquisition();
	}
}