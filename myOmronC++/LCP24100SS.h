#pragma once
#include "LightController.h"

class LCP24100SS : public LightController
{
public:
	bool SetBrightness(char channel, int value) override;

	bool SetStrobeTime_ms(char channel, double ms) override;

	bool Trigger(char channel) override;

protected:

private:
	std::string makeFrame(char ch, char model, int value3) const;
};

