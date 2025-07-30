#pragma once

#include "ThreadSafeQueue.h"
#include <string>
#include <thread>
#include <atomic>
#include <memory>

class LoggerThread
{
public:
	LoggerThread();
	~LoggerThread();

	void Start();
	void Stop();

	void Log(const std::string& message);

protected:

private:
	void Run();

	std::atomic<bool> m_running;
	std::thread m_thread;
	ThreadSafeQueue<std::string> m_logQueue;
};