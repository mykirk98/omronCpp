#include "LCP100DC.h"

LCP100DC::LCP100DC(bool useLowercase)
#ifdef _WIN32
    : hSerial_(INVALID_HANDLE_VALUE)
    , open_(false)
	, lowercase_(useLowercase)
#else
    : fd_(-1)
    , open_(false)
	, lowercase_(useLowercase)
#endif
{
}

LCP100DC::~LCP100DC()
{
    close();
}

bool LCP100DC::isOpen() const
{
    return open_;
}

bool LCP100DC::open(const std::string& port, unsigned long baud)
{
    close();
#ifdef _WIN32
    std::wstring wport(port.begin(), port.end());
    if (wport.rfind(L"\\\\.\\", 0) != 0)
    {
        if (wport.rfind(L"COM", 0) == 0)
            wport = L"\\\\.\\" + wport;
    }
    hSerial_ = CreateFileW(wport.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hSerial_ == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(hSerial_, &dcb))
    {
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
    
    if (!SetCommState(hSerial_, &dcb))
    {
        return false;
    }

    COMMTIMEOUTS to = { 0 };
    to.ReadIntervalTimeout = 50;
    to.ReadTotalTimeoutConstant = 50;
    to.ReadTotalTimeoutMultiplier = 10;
    to.WriteTotalTimeoutConstant = 50;
    to.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial_, &to);
#else
fd_ = ::open(port.c_str(), O_RDWR | O_NOCTTY /* | O_SYNC */);
    if (fd_ < 0)
    {
        fprintf(stderr, "[LCP100DC] ::open('%s') failed: %s (errno=%d)\n", port.c_str(), strerror(errno), errno);
        
        return false;
    }
    if (!setupTermios(baud))
    {
        fprintf(stderr, "[LCP100DC] setupTermios(%lu) failed: %s (errno=%d)\n", baud, strerror(errno), errno);
        ::close(fd_);
        fd_ = -1;

        return false;
    }
#endif
    open_ = true;
    
    return true;
}

void LCP100DC::close()
{
    if (!open_)
    {
        return;
    }

#ifdef _WIN32
    CloseHandle(hSerial_);
    hSerial_ = INVALID_HANDLE_VALUE;
#else
    ::close(fd_);
    fd_ = -1;
#endif
    open_ = false;
}

bool LCP100DC::writeAll(const void* buffer, unsigned long len)
{
    if (!open_)
    {
        return false;
    }

#ifdef _WIN32
    DWORD wr = 0;
    if (!WriteFile(hSerial_, buffer, len, &wr, nullptr))
    {
        return false;
    }
    return wr == len;
#else
    ssize_t wr = ::write(fd_, buffer, len);
    return (wr == (ssize_t)len);
#endif
}
bool LCP100DC::writeAll(const std::string& bytes)
{
    return writeAll(bytes.data(), (unsigned long)bytes.size());
}

std::string LCP100DC::makeFrame8(char ch, char cmd, int data) const
{
	// CH : '1'~'2', 'Z'/'z':전체
    if (!((ch >= '1' && ch <= '2') || ch == 'Z' || ch == 'z'))
    {
        ch = '1';
    }

	if (data < 0) 
    {
        data = 0;
    }
	if (data > 100) 
    {
        data = 100;
    }

    char d[5];
    std::snprintf(d, sizeof(d), "%04d", data);
    std::string s;
    s.push_back('\x02');  // STX
    s.push_back(ch);      // CH
    s.push_back(cmd);     // CMD (D/d)
    s.append(d, 4);    // DATA "0000"~"0100"
    s.push_back('\x03');  // ETX

    return s;
}

std::string LCP100DC::makeFrame4(char ch, char cmd) const
{
    if (!((ch >= '1' && ch <= '2') || ch == 'Z' || ch == 'z'))
    {
        ch = '1';
    }

    std::string s;
	s.push_back('\x02');    // STX
	s.push_back(ch);        // CH
	s.push_back(cmd);       // CMD (O/o, F/f)
	s.push_back('\x03');    // ETX

    return s;
}

bool LCP100DC::setBrightness(char channel, int data)
{
    const char cmd = pickCase('D', 'd', lowercase_);

    return writeAll(makeFrame8(channel, cmd, data));
}

bool LCP100DC::turnOn(char channel)
{
    const char cmd = pickCase('O', 'o', lowercase_);

    return writeAll(makeFrame4(channel, cmd));
}

bool LCP100DC::turnOff(char channel)
{
    const char cmd = pickCase('F', 'f', lowercase_);

    return writeAll(makeFrame4(channel, cmd));
}

bool LCP100DC::trigger_ms(char channel, double ms)
{
    if (!turnOn(channel))
    {
        return false;
    }

    if (ms > 0)
    {
        auto dur = std::chrono::duration<double, std::milli>(ms);
        std::this_thread::sleep_for(dur);
    }

    return turnOff(channel);
}

#ifndef _WIN32
bool LCP100DC::setupTermios(unsigned long baud)
{
struct termios tty;
    memset(&tty, 0, sizeof tty);
    
    if (tcgetattr(fd_, &tty) != 0)
    {
        return false;
    }

    // 1) 완전 raw 모드로 초기화
    cfmakeraw(&tty);

    // 2) 속도 설정
    speed_t spd = B19200;
    switch (baud)
    {
        case 9600:
            spd = B9600;
            break;
        case 19200:
            spd = B19200;
            break;
        // 필요 시 다른 속도도 추가
        default:
            spd = B19200;
            break;
    }

    cfsetospeed(&tty, spd);
    cfsetispeed(&tty, spd);

    // 3) 8N1, 로컬/수신 활성화
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;              // 8 data bits
    tty.c_cflag &= ~PARENB;          // no parity
    tty.c_cflag &= ~CSTOPB;          // 1 stop bit
    tty.c_cflag |= (CLOCAL | CREAD); // local & enable receiver

    // 4) 하드/소프트 플로우 컨트롤 모두 해제
    tty.c_cflag &= ~CRTSCTS;                     // HW flow off
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);      // SW flow off

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 5;

    // 6) 즉시 적용
    if (tcsetattr(fd_, TCSANOW, &tty) != 0)
    {
        return false;
    }

    // 7) 버퍼 비우기 (열자마자 남은 쓰레기 제거)
    tcflush(fd_, TCIOFLUSH);

    return true;
}
#endif

/*
#include "LCP100DC.h"
#include <iostream>

int main() {
    LCP100DC ctrl;

#ifdef _WIN32
    if (!ctrl.open("COM5", 19200))
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
	// 60% 밝기 설정 후 30ms 트리거
    ctrl.setBrightness('1', 60);
    ctrl.trigger_ms('1', 30.0);

	// 전체 끄기
    ctrl.turnOff('Z');

    ctrl.close();
    return 0;
}
*/