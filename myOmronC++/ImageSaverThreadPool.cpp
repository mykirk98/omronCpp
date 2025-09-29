#include "ImageSaverThreadPool.h"

ImageSaverThreadPool::ImageSaverThreadPool(size_t threadCount, const std::string& saveRootDir, std::shared_ptr<YCQueue<FrameData>> pQueue, std::shared_ptr<YCQueue<std::string>> pathQueue, std::shared_ptr<CamLogger> logger)
	: m_running(false)
	, m_strRootDir(saveRootDir)
	, m_pFrameQueue(pQueue)
	, m_pPathQueue(pathQueue)
{
	// Reserve space for the specified number of threads to avoid frequent reallocations
	m_workers.reserve(threadCount);
	m_logger = logger;
}

ImageSaverThreadPool::~ImageSaverThreadPool()
{
	// Ensure all threads are stopped before destruction
	Stop();
}

void ImageSaverThreadPool::Start()
{
	m_running = true;

	for (size_t i = 0; i < m_workers.capacity(); i++)
	{
		// Create a new thread and add it to the worker pool
		m_workers.emplace_back(&ImageSaverThreadPool::WorkerLoop, this);
		// this : pointer to the current instance of ImageSaverThreadPool
	}
	m_logger->Log("[ImageSaverThreadPool] Thread pool started with " + std::to_string(m_workers.size()) + " threads.");
}

void ImageSaverThreadPool::Stop()
{
	// Wait until the queue is empty before stopping the threads
	while (!m_pFrameQueue->IsEmpty())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	m_running = false;
	// clear the queue to stop processing frames, This will ensure that all threads exit gracefully
	m_pFrameQueue->Clear();

	// iterate through the worker threads and join them
	for (std::vector<std::thread>::iterator worker = m_workers.begin(); worker != m_workers.end(); ++worker)
	{
		// Check if the thread is joinable before joining
		if (worker->joinable())
		{
			worker->join();
		}
	}
	// clear the worker threads vector
	m_workers.clear();
	m_logger->Log("[ImageSaverThreadPool] Thread pool stopped.");
}

void ImageSaverThreadPool::WorkerLoop()
{
	while (m_running)
	{
		FrameData frame;
		if (m_pFrameQueue && m_pFrameQueue->PopWithTimeout(frame, std::chrono::milliseconds(200)))
		{
			try
			{
				CIStImageBufferPtr pBuffer(CreateIStImageBuffer());
				ImageProcess::ConvertPixelFormat(frame.pImage, frame.isMono, pBuffer);
				GenICam::gcstring savePath = ImageProcess::SetSavePath(m_strRootDir, frame.cameraName, frame.detailInfo, frame.frameID);
				ImageProcess::SaveImage<BMP>(pBuffer, savePath);

				m_logger->Log("[ImageSaverThreadPool] Saved: " + std::string(savePath) + "\t after Queue size:" + std::to_string(m_pFrameQueue->Size()) + "\ttime: " + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));

				// Notify the path queue that a new path has been added
				if (m_pPathQueue)
				{
					std::string fullMessage = frame.cameraName + " :" + std::string(savePath.c_str());
					m_pPathQueue->Push(fullMessage);
					m_logger->Log("[ImageSaverThreadPool] Send path to GUI by queue: " + fullMessage);
				}
			}
			catch (const GenICam::GenericException& e)
			{
				m_logger->Log("[ImageSaverThreadPool] Worker error: " + std::string(e.GetDescription()));
			}
		}
	}
}