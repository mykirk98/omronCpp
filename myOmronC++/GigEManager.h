#pragma once

#include "GigEWorker.h"
#include <vector>

/*  @brief GigEManager class for managing multiple GigE Workers. */
class GigEManager
{
public:
    /*  @brief Constructor for GigEManager. */
    explicit GigEManager(std::string saveRootDir);
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

protected:

private:
	/*	@brief Initialize StApi */
	CStApiAutoInit m_stApiAutoInit;
	/*  @brief Print the status of all GigE Workers. */
    CIStSystemPtr m_pSystem;
	/*  @brief List of GigE Workers managing individual cameras. */
    std::vector<std::shared_ptr<GigEWorker>> m_workers;

	std::string m_saveRootDir;
};