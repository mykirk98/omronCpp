#include "LightController.h"
#include <iostream>

LightController::LightController()
	: hSerial_(INVALID_HANDLE_VALUE)
	, is_open_(false)
{
}

LightController::~LightController()
{
	close();
}

bool LightController::open(const std::wstring& port, unsigned long baud)
{
	close(); // 이미 열려있으면 닫기

	// COM10 이상도 안전하게 열기 위해 \\.\ 사용 권장
	//NOTE: COM : serial communication(직렬 통신)을 의미
	std::wstring dev = port;
	if (dev.rfind(L"\\\\.\\") != 0)
	{
		// 접두사가 없고, "COMx" 형태면 "\\.\" 접두사 추가
		if (dev.rfind(L"COM", 0) == 0)
		{
			dev = L"\\\\.\\" + dev;
		}
	}

	hSerial_ = CreateFileW(dev.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hSerial_ == INVALID_HANDLE_VALUE)
	{
		std::cerr << "[LightController] CreateFile failed. GetLastError=" << GetLastError() << "\n";
		return false;
	}

	if (!configureSerial(baud))
	{
		std::cerr << "[LightController] configureSerial failed.\n";
		CloseHandle(hSerial_);
		hSerial_ = INVALID_HANDLE_VALUE;
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

	is_open_ = true;

	return true;
}

void LightController::close()
{
	if (is_open_)
	{
		CloseHandle(hSerial_);
		hSerial_ = INVALID_HANDLE_VALUE;
		is_open_ = false;
	}
}

bool LightController::configureSerial(unsigned long baud, BYTE byteSize, BYTE parity, BYTE stopBits)
{
	DCB dcb = { 0 };		//NOTE: DCB: Device Control Block
	dcb.DCBlength = sizeof(DCB);

	if (!GetCommState(hSerial_, &dcb))
	{
		return false;
	}

	dcb.BaudRate = baud;
	dcb.ByteSize = byteSize;
	dcb.Parity = parity;
	dcb.StopBits = stopBits;

	// 흐름 제어 비활성
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fOutX = dcb.fInX = FALSE;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;

	// 설정 적용
	return !!SetCommState(hSerial_, &dcb);
}

bool LightController::writeAll(const void* buf, unsigned long len)
{
	if (!is_open_)
	{
		return false;
	}
	DWORD written = 0;

	if (!WriteFile(hSerial_, buf, len, &written, nullptr))
	{
		return false;
	}

	return (written == len);
}

bool LightController::writeAll(const std::string& bytes)
{
	return writeAll(bytes.data(), static_cast<unsigned long>(bytes.size()));
}