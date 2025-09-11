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

std::string LCP24100SS::makeFrame(char ch, char model, int value3) const
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
	std::snprintf(data, sizeof(data), "%03d", value3);

	std::string s;
	s.push_back('\x02'); // STX
	s.push_back(ch);     // 채널
	s.push_back(model);  // 'P' 휘도, 'T' 스트로브 시간, 'F' 트리거
	s.append(data, 3);   // 3자리 ASCII
	s.push_back('R');    // 보통 'R' (Remote 동작)
	s.push_back('\x03'); // ETX

	return s;
}