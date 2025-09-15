#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include "ILightController.h"

/* @brief 조명 컨트롤러 팩토리 클래스 */
class LightFactory
{
public:
	/* @brief 어떤 모델을 어떻게 생성할지 기억하는 함수 객체
	@param lowercaseOption LCP100DC 등의 옵션(대부분 모델은 무시 가능) */
    using Creator = std::function<std::shared_ptr<ILightController>(bool /*lowercaseOption*/)>;

	/* @brief 싱글톤 인스턴스 반환 메소드 */
    static LightFactory& instance();

    /* @brief 모델 등록 메소드
    @param name 등록할 모델 키 (예: "LCP100DC", "LCP24100SS", ...)
    @param creator 어댑터 생성자 */
    bool registerModel(const std::string& name, Creator creator);
	/* @brief 모델 등록 해제 메소드
	@param name 등록된 모델 키 */
    bool unregisterModel(const std::string& name);

	/* @brief 어댑터 생성 메소드
	@param name 등록된 모델 키
	@param lowercaseOption LCP100DC 등의 옵션(대부분 모델은 무시 가능) */
    std::shared_ptr<ILightController> create(const std::string& name, bool lowercaseOption = false) const;

	/* @brief 등록된 모델 키 목록 반환 메소드 */
    std::vector<std::string> registeredModels() const;

private:
    /* @brief 싱글톤 생성자 */
    LightFactory() = default;
	/* @brief 싱글톤 소멸자 */
    ~LightFactory() = default;

	/* @brief 복사 생성자 삭제 */
    LightFactory(const LightFactory&) = delete;
	/* @brief 복사 대입 연산자 삭제 */
    LightFactory& operator=(const LightFactory&) = delete;

private:
	/* @brief 동시 접근 보호 뮤텍스 */
    mutable std::mutex mutex_;
	/* @brief 모델 생성자 맵 */
    std::unordered_map<std::string, Creator> creators_map_;
};

//// 선택 사항: 정적 등록 헬퍼(전역/정적 객체로 손쉽게 등록)
//class LightFactoryRegistrar
//{
//public:
//    LightFactoryRegistrar(const std::string& name, LightFactory::Creator creator)
//    {
//        LightFactory::instance().registerModel(name, std::move(creator));
//    }
//};
