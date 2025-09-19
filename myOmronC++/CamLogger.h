#pragma once

#include "YCQueue.h"
#include <string>
#include <thread>
#include <atomic>
#include <memory>

class CamLogger
{
public:
	CamLogger();
	~CamLogger();

	void Start();
	void Stop();

	void Log(const std::string& message);

protected:

private:
	void Run();

	std::atomic<bool> m_running;
	std::thread m_thread;
	YCQueue<std::string> m_logQueue;
};