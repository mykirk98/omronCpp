#include "FrameQueue.h"

void FrameQueue::Push(IStImage* frame)
{
	{
		// Lock the mutex to ensure thread safety when accessing the queue
		std::lock_guard<std::mutex> lock(m_mutex);
		// Push the frame into the queue
		m_queue.push(frame);
		std::cout << "[FrameQueue] Frame pushed, queue size: " << m_queue.size() << std::endl;
	}
	// Notify one waiting thread that a new frame is available
	m_cv.notify_one();
}

IStImage* FrameQueue::Pop()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_cv.wait(lock, [this]() { return !m_queue.empty(); });

	IStImage* frame = m_queue.front();
	m_queue.pop();
	return frame;
}

bool FrameQueue::TryPop(IStImage*& frame)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_queue.empty())
	{
		return false;
	}

	frame = m_queue.front();
	m_queue.pop();
	return true;
}

bool FrameQueue::IsEmpty()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_queue.empty();
}

size_t FrameQueue::Size()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_queue.size();
}

// Example usage of FrameQueue class
/*
#include "TriggerCamera.h"
#include <chrono>

int main()
{
	std::cout << "========== Trigger Camera with Wait Example ==========" << std::endl;

	CStApiAutoInit stApiAutoInit;
	CIStSystemPtr pSystem(CreateIStSystem());

	std::shared_ptr<FrameQueue> sharedFrameQueue = std::make_shared<FrameQueue>();

	TriggerCamera camera;
	camera.SetFrameQueue(sharedFrameQueue);
	if (camera.Initialize(pSystem))
	{
		camera.StartAcquisition();

		// calculate average FPS
		std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

		for (int i = 0; i < 10; ++i)
		{
			std::cout << "[Main] Triggering " << i << std::endl;
			if (camera.TriggerAndWait(100))
				std::cout << "[Main] Frame " << i << " captured." << std::endl;
			else
				std::cerr << "[Main] Frame " << i << " timed out." << std::endl;
		}

		for (int i = 0; i < 10; ++i)
		{
			IStImage* image = sharedFrameQueue->Pop();
			std::cout << "[Main] Popped frame, queue size: " << sharedFrameQueue->Size() << std::endl;
		}

		std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
		double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
		double averageFPS = 10.0 / (elapsedTime / 1000.0); // 1000 frames
		std::cout << "[Main] Average FPS: " << averageFPS << std::endl;

		camera.StopAcquisition();
	}

	return 0;
}
*/