#include "PathQueue.h"

PathQueue::PathQueue() {}

PathQueue::~PathQueue() {}

void PathQueue::Push(const std::string& path)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(path);
    m_cv.notify_one();  // 하나의 대기 중인 소비자 깨움
}

bool PathQueue::Pop(std::string& path)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this]() { return !m_queue.empty(); });
    path = m_queue.front();
    m_queue.pop();
    return true;
}

bool PathQueue::PopWithTimeout(std::string& path, std::chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_cv.wait_for(lock, timeout, [this]() { return !m_queue.empty(); }))
    {
        path = m_queue.front();
        m_queue.pop();
        return true;
    }
    return false;
}

bool PathQueue::IsEmpty() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

size_t PathQueue::Size() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

void PathQueue::Clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    while (!m_queue.empty())
        m_queue.pop();
    m_cv.notify_all();
}
