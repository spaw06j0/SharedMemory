#include "../sharedmem.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>
#include <cstring>

volatile bool running = true;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received. Shutting down..." << std::endl;
    running = false;
}

void processCommands(SharedMem& shm) {
    bool runone = false;
    while (running) {
        // Check for detect command
        if (shm.getCmd() == CMD_DET) {
            std::cout << "Processing detect command..." << std::endl;
            
            // Get the image from shared memory
            InfImage img = shm.GetImage();
            
            // Process the image (perform detection)
            // This is where you'd call your actual detection algorithm
            std::cout << "Detecting objects in image (" << img.width << "x" << img.height << ")" << std::endl;
            
            // Example: Create some dummy detection results
            const int objCount = 3;
            ObjInfo objects[objCount];
            objects[0] = {50, 50, 100, 100, 1, 0.95f};
            objects[1] = {200, 150, 80, 120, 2, 0.85f};
            objects[2] = {350, 200, 120, 90, 1, 0.78f};
            std::cout << "Detected " << objCount << " objects" << std::endl;
            // Set the results in shared memory
            std::cout << "Setting object info..." << std::endl;
                
            // Set the results in shared memory
            bool result = shm.setObjInfo(objCount, objects);
            if (!result) {
                std::cerr << "Failed to set object info" << std::endl;
            } else {
                std::cout << "Object info set successfully" << std::endl;
            }
            
            // Signal that detection is complete
            // shm.setCmd(CMD_NULL);
            shm.setCmd(CMD_END);
            runone = true;
        }
        if (runone) {
            break;
        }
        
        // Sleep a bit to avoid using 100% CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    signal(SIGINT, signalHandler);
    const size_t MAX_IMAGE_SIZE = 800 * 800 * 3;
    const size_t MAX_OBJECTS = 100;
    const size_t HEADER_SIZE = sizeof(SharedMemHeader);
    const size_t TOTAL_SIZE = HEADER_SIZE + MAX_IMAGE_SIZE + (MAX_OBJECTS * sizeof(ObjInfo));
    
    // Use the same name as the client
    SharedMem shm("/ai_smart_program", TOTAL_SIZE);
    if (!shm.Create()) {
        std::cerr << "Failed to create shared memory. Exiting." << std::endl;
        return 1;
    }

    std::cout << "Shared memory created. Waiting for commands..." << std::endl;
    processCommands(shm);
    std::cout << "Processing commands complete. Closing shared memory..." << std::endl;
    shm.Close();
    shm.Unlink();
    std::cout << "Shared memory closed and unlinked. Exiting..." << std::endl;
    return 0;
}