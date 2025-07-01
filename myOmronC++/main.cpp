#include "CameraManager.h"
#include <iostream>
#include <string>

int main()
{
	std::string saveDirectory = "C:\\Users\\mykir\\Work\\Experiments\\";    //NOTE: LAB PC DIRECTORY
    size_t cameraCount = 2;

    CameraManager cameraManager;

    if (!cameraManager.InitializeAll(cameraCount))
    {
        std::cerr << "Failed to initialize manager" << std::endl;
        return -1;
    }

    cameraManager.StartAcquisitionAll();

    while (true)
    {
        std::cout << "\n0: Send trigger" << std::endl;
        std::cout << "1: Save image" << std::endl;
        std::cout << "2: Terminate" << std::endl;
        std::cout << "Input: ";

        int choice;
        std::cin >> choice;

        if (choice == 0)
        {
            cameraManager.TriggerAll();
            std::cout << "Sending trigger completed." << std::endl;
        }
        else if (choice == 1)
        {
            cameraManager.SaveImageAll(saveDirectory);
            std::cout << "Saving image completed: " << saveDirectory << std::endl;
        }
        else if (choice == 2)
        {
            break;
        }
        else
        {
            std::cout << "Wrong input." << std::endl;
        }
    }

    cameraManager.StopAcquisitionAll();

    return 0;
}