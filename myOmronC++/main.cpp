#include "CameraStaff.h"

int main()
{
    std::cout << "==========Camera Staff Example==========" << std::endl;
    CStApiAutoInit objStApiAutoInit;
    CIStSystemPtr system = CreateIStSystem();
    std::string saveDir = "C:\\Users\\mykir\\Work\\Experiments\\";	//NOTE: LAB PC DIRECTORY

    CameraStaff staff;
    if (staff.Initialize(system, saveDir))
    {
        staff.Start();

        /*while (true)
        {
            std::cout << "0: Trigger image, Else: Quit\n> ";
            int cmd;
            std::cin >> cmd;

            if (cmd == 0)
                staff.Trigger();
            else
                break;
        }*/

        for (int i = 0; i < 10; ++i)
        {
            std::cout << "Triggering image " << i + 1 << std::endl;
            staff.Trigger();
            std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Simulate some delay between triggers
		}
        staff.Stop();
    }
}