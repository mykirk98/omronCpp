#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <StApi_TL.h>

using namespace StApi;

struct FrameData
{
	IStImage* pImage = nullptr;
	//std::string serialNumber;
	std::string detailInfo;
	uint64_t frameID = 0;
	std::string cameraName;
	bool isMono = false;
};

template <typename T>
class YCQueue
{
public:
	YCQueue()
	{
	}

	~YCQueue()
	{
	}

	void Push(const T& item)
	{
		// ensure thread safety when pushing an item into the queue
		std::lock_guard<std::mutex> lock(m_mutex);
		// copy of the item is pushed into the queue
		m_queue.push(item);
		// notify one waiting consumer that a new item is available
		m_cv.notify_one();
	}

	bool PopWithTimeout(T& item, std::chrono::milliseconds timeout)
	{
		// acquire the mutex to ensure thread-safety when accessing the queue
		std::unique_lock<std::mutex> lock(m_mutex);
		// wait for an item to be available in the queue with a timeout
		if (m_cv.wait_for(lock, timeout, [this]() {return !m_queue.empty(); }))
		{
			// retrieve the front item from the queue
			item = m_queue.front();
			// remove the front item from the queue
			m_queue.pop();
			return true;
		}
		return false;
	}

	bool IsEmpty() const
	{
		// ensure thread safety when checking if the queue is empty
		std::lock_guard<std::mutex> lock(m_mutex);

		return m_queue.empty();
	}

	size_t Size() const
	{
		// ensure thread safety when getting the size of the queue
		std::lock_guard<std::mutex> lock(m_mutex);

		return m_queue.size();
	}

	void Clear()
	{
		// ensure thread safety when clearing the queue
		std::lock_guard<std::mutex> lock(m_mutex);
		// clear the queue by popping all items
		while (!m_queue.empty())
		{
			m_queue.pop();
		}
		// notify all waiting consumers that the queue has been cleared
		m_cv.notify_all();
	}

protected:

private:
	/* @brief Queue to hold items of type T */
	std::queue<T> m_queue;
	/* @brief Mutex for thread safety */
	mutable std::mutex m_mutex;
	/* @brief condition variable for notifying consumers when a new item is available */
	std::condition_variable m_cv;
};