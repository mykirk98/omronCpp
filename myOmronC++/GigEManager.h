#pragma once

#include "GigEWorker.h"
#include "ImageSaverThreadPool.h"
#include "PathQueue.h"
#include <vector>
#include <map>

/*  @brief GigEManager class for managing multiple GigE Workers. */
class GigEManager
{
public:
    /*  @brief Constructor for GigEManager. */
	explicit GigEManager(std::string saveRootDir);
    explicit GigEManager(std::string saveRootDir, std::shared_ptr<PathQueue> pathQueue);
	/*  @brief Destructor for GigEManager. */
    ~GigEManager();

	/*  @brief Initialize the GigEManager and its workers. */
    bool Initialize();
	/*  @brief Start all GigE Workers. */
    void StartAll();
	/*  @brief Stop all GigE Workers. */
    void StopAll();
	/*  @brief Trigger all GigE Workers to capture an image. */
    void TriggerAll();
	/*	@brief Trigger single GigE Worker by index.
	@param index The index of the GigE Worker to trigger. */
	void TriggerSingle(int index);
	/*	@brief Trigger single GigE Worker by camera name.
	@param cameraName The name of the camera to trigger. */
	void TriggerSingle(const std::string& cameraName);

protected:

private:
	/*	@brief Initialize StApi */
	CStApiAutoInit m_stApiAutoInit;
	/*  @brief Print the status of all GigE Workers. */
    CIStSystemPtr m_pSystem;
	/*  @brief List of GigE Workers managing individual cameras. */
    std::vector<std::shared_ptr<GigEWorker>> m_workers;
	std::map<std::string, std::shared_ptr<GigEWorker>> m_workerMap;

	std::string m_saveRootDir;

	std::shared_ptr<FrameQueue> m_frameQueue;
	std::shared_ptr<ImageSaverThreadPool> m_ImageSaverThreadPool;
	std::shared_ptr<PathQueue> m_pathQueue;
};