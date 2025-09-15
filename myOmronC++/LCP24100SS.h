#pragma once
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#endif

/* @brief LCP24100SS 조명 컨트롤러 RS-232 제어 클래스 */
class LCP24100SS {
public:
	/* @brief LCP24100SS 생성자 */
    LCP24100SS();
	/* @brief LCP24100SS 소멸자 */
    ~LCP24100SS();

	/* @brief 포트 열기
	@param port 포트 이름
	@param baud 보레이트 (기본값: 19200) */
    bool open(const std::string& port, unsigned long baud = 19200);
	/* @brief 포트 닫기 */
    void close();
	/* @brief 포트 열림 여부 */
    bool isOpen() const;
	/* @brief 조명 밝기 설정 메소드
	@param channel 채널 설정 ('1'~'6')
	@param data 밝기 값 (0~255) */
	bool setBrightness(char channel, int data);
	/* @brief 스트로브 타임 설정 메소드
	@param channel 채널 설정 ('1'~'6')
	@param data 스트로브 타임 (0.00~9.99ms) */
	bool setStrobeTime_ms(char channel, double data);
	/* @brief 트리거 메소드 */
	bool trigger(char channel);

private:
#ifdef _WIN32
    void* hSerial_; // HANDLE
#else
    int   fd_;      // file descriptor
#endif // _WIN32
    bool  open_;

	/*
	@brief 
	*/
	bool writeAll(const void* buf, unsigned long len);
	bool writeAll(const std::string& bytes);
	std::string makeFrame(char ch, char mode1, int data3) const;

#ifndef _WIN32
	bool setupTermios(unsigned long baud);
#endif
};
