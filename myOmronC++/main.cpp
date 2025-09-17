//#include "LCP24100SS.h"
//#include "LCP100DC.h"
//#include <iostream>
//
//int main() {
//    LCP24100SS LCP24;
//    LCP100DC LCP100;
//
//#ifdef _WIN32
//    if (!LCP24.open("COM3", 19200))
//    {
//        std::cerr << "open fail\n";
//        return 1;
//    }
//    if (!LCP100.open("COM5", 19200))
//    {
//        std::cerr << "open fail\n";
//        return 1;
//    }
//#else
//    // if (!LCP24.open("/dev/ttyUSB1", 19200))
//    // {
//    //     std::cerr << "open fail\n";
//    //     return 1;
//    // }
//    // std::cout << "LCP24 opened\n";
//    if (!LCP100.open("/dev/ttyUSB0", 19200))
//    {
//        std::cerr << "open fail\n";
//        return 1;
//    }
//    std::cout << "LCP100 opened\n";
//#endif
//
//    LCP24.setBrightness('1', 120);
//    LCP24.setStrobeTime_ms('1', 2.00);
//
//    LCP100.setBrightness('1', 10);
//    // LCP100.trigger_ms('1', 30.0);
//
//    for (int i = 0; i < 3; i++)
//    {
//        LCP24.trigger('1');
//        Sleep(100);
//        //usleep(100000);
//        LCP24.trigger('2');
//        Sleep(100);
//        //usleep(100000);
//        LCP24.trigger('3');
//        Sleep(100);
//        //usleep(100000);
//        LCP24.trigger('4');
//        Sleep(100);
//		//usleep(100000);
//        LCP100.trigger_ms('1', 30.0);
//        Sleep(100);
//        //usleep(100000);
//        }
//
//    LCP24.close();
//    LCP100.close();
//    return 0;
//}

#include "LightManager.h"
#include "LCP100DCAdapter.h"
#include "LCP24100SSAdapter.h"

int main()
{
    LightManager mgr;

    // 모델 등록 (보통 프로그램 초기화 단계에서 수행)
    LightFactory::instance().registerModel("LCP100DC",
        [](bool lowercase) { return std::make_shared<LCP100DCAdapter>(lowercase); });
    LightFactory::instance().registerModel("LCP24100SS",
        [](bool) { return std::make_shared<LCP24100SSAdapter>(); });

    // 컨트롤러 추가
    mgr.addController({ "light1", "LCP100DC", "COM5", 19200, true });
    mgr.addController({ "light2", "LCP24100SS", "COM3", 19200 });

    // 열기
    mgr.openAll();

    // 제어
    mgr.setBrightness("light1", '1', 80);
    mgr.trigger("light2", '1', 5.0);
	mgr.trigger("light1", '1', 20.0);

    // 닫기
    mgr.closeAll();
}