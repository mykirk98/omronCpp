#pragma once

#include <queue>
#include <mutex>
#include <StApi_TL.h>
#include <condition_variable>

using namespace StApi;

struct FrameData
{
	IStImage* pImage;
	std::string serialNumber;
	//std::string cameraName;
	uint64_t frameID;
	std::chrono::steady_clock::time_point timestamp;
};


class ImageSaveQueue
{
public:
	ImageSaveQueue();

	// Producer: Push an image frame into the queue
	void Push(const FrameData& frame);
	// Consumer: Pop an image frame from the queue
	bool Pop(FrameData& frame);
	// Consumer: Pop an image frame from the queue with a timeout
	bool PopWithTimeOut(FrameData& frame, std::chrono::milliseconds timeout);

	// Check if the queue is empty
	bool isEmpty() const;
	// Get the size of the queue
	size_t Size() const;
	// Clear the queue
	void Clear();


private:
	std::queue<FrameData> m_queue;
	mutable std::mutex m_mutex;
	std::condition_variable m_cv;
};

