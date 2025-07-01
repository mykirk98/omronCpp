#include "CameraStaff.h"
int main()
{
    CStApiAutoInit objStApiAutoInit;
    CIStSystemPtr system = CreateIStSystem();
    std::string saveDir = "C:\\Users\\mykir\\Work\\Experiments\\";	//NOTE: LAB PC DIRECTORY

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