#include "GigEManager.h"
#include "PathQueue.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

int main()
{
	std::shared_ptr<PathQueue> pathQueue = std::make_shared<PathQueue>();
	std::string saveRootDir = "C:\\Users\\mykir\\Work\\Experiments\\"; // NOTE: LAB WINDOWS PC DIRECTORY
	//std::string saveRootDir = "C:\\Users\\USER\\Pictures\\"; // NOTE: HOME PC DIRECTORY
	//std::string saveRootDir = "/home/msis/Pictures/SentechExperiments/Experiments1/"; // NOTE: LAB LINUX PC DIRECTORY
    //GigEManager manager(saveRootDir, pathQueue);
    GigEManager manager(saveRootDir);

    if (!manager.Initialize())
    {
        std::cerr << "Failed to initialize cameras.\n";
        return -1;
    }

    manager.StartAll();

    //std::cout << "Enter indices to trigger:\n";
    //std::cout << "  0     => trigger ALL cameras\n";
    //std::cout << "  1     => trigger camera at index 0\n";
    //std::cout << "  2 3   => trigger cameras at indices 1 and 2\n";
    //std::cout << "Type 'q' to quit.\n";

    //std::string line;
    //while (true)
    //{
    //    std::cout << "> ";
    //    std::getline(std::cin, line);

    //    if (line == "q")
    //        break;

    //    std::istringstream iss(line);
    //    std::vector<int> inputs;
    //    int num;

    //    while (iss >> num)
    //    {
    //        inputs.push_back(num);
    //    }

    //    if (inputs.empty())
    //    {
    //        std::cerr << "No input detected.\n";
    //        continue;
    //    }

    //    if (std::find(inputs.begin(), inputs.end(), 0) != inputs.end())
    //    {
    //        manager.TriggerAll();
    //    }
    //    else
    //    {
    //        for (int input : inputs)
    //        {
    //            int index = input - 1;
    //            manager.TriggerSingle(index);
    //        }
    //    }
    //}
    
    for (int i = 0; i < 20; ++i)
    {
        manager.TriggerSingle("5MP_1");
        manager.TriggerSingle("5MP_2");
        manager.TriggerSingle("5MP_3");
		manager.TriggerSingle("12MP_1");
		manager.TriggerSingle("12MP_2");
        Sleep(150);
    }
    Sleep(1000);
    manager.StopAll();
    return 0;
}