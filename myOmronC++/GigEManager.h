#pragma once

#include "GigEWorker.h"
#include <vector>
#include <sstream>

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
	/*  @brief Trigger selected GigE Workers based on indices.
	@param indices : A vector of indices representing the workers to trigger. */
    void TriggerSelected(const std::vector<int>& indices);
	/*  @brief Run an interactive loop for user input to trigger cameras. */
    void RunInteractiveLoop();

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