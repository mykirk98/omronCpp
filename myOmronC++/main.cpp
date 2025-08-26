#include "GigEManager.h"

int main()
{
	std::shared_ptr<ThreadSafeQueue<std::string>> pathQueue = std::make_shared<ThreadSafeQueue<std::string>>();
	//std::shared_ptr<PathQueue> pathQueue = std::make_shared<PathQueue>();
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
    
    for (int i = 0; i < 10; ++i)
    {
        manager.TriggerSingle("5MP_1");
        manager.TriggerSingle("5MP_2");
        manager.TriggerSingle("5MP_3");
		//manager.TriggerSingle("12MP_1");
		manager.TriggerSingle("12MP_2");
        manager.TriggerSingle("2MP_1");
		manager.TriggerSingle("2MP_2");
#ifdef _WIN32
        Sleep(150);
#else
		usleep(150 * 1000);  // 150 ms
#endif // _WIN32

    }
    Sleep(3000);
    manager.StopAll();
    return 0;
}