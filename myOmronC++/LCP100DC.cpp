#include "LCP100DC.h"
#include <cstdio>
#include <cmath>

std::string LCP100DC::makeFrame8(char ch, char cmd, int data0000_0100) const {
    // 채널 유효성 (장비 기준: '1'~'2', 전체 'Z'/'z')
    if (!((ch >= '1' && ch <= '2') || ch == 'Z' || ch == 'z')) ch = '1';

    data0000_0100 = clamp(data0000_0100, 0, 100);
    char data[5]; std::snprintf(data, sizeof(data), "%04d", data0000_0100);

    std::string s;
    s.push_back('\x02');      // STX
    s.push_back(ch);          // CH
    s.push_back(cmd);         // CMD
    s.append(data, 4);        // DATA "0000"~"0100"
    s.push_back('\x03');      // ETX
    return s;
}

std::string LCP100DC::makeFrame4(char ch, char cmd) const {
    if (!((ch >= '1' && ch <= '2') || ch == 'Z' || ch == 'z')) ch = '1';
    std::string s;
    s.push_back('\x02');
    s.push_back(ch);
    s.push_back(cmd);
    s.push_back('\x03');
    return s;
}

bool LCP100DC::setBrightness(char channel, int percent_0_100) {
    // CMD: 밝기 변경 'D' 또는 'd', DATA: "0000"~"0100"
    char cmd = upDown('D', 'd', opt_.useLowercase);
    auto frame = makeFrame8(channel, cmd, percent_0_100);
    return writeAll(frame);
}

bool LCP100DC::turnOn(char channel) {
    // CMD: ON 'O' 또는 'o' (4바이트형)
    char cmd = upDown('O', 'o', opt_.useLowercase);
    auto frame = makeFrame4(channel, cmd);
    return writeAll(frame);
}

bool LCP100DC::turnOff(char channel) {
    // CMD: OFF 'F' 또는 'f' (4바이트형)
    char cmd = upDown('F', 'f', opt_.useLowercase);
    auto frame = makeFrame4(channel, cmd);
    return writeAll(frame);
}

bool LCP100DC::trigger_ms(char channel, double ms) {
    // RS-232 소프트 트리거: ON → ms 대기 → OFF
    if (!turnOn(channel)) return false;
    if (ms > 0) {
        auto dur = std::chrono::duration<double, std::milli>(ms);
        std::this_thread::sleep_for(dur);
    }
    return turnOff(channel);
}

bool LCP100DC::requestBrightness(char channel) {
    // 매뉴얼 예시 스크린샷에 “CH1 밝기 리턴: STX '1' 'W' "0100" ETX”로 보임
    // 일부 장비는 'W'만 보내면 현재값을 돌려주거나, 혹은 'W' + 더미 DATA를 요구할 수 있음.
    // 호환성을 위해 DATA=0000로 보냄. 필요시 실제 스펙대로 수정.
    char cmd = upDown('W', 'w', opt_.useLowercase);
    auto frame = makeFrame8(channel, cmd, 0);
    return writeAll(frame);
}
