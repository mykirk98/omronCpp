#include "FrameQueue.h"

FrameQueue::FrameQueue()
{
}

FrameQueue::~FrameQueue()
{
}

void FrameQueue::Push(const FrameData& frame)
{
	// producer : push a frame into the queue
	
	// ensure thread safety when pushing a frame into the queue
	std::lock_guard<std::mutex> lock(m_mutex);
	// copy of the frame is pushed into the queue
	m_queue.push(frame);
	// notify one waiting consumer(ImageSaveThread) that a new frame is available
	m_cv.notify_one();
}

bool FrameQueue::Pop(FrameData& frame)
{
	// consumer : pop a frame from the queue

	// acquire the mutex to ensure thread-safety when access to the queue
	std::unique_lock<std::mutex> lock(m_mutex);
	// wait until there is a frame available in the queue by using condition variable
	// notify_one() in Push() will wake up one waiting consumer
	m_cv.wait(lock, [this]() { return !m_queue.empty(); });
	// retrieve the front frame from the queue
	frame = m_queue.front();
	// remove the front frame from the queue
	m_queue.pop();

	// return true to indicate that a frame was successfully retrieved
	return true;
}

bool FrameQueue::PopWithTimeOut(FrameData& frame, std::chrono::milliseconds timeout)
{
	// consumer : pop a frame from the queue with a timeout

	// acquire the mutex to ensure thread-safety when access to the queue
	std::unique_lock<std::mutex> lock(m_mutex);
	// wait for a frame to be available in the queue with a timeout
	if (m_cv.wait_for(lock, timeout, [this]() { return !m_queue.empty(); }))
	{
		// retrieve the front frame from the queue
		frame = m_queue.front();
		// remove the front frame from the queue
		m_queue.pop();
		
		// return true to indicate that a frame was successfully retrieved
		return true;
	}
	// if the timeout expires and no frame is available, return false
	return false;
}

bool FrameQueue::isEmpty() const
{
	// ensure thread safety when checking if the queue is empty
	std::lock_guard<std::mutex> lock(m_mutex);

	// return true if the queue is empty, false otherwise
	return m_queue.empty();
}

size_t FrameQueue::Size() const
{
	// ensure thread safety when getting the size of the queue
	std::lock_guard<std::mutex> lock(m_mutex);

	// return the size of the queue
	return m_queue.size();
}

void FrameQueue::Clear()
{
	// ensure thread safety when clearing the queue
	std::lock_guard<std::mutex> lock(m_mutex);
	// clear the queue by popping all elements
	while (!m_queue.empty())
	{
		m_queue.pop();
	}
	// notify all waiting consumers that the queue has been cleared
	m_cv.notify_all();
}