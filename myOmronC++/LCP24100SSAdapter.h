#pragma once

#include "ILightController.h"
#include "LCP24100SS.h"

/* @brief LCP24100SS 어댑터 클래스 */
class LCP24100SSAdapter : public ILightController
{
public:
	/* @brief LCP24100SSAdapter 생성자 */
	explicit LCP24100SSAdapter()
		: pImplementation(std::make_unique<LCP24100SS>())
	{
	}
	/* @brief LCP24100SSAdapter 소멸자 */
	~LCP24100SSAdapter() override
	{
		close();
	}
	/* @brief 시리얼 포트 열기 메소드
	@param port 시리얼 포트 이름
	@param baud 보레이트 (기본값: 19200) */
	bool open(const std::string& port, unsigned long baud = 19200) override
	{
		return pImplementation->open(port, baud);
	}
	/* @brief 시리얼 포트 닫기 메소드 */
	void close() override
	{
		pImplementation->close();
	}
	/* @brief 시리얼 포트 열림 여부 */
	bool isOpen() const override
	{
		return pImplementation->isOpen();
	}
	/* @brief 밝기 설정 메소드
	@param channel 채널 설정 ('1'~'6')
	@param value 밝기 값 (0~240) */
	bool setBrightness(char channel, int value) override
	{
		return pImplementation->setBrightness(channel, value);
	}
	/* @brief 트리거 메소드
	@param channel 채널 설정 ('1'~'6')
	@param ms 대기 시간 (밀리초 단위) */
	bool trigger_ms(char channel, double ms) override
	{
		if (!pImplementation->setStrobeTime_ms(channel, ms))
		{
			return false;
		}
		return pImplementation->trigger(channel);
	}

protected:

private:
	/* @brief 실제 LCP24100SS 구현체 */
	std::unique_ptr<LCP24100SS> pImplementation;
};