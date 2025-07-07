#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

#include <StApi_TL.h>

using namespace StApi;

class FrameQueue
{
public:
	void Push(IStImage* frame);
	IStImage* Pop();
	//bool TryPop(IStImage*& frame, int timeoutMs = 1000);
	bool TryPop(IStImage*& frame);
	bool IsEmpty();
	size_t Size();

protected:

private:
	std::queue<IStImage*> m_queue; // Queue to hold frames
	std::mutex m_mutex; // Mutex for thread safety
	std::condition_variable m_cv; // Condition variable for synchronization
};

