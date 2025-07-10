#include "ImageSaverThreadPool.h"

ImageSaverThreadPool::ImageSaverThreadPool(size_t threadCount, const std::string& saveRootDir, bool convertToColor)
	: m_running(false)
	, m_saveRootDir(saveRootDir)
	, m_convertToColor(convertToColor)
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
	m_running = false;
	// clear the queue to stop processing frames
	// This will ensure that all threads exit gracefully
	m_queue.Clear();

	// iterate through the worker threads and join them
	for (std::vector<std::thread>::iterator it = m_workers.begin(); it != m_workers.end(); ++it)
	{
		// Check if the thread is joinable before joining
		if (it->joinable())
		{
			it->join();
		}
	}
	// clear the worker threads vector
	m_workers.clear();
}

void ImageSaverThreadPool::Enqueue(const FrameData& frame)
{
	// Producer: push a frame into the queue
	m_queue.Push(frame);
}

void ImageSaverThreadPool::WorkerLoop()
{
	while (m_running)
	{
		FrameData frame;
		if (m_queue.PopWithTimeOut(frame, std::chrono::milliseconds(200)))
		{
			try
			{
				CIStImageBufferPtr pImageBuffer(CreateIStImageBuffer());
				ConvertPixelFormat(frame.pImage, m_convertToColor, pImageBuffer);

				GenICam::gcstring savePath = SetSavePath(m_saveRootDir, frame.serialNumber, frame.frameID);
				SaveImage<BMP>(pImageBuffer, savePath);
				std::cout << "[ImageSaverThreadPool] Saved image with ID: " << frame.frameID 
					<< " to path: " << savePath.c_str() << std::endl;
			}
			catch (const GenICam::GenericException& e)
			{
				std::cerr << "[ImageSaverThreadPool] Error processing frame with ID: " << frame.frameID << e.GetDescription() << std::endl;
			}
		}
	}
}

void ImageSaverThreadPool::ConvertPixelFormat(IStImage* pSrcImage, bool isColor, CIStImageBufferPtr& pDstBuffer)
{
	try
	{
		// Create a data converter object for pixel format conversion.
		CIStPixelFormatConverterPtr pPixelFormatConverter(CreateIStConverter(StConverterType_PixelFormat));
		
		if (isColor)
		{
			pPixelFormatConverter->SetDestinationPixelFormat(StPFNC_BGR8);
		}
		else
		{
			pPixelFormatConverter->SetDestinationPixelFormat(StPFNC_Mono8);
		}
		// Convert the pixel format of the source image to the destination buffer.
		pPixelFormatConverter->Convert(pSrcImage, pDstBuffer);
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[ImageSaverThreadPool] Converting pixel format error: " << e.GetDescription() << std::endl;
	}
}

GenICam::gcstring ImageSaverThreadPool::SetSavePath(const std::string& baseDir, const std::string& cameraName, const uint64_t frameID)
{
	try
	{
		// Change frameID to string
		std::string strFrameID = std::to_string(frameID);
		
		std::string filePath = baseDir + "\\" + cameraName + "\\" + strFrameID;

		return GenICam::gcstring(filePath.c_str());
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[ImageSaverThreadPool] Setting save path error: " << e.GetDescription() << std::endl;
	}
	return GenICam::gcstring();
}