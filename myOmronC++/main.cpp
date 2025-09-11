#include "LCP24100SS.h"

int main() {
    std::unique_ptr<LightController> ctrl = std::make_unique<LCP24100SS>();

#ifdef _WIN32
    // COM10 이상이면 "\\\\.\\COM10" 형태 권장
    if (!ctrl->open("COM3", 19200))
    {
        std::cerr << "open failed\n";
        return 1;
    }
#else
    // Ubuntu 예시: dmesg | grep tty 로 포트 확인 (/dev/ttyUSB0 등)
    if (!ctrl->open("/dev/ttyUSB0", 19200))
    {
        std::cerr << "open failed\n";
        return 1;
    }
#endif
    ctrl->SetBrightness('1', 120);   // 밝기 120
    ctrl->SetStrobeTime_ms('1', 2.00); // 2.00 ms
    //ctrl->Trigger('1');              // 소프트 트리거

    for (int i = 0; i < 10; i++)
    {
        ctrl->Trigger('1');
        //Sleep(50); // 100ms 간격
        ctrl->Trigger('2');
        //Sleep(50); // 100ms 간격
        ctrl->Trigger('3');
        //Sleep(50); // 100ms 간격
        ctrl->Trigger('4');
        //Sleep(50); // 100ms 간격
        //ctrl->Trigger('3');
		//Sleep(50); // 100ms 간격
		//ctrl->Trigger('2');
		Sleep(500); // 100ms 간격
	}
	//ctrl->Trigger('1');
	//Sleep(500); // 100ms 간격
    ctrl->close();
    return 0;
}