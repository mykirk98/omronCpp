#include "LCP24100SS.h"

using std::max;
using std::min;

LCP24100SS::LCP24100SS()
#ifdef _WIN32
    : hSerial_(INVALID_HANDLE_VALUE)
    , open_(false)
#else
    : fd_(-1)
    , open_(false)
#endif
{
}

LCP24100SS::~LCP24100SS()
{
    close();
}

bool LCP24100SS::isOpen() const
{
    return open_;
}

bool LCP24100SS::open(const std::string& port, unsigned long baud)
{
    close();

#ifdef _WIN32
    std::wstring wport(port.begin(), port.end());
    if (wport.rfind(L"\\\\.\\", 0) != 0)
    {
        if (wport.rfind(L"COM", 0) == 0)
        {
            wport = L"\\\\.\\" + wport;
        }
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
	// No flow control
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
    fd_ = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd_ < 0)
    {
        return false;
    }
    if (!setupTermios(baud))
    {
        return false;
    }
#endif

    open_ = true;
    
    return true;
}

void LCP24100SS::close()
{
    if (!open_)
        return;

#ifdef _WIN32
    CloseHandle(hSerial_);
    hSerial_ = INVALID_HANDLE_VALUE;
#else
    ::close(fd_);
    fd_ = -1;
#endif
    open_ = false;
}

bool LCP24100SS::writeAll(const void* buf, unsigned long len)
{
    if (!open_)
        return false;

#ifdef _WIN32
    DWORD wr = 0;
    if (!WriteFile(hSerial_, buf, len, &wr, nullptr))
        return false;
    
    return wr == len;
#else
    ssize_t wr = ::write(fd_, buf, len);

    return (wr == (ssize_t)len);
#endif
}

bool LCP24100SS::writeAll(const std::string& bytes)
{
    return writeAll(bytes.data(), (unsigned long)bytes.size());
}

std::string LCP24100SS::makeFrame(char ch, char mode1, int data3) const {
    // ä��/������ ���� ����
    if (ch < '1' || ch > '6')
        ch = '1';
	if (data3 < 0)
    {
        data3 = 0;
    }
	if (data3 > 999)
    {
        data3 = 999;
    }

    char d[4];
    std::snprintf(d, sizeof(d), "%03d", data3);

    std::string s;
    s.push_back('\x02'); // STX
    s.push_back(ch);
    s.push_back(mode1);  // 'P','T','F'
    s.append(d, 3);
    s.push_back('R');    // Mode2
    s.push_back('\x03'); // ETX
    return s;
}

bool LCP24100SS::setBrightness(char channel, int data)
{
	if (data < 0)
    {
        data = 0;
    }
	if (data > 240)
    {
        data = 240;
    }

    std::string f = makeFrame(channel, 'P', data);

    return writeAll(f);
}

bool LCP24100SS::setStrobeTime_ms(char channel, double data)
{
    if (data < 0.0)
    {
        data = 0.0;
    }
    if (data > 9.99)
    {
        data = 9.99;
    }

    int ticks = (int)std::lround(data * 100.0); // 0.01ms ���� �� 0~999
    std::string f = makeFrame(channel, 'T', ticks);

    return writeAll(f);
}

bool LCP24100SS::trigger(char channel)
{
    std::string f = makeFrame(channel, 'F', 0);

    return writeAll(f);
}

#ifndef _WIN32
bool LCP24100SS::setupTermios(unsigned long baud)
{
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd_, &tty) != 0) return false;

    // 1) 완전 raw 모드로 초기화
    cfmakeraw(&tty);

    // 2) 속도 설정
    speed_t spd = B19200;
    switch (baud) {
        case 9600:  spd = B9600; break;
        case 19200: spd = B19200; break;
        // 필요 시 다른 속도도 추가
        default:    spd = B19200; break;
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

    // 5) 읽기 타임아웃 (필요시 조정). 여기서는 비차단/짧은 타임아웃
    tty.c_cc[VMIN]  = 0;  // read 최소 바이트 수
    tty.c_cc[VTIME] = 5;  // 0.5초

    // 6) 즉시 적용
    if (tcsetattr(fd_, TCSANOW, &tty) != 0) return false;

    // 7) 버퍼 비우기 (열자마자 남은 쓰레기 제거)
    tcflush(fd_, TCIOFLUSH);

    return true;
}
#endif

/*
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

    // ä��1: ��� 120, ��Ʈ�κ� 2.00ms, Ʈ���� 1ȸ
    ctrl.setBrightness('1', 120);
    ctrl.setStrobeTime_ms('1', 2.00);
    ctrl.trigger('1');

    ctrl.close();
    return 0;
}
*/