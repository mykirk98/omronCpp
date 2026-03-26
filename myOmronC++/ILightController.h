#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

/* @brief 조명 컨트롤러 공통 인터페이스 */
class ILightController
{
public:
	/* @brief 가상 소멸자 */
    virtual ~ILightController() = default;

	/* @brief 시리얼 포트 열기 메소드
	@param port 시리얼 포트 이름
	@param baud 보레이트 */
    virtual bool open(const std::string& port, unsigned long baud) = 0;
    /* @brief 시리얼 포트 닫기 메소드 */
    virtual void close() = 0;
	/* @brief 시리얼 포트 열림 여부 */
    virtual bool isOpen() const = 0;

	/* @brief 밝기 설정 메소드
    @param channel 채널 설정
	@param value 밝기 값 */
    virtual bool setBrightness(char channel, int value) = 0;

	/* @brief 트리거 메소드
	@param channel 채널 설정
	@param ms 대기 시간 (밀리초 단위) */
    virtual bool trigger_ms(char channel, double ms) = 0;

protected:

private:
};