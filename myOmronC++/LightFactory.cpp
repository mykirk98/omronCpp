#include "LightFactory.h"
//#include <algorithm>

LightFactory& LightFactory::instance()
{
    static LightFactory singleton_instance; // Meyers Singleton, thread-safe in C++11+
    return singleton_instance;
}

bool LightFactory::registerModel(const std::string& name, Creator creator)
{
    std::lock_guard<std::mutex> lock(mutex_);
	// 이미 등록된 모델 찾기
    std::unordered_map<std::string, Creator>::iterator it = creators_map_.find(name);

	// 이미 등록된 모델이 있으면 false 반환
    if (it != creators_map_.end())
    {
        return false;
    }

	// 모델의 소유권 이전하여 등록
    creators_map_[name] = std::move(creator);
    return true;
}

bool LightFactory::unregisterModel(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex_);
	// 등록된 모델 찾기
    std::unordered_map<std::string, Creator>::iterator it = creators_map_.find(name);

	// 등록된 모델이 없으면 false 반환
    if (it == creators_map_.end())
    {
        return false;
    }
	// 모델 제거
    creators_map_.erase(it);
    return true;
}

std::shared_ptr<ILightController> LightFactory::create(const std::string& name, bool lowercaseOption) const
{
    std::lock_guard<std::mutex> lock(mutex_);
	// 등록된 모델 찾기
    std::unordered_map<std::string, Creator>::const_iterator it = creators_map_.find(name);

	// 등록된 모델이 없으면 nullptr 반환
    if (it == creators_map_.end())
    {
        return nullptr; // 등록되지 않은 모델
    }
	// 모델 생성자 호출
    return it->second(lowercaseOption);
}

std::vector<std::string> LightFactory::registeredModels() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> models;

	// 미리 크기 할당
    models.reserve(creators_map_.size());
    // 키 목록 복사
    for(std::unordered_map<std::string, Creator>::const_iterator it = creators_map_.begin(); it != creators_map_.end(); ++it)
    {
        models.push_back(it->first);
	}

    return models;
}
