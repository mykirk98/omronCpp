#pragma once
#include <string>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif // _WIN32

// 추상화된 조명 컨트롤러 인터페이스
class LightController
{
public:
	virtual ~LightController();

	// 공통: 포트 열기/닫기
	bool open(const std::wstring& port, unsigned long baud = CBR_19200);
	void close();

	// 상태
	bool isOpen() const
	{
		return is_open_;
	}

	// 모델별 구현이 필요한 기능들 (순수 가상 함수)
	virtual bool SetBrightness(char channel, int value) = 0;
	virtual bool SetStrobeTime_ms(char channel, double ms) = 0;
	virtual bool Trigger(char channel) = 0;

protected:
	LightController();

	// 파생 클래스에서 쓰는 공통 헬퍼
	bool writeAll(const void* buf, unsigned long len);
	bool writeAll(const std::string& bytes);

	// 시리얼 핸들 접근(필요시 파생 클래스에서 사용)
	HANDLE handle() const
	{
		return hSerial_;
	}

	// 타임아웃/라인세팅 재설정에 필요하면 파생 클래스에서 호출 가능
	bool configureSerial(unsigned long baud, BYTE byteSize = 8, BYTE parity = NOPARITY, BYTE stopBits = ONESTOPBIT) const;

private:
	//HANDLE hSerial_;
#ifdef _WIN32
	HANDLE hSerial_;  // HANDLE
#else
	int fd_;         // file descriptor
#endif // _WIN32
	bool is_open_;
};