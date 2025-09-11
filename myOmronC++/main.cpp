#include "LCP24100SS.h"
#include <iostream>

int main() {
    LCP24100SS ctrl;
#ifdef _WIN32
    if (!ctrl.open("COM3", 19200))
    {
        std::cerr << "open fail\n";
        return 1;
    }
#else
    if (!ctrl.open("/dev/ttyUSB0", 19200))
    {
        std::cerr << "open fail\n";
        return 1;
    }
#endif

    // 채널1: 밝기 120, 스트로브 2.00ms, 트리거 1회
    ctrl.setBrightness('1', 120);
    ctrl.setStrobeTime_ms('1', 2.00);
    ctrl.trigger('1');

    ctrl.close();
    return 0;
}