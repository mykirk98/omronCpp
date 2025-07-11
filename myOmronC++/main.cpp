#include "GigEManager.h"
#include <iostream>

int main() {
    GigEManager manager;

    // Step 1: Initialize all cameras
    if (!manager.Initialize()) {
        std::cerr << "[main] Failed to initialize cameras." << std::endl;
        return -1;
    }

    // Step 2: Start acquisition on all cameras
    manager.StartAll();
    std::cout << "[main] Acquisition started on all cameras." << std::endl;

    // Step 3: Interactive trigger loop
    manager.RunInteractiveLoop();

    // Step 4: Stop acquisition on all cameras
    manager.StopAll();
    std::cout << "[main] Acquisition stopped on all cameras." << std::endl;

    return 0;
}
