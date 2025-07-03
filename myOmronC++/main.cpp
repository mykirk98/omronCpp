#include "TriggerCamera.h"

int main()
{
	std::cout << "==========Trigger Camera Example==========" << std::endl;
	CStApiAutoInit objStApiAutoInit;
	CIStSystemPtr pSystem(CreateIStSystem());

	TriggerCamera cameraWorker;
	if (cameraWorker.Initialize(pSystem))
	{
		cameraWorker.StartAcquisition();

		std::cout << "0: Generate trigger" << std::endl;
		std::cout << "Else: Exit" << std::endl;
		std::cout << "Select: ";

		while (true)
		{
			size_t nindex;
			std::cin >> nindex;
			if (nindex == 0)
			{
				cameraWorker.pICommandTriggerSoftware->Execute();
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}