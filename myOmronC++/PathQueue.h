#pragma once

#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>

class PathQueue
{
public:
    PathQueue();
    ~PathQueue();

    // producer: 저장된 경로 push
    void Push(const std::string& path);

    // consumer: 경로 pop (blocking)
    bool Pop(std::string& path);

    // consumer: 경로 pop (with timeout)
    bool PopWithTimeout(std::string& path, std::chrono::milliseconds timeout);

    // 유틸리티
    bool IsEmpty() const;
    size_t Size() const;
    void Clear();

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<std::string> m_queue;
};
