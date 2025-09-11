#include "LightController.h"

LightController::LightController()
#ifdef _WIN32
	: hSerial_(INVALID_HANDLE_VALUE)
	, is_open_(false)
#else
	, fd_(-1)
	, is_open_(false)
#endif // _WIN32
{
}

LightController::~LightController()
{
	close();
}

bool LightController::open(const std::string& port, unsigned long baud)
{
	close(); // 이미 열려있으면 닫기

	// COM10 이상도 안전하게 열기 위해 \\.\ 사용 권장
	//NOTE: COM : serial communication(직렬 통신)을 의미
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
		std::cerr << "[LightController] CreateFile failed. Error=" << GetLastError() << "\n";
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

	if (!SetCommState(hSerial_, &dcb))
	{
		return false;
	}

	// 타임아웃 설정
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
		std::cerr << "[LightController] open() failed: " << strerror(errno) << "\n";
		return false;
	}

	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr(fd_, &tty) != 0)
	{
		std::cerr << "[LightController] tcgetattr failed\n";
		return false;
	}

	cfsetospeed(&tty, baud == 19200 ? B19200 : B9600);
	cfsetispeed(&tty, baud == 19200 ? B19200 : B9600);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
	tty.c_iflag &= ~IGNBRK;
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 5;
	tty.c_cflag |= (CLOCAL | CREAD);
	tty.c_cflag &= ~(PARENB | PARODD); // no parity
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr(fd_, TCSANOW, &tty) != 0)
	{
		std::cerr << "[LightController] tcsetattr failed\n";
		return false;
	}
#endif // _WIN32

	is_open_ = true;

	return true;
}

void LightController::close()
{
	if (is_open_)
	{
#ifdef _WIN32
		CloseHandle(hSerial_);
		hSerial_ = INVALID_HANDLE_VALUE;
#else
		::close(fd_);
		fd_ = -1;
#endif // _WIN32
		
		is_open_ = false;
	}
}

bool LightController::writeAll(const void* buf, unsigned long len) const
{
	if (!is_open_)
	{
		return false;
	}
#ifdef _WIN32
	DWORD written = 0;
	if (!WriteFile(hSerial_, buf, len, &written, nullptr))
	{
		return false;
	}
	return (written == len);
#else
	ssize_t written = ::write(fd_, buf, len);
	return (written == (ssize_t)len);
#endif // _WIN32
}

bool LightController::writeAll(const std::string& bytes)
{
	return writeAll(bytes.data(), static_cast<unsigned long>(bytes.size()));
}