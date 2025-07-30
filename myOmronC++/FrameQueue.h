#pragma once

#include <queue>
#include <mutex>
#include <StApi_TL.h>
#include <condition_variable>

using namespace StApi;

/*	@brief Structure to hold frame data and metadata
	@brief FrameData structure contains an image pointer, serial number, frame ID, and timestamp. */
struct FrameData
{
	IStImage* pImage = nullptr;
	std::string serialNumber;
	uint64_t frameID = 0;
	std::string cameraName;
	bool isMono = NULL;
};

/*	@brief FrameQueue class
	@brief This class implements a thread-safe queue for storing image frames. */
class FrameQueue
{
public:
	FrameQueue();
	~FrameQueue();

	/*	@brief Producer(Camera thread) push an image frame into the queue
		@param frame : FrameData object containing image data and metadata */
	void Push(const FrameData& frame);
	/*	@brief Consumer(ImageSaveThreadPool) pop an image frame from the queue
		@param frame : FrameData object to take from the ImageSaveThreadPool */
	bool Pop(FrameData& frame);
	/*	@brief Consumer(ImageSaveThreadPool) pop an image frame from the queue with a timeout
		@param frame : FrameData object to take from the ImageSaveThreadPool
		@param timeout : Duration to wait for a frame to be available in the queue */
	bool PopWithTimeOut(FrameData& frame, std::chrono::milliseconds timeout);

	/* @brief Check if the queue is empty */
	bool isEmpty() const;
	/* @brief Get the size of the queue */
	size_t Size() const;
	/* @brief Clear the queue */
	void Clear();

protected:

private:
	/* @brief Queue to hold FrameData objects */
	std::queue<FrameData> m_queue;
	/* @brief Mutex for thread safety */
	mutable std::mutex m_mutex;
	/* @brief Condition variable for notifying consumers when a new frame is available */
	std::condition_variable m_cv;
};