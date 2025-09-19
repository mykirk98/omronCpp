#pragma once

#include "YCQueue.h"
#include "ImageProcess.h"
#include <thread>
#include <vector>
#include <atomic>
#include <string>
#include "CamLogger.h"

/* @brief This class manages a pool of threads for saving images. */
class ImageSaverThreadPool
{
public:
	/*	@brief ImageSaverThreadPool constructor
		@param threadCount : Number of threads in the pool
		@brief rootDir : Root directory where images will be saved */
	ImageSaverThreadPool(size_t threadCount, const std::string& rootDir, std::shared_ptr<YCQueue<FrameData>> pQueue, std::shared_ptr<YCQueue<std::string>> pathQueue, std::shared_ptr<CamLogger> logger);
	/* @brief ImageSaverThreadPool destructor */
	~ImageSaverThreadPool();

	/* @brief Start the thread pool */
	void Start();
	/* @brief Stop the thread pool */
	void Stop();

protected:

private:
	/* @brief WorkerLoop function */
	void WorkerLoop();

	/* @brief Thread pool for saving images */
	std::vector<std::thread> m_workers;
	/* @brief FrameQueue object for managing image frames */
	std::shared_ptr<YCQueue<FrameData>> m_pFrameQueue;
	/* @brief Flag to indicate whether the thread pool is running */
	std::atomic<bool> m_running;
	/* @brief Root directory where images will be saved */
	std::string m_strRootDir;

	std::shared_ptr<YCQueue<std::string>> m_pPathQueue;
	std::shared_ptr<CamLogger> m_logger;
};