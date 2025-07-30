#include "LoggerThread.h"

LoggerThread::LoggerThread()
	: m_running(false)
{
}

LoggerThread::~LoggerThread()
{
	Stop();
}

void LoggerThread::Start()
{
	if (m_running)
		return;

	m_running = true;
	m_thread = std::thread(&LoggerThread::Run, this);
}

void LoggerThread::Stop()
{
	if (!m_running)
		return;

	m_running = false;
	// Push an empty message to unblock the thread if it's waiting
	m_logQueue.Push("");

	if (m_thread.joinable())
	{
		m_thread.join();
	}
}

void LoggerThread::Log(const std::string& message)
{
	m_logQueue.Push(message);
}

void LoggerThread::Run()
{
	while (m_running)
	{
		std::string message;
		if (m_logQueue.PopWithTimeout(message, std::chrono::milliseconds(100)))
		{
			if (!message.empty())
			{
				std::cout << message << std::endl;
			}
		}
	}

	// Drain remaining messages in the queue
	std::string message;
	while (m_logQueue.PopWithTimeout(message, std::chrono::milliseconds(10)))
	{
		if (!message.empty())
		{
			std::cout << message << std::endl;
		}
	}
}