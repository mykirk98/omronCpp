// LfineLCP24_RS232.cpp : Build with MSVC (콘솔 프로젝트)
// COM 포트, 채널, 값만 바꿔서 실행하면 됩니다.

#include <windows.h>
#include <string>
#include <iostream>
#include <cstdio>

static bool openSerial(HANDLE& h, const std::wstring& comPort, DWORD baud = CBR_19200)
{
    h = CreateFileW(comPort.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE)
    {
        std::cerr << "[ERR] CreateFile fail. GetLastError=" << GetLastError() << "\n";
        return false;
    }
    // 19200 8N1, no flow control
    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(DCB);
    
    if (!GetCommState(h, &dcb))
    {
        std::cerr << "[ERR] GetCommState\n";
        return false;
    }
    
    dcb.BaudRate = baud;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fOutX = dcb.fInX = FALSE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(h, &dcb))
    {
        std::cerr << "[ERR] SetCommState\n";
        return false;
    }

    COMMTIMEOUTS to = { 0 };
    to.ReadIntervalTimeout = 50;
    to.ReadTotalTimeoutConstant = 50;
    to.ReadTotalTimeoutMultiplier = 10;
    to.WriteTotalTimeoutConstant = 50;
    to.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(h, &to);

    return true;
}

static bool writeAll(HANDLE h, const void* buf, DWORD len)
{
    DWORD written = 0;
    if (!WriteFile(h, buf, len, &written, nullptr))
    {
        return false;
    }
    return (written == len);
}

// LCP24-100SS 명령 프레임 빌더
// 프레임: STX(0x02) CH('1'~'6') Mode1 Mode2 Data(ASCII 3자리) ETX(0x03)
static std::string makeFrame(char ch, char mode1, char mode2, int value3)
{
    if (ch < '1' || ch > '6')
    {
        throw std::runtime_error("channel must be '1'..'6'");
    }
    if (value3 < 0)
    {
        value3 = 0;
    }
    if (value3 > 999)
    {
        value3 = 999;
    }
    char data[4]; std::snprintf(data, sizeof(data), "%03d", value3);
    std::string s;
    s.push_back('\x02'); // STX
    s.push_back(ch);     // 채널
    s.push_back(mode1);  // 'P' 휘도, 'T' 스트로브 시간, 'F' 트리거
    s.append(data, 3);   // 3자리 ASCII
    s.push_back(mode2);  // 보통 'R' (Remote 동작)
    s.push_back('\x03'); // ETX

    return s;
}

// 밝기(휘도) 설정: 0~240 (문서 기준)
static bool SetBrightness(HANDLE h, char channel, int level_0_240)
{
    if (level_0_240 < 0)
    {
        level_0_240 = 0;
    }
    if (level_0_240 > 240)
    {
        level_0_240 = 240;
    }
    auto frame = makeFrame(channel, 'P', 'R', level_0_240);

    return writeAll(h, frame.data(), (DWORD)frame.size());
}

// 스트로브 시간(ms) 설정: 0.1~9.99ms → 매뉴얼은 0~999(=0.01ms 단위처럼 2자리 소수 표현)
// 관례적으로 0.01ms 단위로 보정(예: 1.00ms → 100)
static bool SetStrobeTime_ms(HANDLE h, char channel, double ms)
{
    if (ms < 0.0)
    {
        ms = 0.0;
    }
    if (ms > 9.99)
    {
        ms = 9.99;
    }
    int val = (int)std::round(ms * 100.0); // 0.01ms 단위로 변환
    auto frame = makeFrame(channel, 'T', 'R', val);

    return writeAll(h, frame.data(), (DWORD)frame.size());
}

// 트리거 명령: 일부 컨트롤러는 데이터가 의미 없어서 000 사용.
// (외부 Trigger In이 아니라, RS-232로 “한 번 점등”을 발화시키고 싶을 때)
static bool fireTrigger(HANDLE h, char channel)
{
    auto frame = makeFrame(channel, 'F', 'R', 0);

    return writeAll(h, frame.data(), (DWORD)frame.size());
}

//int main()
//{
//    // 장치 관리자에서 확인된 포트로 변경하세요. 예: COM4
//    std::wstring com = L"\\\\.\\COM3"; // COM10 이상 호환을 위해 \\.\ 접두사
//    HANDLE h;
//
//    if (!openSerial(h, com))
//    {
//        std::cerr << "Failed to open serial port.\n";
//        return 1;
//    }
//    std::cout << "[OK] Opened " << std::string(com.begin(), com.end()) << "\n";
//
//    try
//    {
//        // 채널 1 예제
//        //if (!SetBrightness(h, '1', 120))
//        //{
//        //    std::cerr << "[ERR] SetBrightness\n";
//        //}
//        //if (!SetStrobeTime_ms(h, '1', 2.00))
//        //{
//        //    std::cerr << "[ERR] setStrobeTime\n";
//        //}
//        // //RS-232 소프트 트리거 한 번 발사
//        //if (!fireTrigger(h, '1'))
//        //{
//        //    std::cerr << "[ERR] fireTrigger\n";
//        //}
//        for(int i = 0; i < 3; i++)
//        //while (true)
//        {
//            fireTrigger(h, '1');
//			Sleep(50); // 100ms 간격
//            fireTrigger(h, '2');
//            Sleep(50); // 100ms 간격
//            fireTrigger(h, '3');
//            Sleep(50); // 100ms 간격
//            fireTrigger(h, '4');
//            Sleep(50); // 100ms 간격
//        }
//
//        for (int i = 0; i < 1; i++)
//        {
//            fireTrigger(h, '1');
//			Sleep(500); // 100ms 간격
//            fireTrigger(h, '2');
//            Sleep(500); // 100ms 간격
//            fireTrigger(h, '3');
//            Sleep(500); // 100ms 간격
//            fireTrigger(h, '4');
//			Sleep(500); // 100ms 간격
//        }
//    }
//    catch (const std::exception& e)
//    {
//        std::cerr << "[EXC] " << e.what() << "\n";
//    }
//
//    CloseHandle(h);
//    
//    return 0;
//}

#include "LCP24100SS.h"
#include <memory>
#include <iostream>

int main() {
    std::unique_ptr<LightController> ctrl = std::make_unique<LCP24100SS>();

    if (!ctrl->open(L"\\\\.\\COM3", 19200)) {
        std::cerr << "Open failed\n";
        return 1;
    }
    ctrl->SetBrightness('1', 120);   // 밝기 120
    ctrl->SetStrobeTime_ms('1', 2.00); // 2.00 ms
    //ctrl->Trigger('1');              // 소프트 트리거

    for (int i = 0; i < 3; i++)
    {
        ctrl->Trigger('1');
        Sleep(50); // 100ms 간격
        ctrl->Trigger('2');
        Sleep(50); // 100ms 간격
        ctrl->Trigger('3');
        Sleep(50); // 100ms 간격
        ctrl->Trigger('4');
        Sleep(50); // 100ms 간격
	}
    ctrl->close();
    return 0;
}
