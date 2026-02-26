#include "GigEManager.h"

int main()
{
    GigEManager manager(HOME_PC_DIRECTORY);

    if (!manager.Initialize())
    {
        std::cerr << "Failed to initialize cameras.\n";
        return -1;
    }

    manager.StartAll();

    for (int i = 0; i < 5; ++i)
    {
        manager.TriggerSingle("TOP");
        manager.TriggerSingle("BOTTOM");
        //manager.TriggerSingle("5MP_3");
        //manager.TriggerSingle("5MP_4");
        //manager.TriggerSingle("12MP_1");
        //manager.TriggerSingle("12MP_2");
        //manager.TriggerSingle("2MP_1");
        //manager.TriggerSingle("2MP_2");
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