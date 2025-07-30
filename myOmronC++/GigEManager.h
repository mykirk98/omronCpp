#pragma once

#include <thread>
#include <vector>
#include <map>
#include <memory>
#include "GigECamera.h"
#include "ThreadSafeQueue.h"
#include "ImageSaverThreadPool.h"

/*  @brief GigEManager class for managing multiple GigE Workers. */
class GigEManager
{
public:
	/*  @brief Constructor for GigEManager. */
	explicit GigEManager(std::string saveRootDir);
	explicit GigEManager(std::string saveRootDir, std::shared_ptr<ThreadSafeQueue<std::string>> pathQueue);
	/*  @brief Destructor for GigEManager. */
	~GigEManager();

	/*  @brief Initialize the GigEManager and its workers. */
	bool Initialize();
	/*  @brief Start all GigE Workers. */
	void StartAll();
	/*  @brief Stop all GigE Workers. */
	void StopAll();
	/*  @brief Trigger all GigE Workers to capture an image. */
	//void TriggerAll();		//TODO: Later, PTP support for parallel triggering
	/*	@brief Trigger single GigE Worker by index.
	@param index The index of the GigE Worker to trigger. */
	void TriggerSingle(int index);
	/*	@brief Trigger single GigE Worker by camera name.
	@param cameraName The name of the camera to trigger. */
	void TriggerSingle(const std::string& cameraName);

protected:

private:
	void CameraLoop(std::shared_ptr<GigECamera> camera);
	/*	@brief Initialize StApi */
	CStApiAutoInit m_stApiAutoInit;
	/*  @brief Print the status of all GigE Workers. */
    CIStSystemPtr m_pSystem;
	/*	@brief vector of GigECamera objects representing the GigE cameras. */
	std::vector<std::shared_ptr<GigECamera>> m_cameras;
	/*	@brief Map of GigECamera objects indexed by camera name. */
	std::map<std::string, std::shared_ptr<GigECamera>> m_cameraMap;
	/*	@brief Vector of threads for running GigE camera loops.*/
	std::vector<std::thread> m_threads;

	/*	@brief Flag to indicate whether the GigEManager is running. */
	std::atomic<bool> m_running;
	/*	@brief Base path to save images. */
	std::string m_strSaveRootDir;

	/*	@brief Frame queue for managing image frames. */
	std::shared_ptr<ThreadSafeQueue<FrameData>> m_pFrameQueue;
	/*	@brief Thread pool for saving images. */
	std::shared_ptr<ImageSaverThreadPool> m_pImageSaverThreadPool;
	/*	@brief Path queue for managing paths for communicate with other processes. */
	std::shared_ptr<ThreadSafeQueue<std::string>> m_pPathQueue;
};