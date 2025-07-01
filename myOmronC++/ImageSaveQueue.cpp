#include "ImageSaveQueue.h"

ImageSaveQueue::ImageSaveQueue()
{
}

void ImageSaveQueue::Push(const FrameData& frame)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_queue.push(frame);
	m_cv.notify_one();  // Notify one waiting consumer that a new frame is available
}

bool ImageSaveQueue::Pop(FrameData& frame)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_cv.wait(lock, [this]() { return !m_queue.empty(); });

	frame = m_queue.front();
	m_queue.pop();
	return true;
}

bool ImageSaveQueue::PopWithTimeOut(FrameData& frame, std::chrono::milliseconds timeout)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_cv.wait_for(lock, timeout, [this]() { return !m_queue.empty(); }))
	{
		frame = m_queue.front();
		m_queue.pop();
		return true;
	}
	return false;
}

bool ImageSaveQueue::isEmpty() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_queue.empty();
}

size_t ImageSaveQueue::Size() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_queue.size();
}

void ImageSaveQueue::Clear()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	while (!m_queue.empty())
	{
		m_queue.pop();
	}
	m_cv.notify_all();  // Notify all waiting consumers that the queue has been cleared
}
