#pragma once
#include <string>
#include <chrono>
#include <thread>
#include <cstdio>
#include <cstring>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#endif

/* @brief LCP100DC 조명 컨트롤러 RS-232 제어 클래스 */
class LCP100DC {
public:
	/* @brief LCP100DC 생성자 */
    explicit LCP100DC(bool useLowercase = false);
	/* @brief LCP100DC 소멸자 */
    ~LCP100DC();

	/* @brief 시리얼 포트 열기 메소드
	@param port 시리얼 포트 이름
	@param baud 보레이트 (기본값: 19200) */
    bool open(const std::string& port, unsigned long baud = 19200);
	/* @brief 시리얼 포트 닫기 메소드 */
    void close();
	/* @brief 시리얼 포트 열림 여부 */
    bool isOpen() const;
	/* @brief 소문자/대문자 명령 전환 */
	void setUseLowercase(bool v) { lowercase_ = v; }
	/* @brief 밝기 설정 메소드
	@param channel 채널 설정 ('1'~'2', 'Z'/'z':전체)
	@param data 밝기 값 (0~100) */
    bool setBrightness(char channel, int data);

	/* @brief turnOn 조명 ON 메소드
	@param channel 채널 설정 ('1'~'2', 'Z'/'z':전체) */
    bool turnOn(char channel);
	/* @brief turnOff 조명 OFF 메소드
	@param channel 채널 설정 ('1'~'2', 'Z'/'z':전체) */
    bool turnOff(char channel);

	/* @brief 조명 ON -> 대기(ms) -> OFF 메소드 */
    bool trigger_ms(char channel, double ms);

private:
#ifdef _WIN32
    void* hSerial_;  // HANDLE
#else
    int   fd_;       // file descriptor
#endif
    bool open_;
	bool lowercase_;
	
	/* @brief 시리얼 포트에 모든 데이터를 쓸 때까지 반복해서 쓰는 함수
	@param buffer 쓸 데이터 버퍼
	@param len 버퍼 길이 (바이트 단위) */
    bool writeAll(const void* buffer, unsigned long len);
	/* @brief 시리얼 포트에 문자열 데이터를 쓸 때까지 반복해서 쓰는 함수
	@param bytes 쓸 문자열 데이터 */
    bool writeAll(const std::string& bytes);
	/* @brief 8바이트 프레임 생성 (STX CH CMD DATA(4) ETX)
	@param ch 채널 ('1'~'2', 'Z'/'z':전체)
	@param cmd 명령 문자 ('D'/'d':밝기설정)
	@param data 데이터 (0~100) */
    std::string makeFrame8(char ch, char cmd, int data) const;
	/* @brief 4바이트 프레임 생성 (STX CH CMD ETX)
	@param ch 채널 ('1'~'2', 'Z'/'z':전체)
	@param cmd 명령 문자 ('O'/'o':ON, 'F'/'f':OFF) */
    std::string makeFrame4(char ch, char cmd) const;
#ifndef _WIN32
	/* @brief POSIX termios 설정 함수
	@param baud 보레이트 */
    bool setupTermios(unsigned long baud);
#endif

	static inline char pickCase(char upper, char lower, bool useLower)
	{
		return useLower ? lower : upper;
	}
};
