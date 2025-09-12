#include "LCP24100SS.h"
#include "LCP100DC.h"
#include <iostream>

int main() {
    LCP24100SS LCP24;
    LCP100DC LCP100;

#ifdef _WIN32
    if (!LCP24.open("COM3", 19200))
    {
        std::cerr << "open fail\n";
        return 1;
    }
    if (!LCP100.open("COM5", 19200))
    {
        std::cerr << "open fail\n";
        return 1;
    }
#else
    if (!LCP24.open("/dev/ttyUSB0", 19200))
    {
        std::cerr << "open fail\n";
        return 1;
    }
    if (!LCP100.open("/dev/ttyUSB1", 19200))
    {
        std::cerr << "open fail\n";
        return 1;
    }
#endif

    // 채널1: 밝기 120, 스트로브 2.00ms, 트리거 1회
    //LCP24.setBrightness('1', 120);
    //LCP24.setStrobeTime_ms('1', 2.00);
    
    for (int i = 0; i < 10; i++)
    {
        LCP24.trigger('1');
        Sleep(100);
        LCP24.trigger('2');
        Sleep(100);
        LCP24.trigger('3');
        Sleep(100);
        LCP24.trigger('4');
		Sleep(100);
        LCP100.trigger_ms('1', 30.0);
        Sleep(100);
        }

    LCP24.close();
    LCP100.close();
    return 0;
}