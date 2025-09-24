#include "LightManager.h"

LightManager::LightManager()
{
}
LightManager::~LightManager()
{
    closeAll();
}

bool LightManager::addController(const ControllerSpec& spec)
{
    std::lock_guard<std::mutex> lock(mutex_);
	// 컨트롤러 ID 중복 검사
    if (controller_map_.count(spec.id))
    {
        std::cerr << "[LightManager] Controller already exists: " << spec.id << std::endl;
        return false;
    }

	// 팩토리를 통해 컨트롤러 생성
	std::shared_ptr<ILightController> controller = LightFactory::instance().create(spec.model, spec.lowercaseOption);
    if (!controller)
    {
        std::cerr << "[LightManager] Unknown model: " << spec.model << std::endl;
        return false;
    }
	// 컨트롤러 맵과 설정 정보 맵에 추가
    controller_map_[spec.id] = controller;
    controller_spec_map_[spec.id] = spec;
    return true;
}

void LightManager::removeController(const std::string& id)
{
    std::lock_guard<std::mutex> lock(mutex_);
	// 컨트롤러가 열려 있으면 닫고 맵에서 제거
	std::unordered_map<std::string, std::shared_ptr<ILightController>>::iterator it = controller_map_.find(id);
    if (it != controller_map_.end())
    {
        it->second->close();
        controller_map_.erase(it);
    }
	// 설정 정보 맵에서 제거
    controller_spec_map_.erase(id);
}

bool LightManager::open(const std::string& id)
{
    std::lock_guard<std::mutex> lock(mutex_);
	// 컨트롤러가 존재하지 않으면 실패
    std::unordered_map<std::string, std::shared_ptr<ILightController>>::iterator it = controller_map_.find(id);
    if (it == controller_map_.end())
    {
        return false;
    }
	// 설정 정보가 존재하지 않으면 실패
	std::unordered_map<std::string, ControllerSpec>::iterator specIt = controller_spec_map_.find(id);
    if (specIt == controller_spec_map_.end())
    {
        return false;
    }
    // 컨트롤러 시리얼 포트 열기
    return it->second->open(specIt->second.port, specIt->second.baud);
}

void LightManager::close(const std::string& id)
{
    std::lock_guard<std::mutex> lock(mutex_);
	// 컨트롤러 시리얼 포트 닫기
    std::unordered_map<std::string, std::shared_ptr<ILightController>>::iterator it = controller_map_.find(id);
    if (it != controller_map_.end())
    {
        it->second->close();
    }
}

void LightManager::openAll()
{
    std::lock_guard<std::mutex> lock(mutex_);
	// 모든 컨트롤러 시리얼 포트 열기
    for (std::unordered_map<std::string, std::shared_ptr<ILightController>>::iterator it = controller_map_.begin(); it != controller_map_.end(); ++it)
    {
        const ControllerSpec& spec = controller_spec_map_[it->first];
        it->second->open(spec.port, spec.baud);
    }
}

void LightManager::closeAll()
{
    std::lock_guard<std::mutex> lock(mutex_);
	// 모든 컨트롤러 시리얼 포트 닫기
	for (std::unordered_map<std::string, std::shared_ptr<ILightController>>::iterator it = controller_map_.begin(); it != controller_map_.end(); ++it)
    {
        it->second->close();
    }
}

bool LightManager::setBrightness(const std::string& id, char channel, int value)
{
    std::shared_ptr<ILightController> controller = get(id);
	// 컨트롤러가 존재하지 않으면 실패
    if (!controller)
    {
        return false;
    }
	// 밝기 설정
    return controller->setBrightness(channel, value);
}

bool LightManager::trigger(const std::string& id, char channel, double ms)
{
    std::shared_ptr<ILightController> controller = get(id);
	// 컨트롤러가 존재하지 않으면 실패
    if (!controller)
    {
        return false;
    }
    return controller->trigger_ms(channel, ms);
}

std::shared_ptr<ILightController> LightManager::get(const std::string& id)
{
    std::lock_guard<std::mutex> lock(mutex_);
	// 컨트롤러가 존재하지 않으면 nullptr 반환
	std::unordered_map<std::string, std::shared_ptr<ILightController>>::iterator it = controller_map_.find(id);
    if (it == controller_map_.end())
    {
        return nullptr;
    }
	// 컨트롤러 반환
    return it->second;
}

/* 
#include "LightManager.h"
#include "LCP100DCAdapter.h"
#include "LCP24100SSAdapter.h"

int main()
{
    LightManager mgr;

    // 모델 등록 (보통 프로그램 초기화 단계에서 수행)
    LightFactory::instance().registerModel("LCP100DC",
        [](bool lowercase) { return std::make_shared<LCP100DCAdapter>(lowercase); });
    LightFactory::instance().registerModel("LCP24100SS",
        [](bool) { return std::make_shared<LCP24100SSAdapter>(); });

    // 컨트롤러 추가
    mgr.addController({ "light1", "LCP100DC", "COM5", 19200, true });
    mgr.addController({ "light2", "LCP24100SS", "COM3", 19200 });

    // 열기
    mgr.openAll();

    // 제어
    mgr.setBrightness("light1", '1', 80);
    mgr.trigger("light2", '1', 5.0);
    mgr.trigger("light1", '1', 20.0);

    // 닫기
    mgr.closeAll();
}
*/