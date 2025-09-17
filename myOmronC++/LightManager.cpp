#include "LightManager.h"
#include <iostream>

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
    if (controller_map_.count(spec.id))
    {
        std::cerr << "[LightManager] Controller already exists: " << spec.id << std::endl;
        return false;
    }

	std::shared_ptr<ILightController> controller = LightFactory::instance().create(spec.model, spec.lowercaseOption);
    if (!controller)
    {
        std::cerr << "[LightManager] Unknown model: " << spec.model << std::endl;
        return false;
    }

    controller_map_[spec.id] = controller;
    controller_spec_map_[spec.id] = spec;
    return true;
}


void LightManager::removeController(const std::string& id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
	std::unordered_map<std::string, std::shared_ptr<ILightController>>::iterator it = controller_map_.find(id);
    if (it != controller_map_.end())
    {
        it->second->close();
        controller_map_.erase(it);
    }
    controller_spec_map_.erase(id);
}


bool LightManager::open(const std::string& id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::unordered_map<std::string, std::shared_ptr<ILightController>>::iterator it = controller_map_.find(id);
    if (it == controller_map_.end())
    {
        return false;
    }

	std::unordered_map<std::string, ControllerSpec>::iterator specIt = controller_spec_map_.find(id);
    if (specIt == controller_spec_map_.end())
    {
        return false;
    }

    return it->second->open(specIt->second.port, specIt->second.baud);
}


void LightManager::close(const std::string& id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::unordered_map<std::string, std::shared_ptr<ILightController>>::iterator it = controller_map_.find(id);
    if (it != controller_map_.end())
    {
        it->second->close();
    }
}


void LightManager::openAll()
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (std::unordered_map<std::string, std::shared_ptr<ILightController>>::iterator it = controller_map_.begin(); it != controller_map_.end(); ++it)
    {
        const ControllerSpec& spec = controller_spec_map_[it->first];
        it->second->open(spec.port, spec.baud);
    }
}


void LightManager::closeAll()
{
    std::lock_guard<std::mutex> lock(mutex_);
    
	for (std::unordered_map<std::string, std::shared_ptr<ILightController>>::iterator it = controller_map_.begin(); it != controller_map_.end(); ++it)
    {
        it->second->close();
    }
}


bool LightManager::setBrightness(const std::string& id, char channel, int value)
{
    std::shared_ptr<ILightController> ctl = get(id);

    if (!ctl) return false;
    return ctl->setBrightness(channel, value);
}


bool LightManager::trigger(const std::string& id, char channel, double ms)
{
    std::shared_ptr<ILightController> ctl = get(id);

    if (!ctl) return false;
    return ctl->trigger_ms(channel, ms);
}


std::shared_ptr<ILightController> LightManager::get(const std::string& id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
	std::unordered_map<std::string, std::shared_ptr<ILightController>>::iterator it = controller_map_.find(id);
    if (it == controller_map_.end())
    {
        return nullptr;
    }
    return it->second;
}


/* 
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