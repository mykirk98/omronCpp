#include "Logger.h"

Logger::Logger()
	: m_running(false)
{
}

Logger::~Logger()
{
	Stop();
}

void Logger::Start()
{
	if (m_running)
		return;

	m_running = true;
	m_thread = std::thread(&Logger::Run, this);
}

void Logger::Stop()
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

void Logger::Log(const std::string& message)
{
	m_logQueue.Push(message);
}

void Logger::Run()
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