#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <iostream>
#include "ILightController.h"
#include "LightFactory.h"

/* @brief 조명 컨트롤러 관리 클래스 */
class LightManager
{
public:
	/* @brief 컨트롤러 사양 구조체 */
    struct ControllerSpec
    {
        std::string id;                // 컨트롤러 식별자
		std::string  model;             // 모델 이름 (LightFactory에 등록된 이름)
		std::string  port;              // 시리얼 포트 이름 (예: "COM3", "/dev/ttyUSB0")
		unsigned long baud = 19200;     // 보레이트 (기본값: 19200)
		bool lowercaseOption = false;   // 모델 옵션 (예: LCP100DC의 소문자 옵션)
    };

public:
	/* @brief 생성자 */
	explicit LightManager();
    /* @brief 소멸자 */
    ~LightManager();

	/* @brief 컨트롤러 추가 메소드
	@param spec 컨트롤러 사양 구조체 */
    bool addController(const ControllerSpec& spec);
	/* @brief 컨트롤러 제거 메소드
	@param id 컨트롤러 식별자 */
    void removeController(const std::string& id);

	/* @brief 컨트롤러 열기 메소드
	@param id 컨트롤러 식별자 */
    bool open(const std::string& id);

	/* @brief 컨트롤러 닫기 메소드
	@param id 컨트롤러 식별자 */
    void close(const std::string& id);
	/* @brief 모든 컨트롤러 열기 메소드 */
    void openAll();
	/* @brief 모든 컨트롤러 닫기 메소드 */
    void closeAll();

	/* @brief 밝기 설정 메소드
	@param id 컨트롤러 식별자
	@param channel 채널 설정
	@param value 밝기 값 */
    bool setBrightness(const std::string& id, char channel, int value);
	/* @brief 트리거 메소드
	@param id 컨트롤러 식별자
	@param channel 채널 설정
	@param ms 대기 시간 (밀리초 단위) */
    bool trigger(const std::string& id, char channel, double ms);

private:
	/* @brief 컨트롤러 조회 메소드
	@param id 컨트롤러 식별자 */
    std::shared_ptr<ILightController> get(const std::string& id);

private:
	/* @brief 컨트롤러 맵 (식별자 -> 컨트롤러) */
    std::unordered_map<std::string, std::shared_ptr<ILightController>> controller_map_;
	/* @brief 컨트롤러 설정 정보 맵 (식별자 -> 사양) */
    std::unordered_map<std::string, ControllerSpec> controller_spec_map_;
	/* @brief 동시 접근 보호 뮤텍스 */
    std::mutex mutex_;
};
