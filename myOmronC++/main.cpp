#include "CameraManager.h"
#include <iostream>
#include <string>
#include "CameraStaff.h"

int main()
{
    using namespace StApi;
	CStApiAutoInit objStApiAutoInit; // Initialize StApi
	CIStSystemPtr system(CreateIStSystem()); // Create a system object for device scan and connection

    //CIStSystemPtr system = CreateIStSystem();
    std::string saveDir = "C:\\Captured\\";

    CameraStaff staff;
    if (staff.Initialize(system, saveDir))
    {
        staff.Start();

        while (true)
        {
            std::cout << "0: Trigger image, Else: Quit\n> ";
            int cmd;
            std::cin >> cmd;

            if (cmd == 0)
                staff.Trigger();
            else
                break;
        }

        staff.Stop();
    }

}
