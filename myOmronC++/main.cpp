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