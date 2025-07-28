#include "ImageSaverThreadPool.h"

ImageSaverThreadPool::ImageSaverThreadPool(size_t threadCount, const std::string& saveRootDir, std::shared_ptr<FrameQueue> pQueue, bool convertToColor)
	: m_running(false)
	, m_saveRootDir(saveRootDir)
	, m_convertToColor(convertToColor)
	, m_queue(pQueue)
{
	// Reserve space for the specified number of threads to avoid frequent reallocations
	m_workers.reserve(threadCount);
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
		// & : pointer to the WorkerLoop member function
		// this : pointer to the current instance of ImageSaverThreadPool
	}
}

void ImageSaverThreadPool::Stop()
{
	// Wait until the queue is empty before stopping the threads
	while (!m_queue->isEmpty())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	m_running = false;
	// clear the queue to stop processing frames
	// This will ensure that all threads exit gracefully
	m_queue->Clear();

	// iterate through the worker threads and join them
    for (auto& worker : m_workers)
    {
		// Check if the thread is joinable before joining
		if (worker.joinable())
		{
			worker.join();
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
		if (m_queue && m_queue->PopWithTimeOut(frame, std::chrono::milliseconds(200)))
		{
			try
			{
				CIStImageBufferPtr pBuffer(CreateIStImageBuffer());
				//ConvertPixelFormat(frame.pImage, m_convertToColor, pBuffer);
				ImageProcess::ConvertPixelFormat(frame.pImage, m_convertToColor, pBuffer);
				//GenICam::gcstring savePath = SetSavePath(m_saveRootDir, frame.cameraName, frame.serialNumber, frame.frameID);
				GenICam::gcstring savePath = ImageProcess::SetSavePath(m_saveRootDir, frame.cameraName, frame.serialNumber, frame.frameID);
				//SaveImage<BMP>(pBuffer, savePath);
				ImageProcess::SaveImage<BMP>(pBuffer, savePath);

				std::cout << "[ImageSaverThreadPool] Queue size: " << m_queue->Size() << std::endl;
				std::cout << "[ImageSaverThreadPool] Saved: " << savePath << std::endl;
			}
			catch (const GenICam::GenericException& e)
			{
				std::cerr << "[ImageSaverThreadPool] Worker error: " << e.GetDescription() << std::endl;
			}
		}
	}
}