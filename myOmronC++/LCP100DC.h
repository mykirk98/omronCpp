#pragma once
#include "LightController.h"
#include <string>
#include <chrono>
#include <thread>

// LCP100DC: RS-232로 밝기/ON/OFF 제어 (프로토콜은 매뉴얼 4. Protocol 참조)
class LCP100DC : public LightController {
public:
    // 대문자/소문자 모두 허용되지만 기본은 대문자 사용
    struct Options {
        bool useLowercase = false; // true면 'd','o','f' 사용
    };

    explicit LCP100DC(const Options& opt = {}) : opt_(opt) {}

    // base 인터페이스 구현
    bool setBrightness(char channel, int percent_0_100) override; // "0000"~"0100"으로 전송
    bool setStrobeTime_ms(char, double) override { return false; } // 이 모델은 스트로브 시간 명령 없음
    bool trigger(char channel) override { return trigger_ms(channel, 5.0); } // ON→지연→OFF(소프트 트리거)

    // 명시적 제어
    bool turnOn(char channel);
    bool turnOff(char channel);
    bool trigger_ms(char channel, double ms);

    // (선택) 현재 밝기 리턴 요청 (문서 예시의 'W'가 “밝기 리턴”으로 보임)
    bool requestBrightness(char channel);

private:
    Options opt_;

    // 프레임 빌더
    std::string makeFrame8(char ch, char cmd, int data0000_0100) const; // STX CH CMD DATA(4) ETX
    std::string makeFrame4(char ch, char cmd) const;                   // STX CH CMD ETX

    // 유틸
    static inline char upDown(char upper, char lower, bool lowerMode) { return lowerMode ? lower : upper; }
    static inline int  clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
};
