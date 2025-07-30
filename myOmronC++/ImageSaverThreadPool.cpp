#include "ImageSaverThreadPool.h"

ImageSaverThreadPool::ImageSaverThreadPool(size_t threadCount, const std::string& saveRootDir, std::shared_ptr<ThreadSafeQueue<FrameData>> pQueue, std::shared_ptr<ThreadSafeQueue<std::string>> pathQueue, std::shared_ptr<Logger> logger)
	: m_running(false)
	, m_strSaveRootDir(saveRootDir)
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
				GenICam::gcstring savePath = ImageProcess::SetSavePath(m_strSaveRootDir, frame.cameraName, frame.serialNumber, frame.frameID);
				//ImageProcess::SaveImage<BMP>(pBuffer, savePath);
				ImageProcess::SaveImage<JPEG>(pBuffer, savePath);

				m_logger->Log("[ImageSaverThreadPool] Saved: " + std::string(savePath) + JPEG::extension + "\t after Queue size:" + std::to_string(m_pFrameQueue->Size()));

				// Notify the path queue that a new path has been added
				//if (m_pathQueue)
				//{
				//	std::string fullMessage = frame.cameraName + " :" + savePath.c_str() + JPEG::extension;
				//	m_pathQueue->Push(fullMessage);
				//}
			}
			catch (const GenICam::GenericException& e)
			{
				m_logger->Log("[ImageSaverThreadPool] Worker error: " + std::string(e.GetDescription()));
			}
		}
	}
}