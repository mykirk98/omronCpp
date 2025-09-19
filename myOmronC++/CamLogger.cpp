#include "CamLogger.h"

CamLogger::CamLogger()
	: m_running(false)
{
}

CamLogger::~CamLogger()
{
	Stop();
}

void CamLogger::Start()
{
	if (m_running)
		return;

	m_running = true;
	m_thread = std::thread(&CamLogger::Run, this);
}

void CamLogger::Stop()
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

void CamLogger::Log(const std::string& message)
{
	m_logQueue.Push(message);
}

void CamLogger::Run()
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