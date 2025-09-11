#include "LCP24100SS.h"
#include <cmath>

bool LCP24100SS::SetBrightness(char channel, int value)
{
	if (value < 0)
	{
		value = 0;
	}
	else if (value > 240)
	{
		value = 240;
	}

	std::string frame = makeFrame(channel, 'P', value);

	return writeAll(frame);
}

bool LCP24100SS::SetStrobeTime_ms(char channel, double ms)
{
	if (ms < 0.1)
	{
		ms = 0.1;
	}
	else if (ms > 9.99)
	{
		ms = 9.99;
	}

	int val = static_cast<int>(std::round(ms * 100.0));
	std::string frame = makeFrame(channel, 'T', val);

	return writeAll(frame);
}

bool LCP24100SS::Trigger(char channel)
{
	std::string frame = makeFrame(channel, 'F', 0);

	return writeAll(frame);
}

std::string LCP24100SS::makeFrame(char ch, char model, int value3) const	// 뒤의 const는 멤버 변수를 변경하지 않음을 의미
{
	if (ch < '1' || ch > '6')
	{
		ch = '1'; // 기본값
	}
	if (value3 < 0)
	{
		value3 = 0;
	}
	if (value3 > 999)
	{
		value3 = 999;
	}

	char data[4];
	std::snprintf(data, sizeof(data), "%03d", value3);	// zero-padded 3자리 ASCII

	std::string s;
	s.push_back('\x02'); // STX
	s.push_back(ch);     // 채널
	s.push_back(model);  // 'P' 휘도, 'T' 스트로브 시간, 'F' 트리거
	s.append(data, 3);   // 3자리 ASCII
	s.push_back('R');    // 보통 'R' (Remote 동작)
	s.push_back('\x03'); // ETX

	return s;
}

/*	// Example usage
#include "LCP24100SS.h"

int main() {
	std::unique_ptr<LightController> ctrl = std::make_unique<LCP24100SS>();

#ifdef _WIN32
	// COM10 이상이면 "\\\\.\\COM10" 형태 권장
	if (!ctrl->open("COM3", 19200))
	{
		std::cerr << "open failed\n";
		return 1;
	}
#else
	// Ubuntu 예시: dmesg | grep tty 로 포트 확인 (/dev/ttyUSB0 등)
	if (!ctrl->open("/dev/ttyUSB0", 19200))
	{
		std::cerr << "open failed\n";
		return 1;
	}
#endif
	ctrl->SetBrightness('1', 120);   // 밝기 120
	ctrl->SetStrobeTime_ms('1', 2.00); // 2.00 ms
	//ctrl->Trigger('1');              // 소프트 트리거

	for (int i = 0; i < 10; i++)
	{
		ctrl->Trigger('1');
		//Sleep(50); // 100ms 간격
		ctrl->Trigger('2');
		//Sleep(50); // 100ms 간격
		ctrl->Trigger('3');
		//Sleep(50); // 100ms 간격
		ctrl->Trigger('4');
		//Sleep(50); // 100ms 간격
		//ctrl->Trigger('3');
		//Sleep(50); // 100ms 간격
		//ctrl->Trigger('2');
		Sleep(500); // 100ms 간격
	}
	//ctrl->Trigger('1');
	//Sleep(500); // 100ms 간격
	ctrl->close();
	return 0;
}
*/