#pragma once

#include "GigECamera.h"
#include <thread>
#include <atomic>
#include <memory>

/* @brief thread worker class for handling GigE camera operations. */
class GigEWorker
{
public:
    /*  @brief Constructor for GigEWorker.
	    @param camera A unique pointer to a GigECamera object. */
    explicit GigEWorker(std::unique_ptr<GigECamera> camera);
    /*  @brief Destructor for GigEWorker. */
    ~GigEWorker();

	/*  @brief Start the worker thread. */
    void Start();
	/*  @brief Stop the worker thread. */
    void Stop();
    /*  @brief Trigger the camera to capture an image. */
    void Trigger();
	/*  @brief Check if the worker thread is currently running.*/
    bool IsRunning() const;

protected:

private:
    /*  @brief Worker loop function that runs in a separate thread. */
    void WorkerLoop();
    /*  @brief Callback function for handling camera events. */
    std::unique_ptr<GigECamera> m_camera;
	/*  @brief Thread object for running the worker loop. */
    std::thread m_thread;
	/*  @brief Atomic boolean to indicate if the worker is running. */
    std::atomic<bool> m_running;
};