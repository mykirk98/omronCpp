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

	/* @brief 시리얼 포트 열기 메소드
	@param port 시리얼 포트 이름
	@param baud 보레이트 (기본값: 19200) */
    bool open(const std::string& port, unsigned long baud = 19200);
	/* @brief 시리얼 포트 닫기 메소드 */
    void close();
	/* @brief 시리얼 포트 열림 여부 */
    bool isOpen() const;

	/* @brief 밝기 설정 메소드
	@param channel 채널 설정
	@param data 밝기 값 (0~255) */
    bool setBrightness(char channel, int data);
	/* @brief 스트로브 시간 설정 메소드
	@param channel 채널 설정
	@param data 스트로브 시간 (0.00~9.99 ms) */
    bool setStrobeTime_ms(char channel, double data);
	/* @brief 트리거 신호 발생 메소드
	@param channel 채널 설정 */
    bool trigger(char channel);

private:
#ifdef _WIN32
    void* hSerial_; // HANDLE
#else
    int   fd_;      // file descriptor
#endif
    bool  open_;

	/* @brief 시리얼 포트에 모든 데이터를 쓸 때까지 반복해서 쓰는 함수
	@param buf 쓸 데이터 버퍼
	@param len 버퍼 길이 (바이트 단위) */
    bool writeAll(const void* buf, unsigned long len);
	/* @brief 시리얼 포트에 문자열 데이터를 쓸 때까지 반복해서 쓰는 함수
	@param bytes 쓸 문자열 데이터 */
    bool writeAll(const std::string& bytes);
	/* @brief 프레임 생성기
	@param ch 채널 문자
	@param mode1 명령 모드
	@param data3 데이터 값 */
    std::string makeFrame(char ch, char mode1, int data3) const;

#ifndef _WIN32
	/* @brief termios 설정 메소드
	@param baud 보레이트 */
    bool setupTermios(unsigned long baud);
#endif
};
