#pragma once

#include "FrameQueue.h"
#include "ImageProcess.h"
#include <thread>
#include <vector>
#include <atomic>
#include <string>
#include <StApi_IP.h>

using namespace StApi;

/* @brief This class manages a pool of threads for saving images. */
class ImageSaverThreadPool
{
public:
	/*	@brief ImageSaverThreadPool constructor
		@param threadCount : Number of threads in the pool
		@brief saveRootDir : Root directory where images will be saved
		@brief convertToColor : Flag to indicate whether to convert images to color format */
	ImageSaverThreadPool(size_t threadCount, const std::string& saveRootDir, std::shared_ptr<FrameQueue> pQueue, bool convertToColor = false);
	/* @brief ImageSaverThreadPool destructor */
	~ImageSaverThreadPool();

	/* @brief Start the thread pool */
	void Start();
	/* @brief Stop the thread pool */
	void Stop();

private:
	/* @brief WorkerLoop function */
	void WorkerLoop();

	/* @brief Thread pool for saving images */
	std::vector<std::thread> m_workers;
	/* @brief FrameQueue object for managing image frames */
	std::shared_ptr<FrameQueue> m_queue;
	/* @brief Flag to indicate whether the thread pool is running */
	std::atomic<bool> m_running;
	/* @brief Root directory where images will be saved */
	std::string m_saveRootDir;
	/* @brief Flag to indicate whether to convert images to color format */
	bool m_convertToColor;
};