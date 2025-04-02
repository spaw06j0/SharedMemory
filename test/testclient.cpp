#include "../sharedmem.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <opencv2/opencv.hpp>

bool waitForResponse(SharedMem& shm, int expectedCmd, int timeoutMs = 5000) {
    auto start = std::chrono::steady_clock::now();
    
    while (true) {
        // Check if the command has completed
        if (shm.getCmd() == expectedCmd) {
            return true;
        }
        
        // Check for timeout
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        if (elapsed.count() > timeoutMs) {
            std::cerr << "Timeout waiting for response" << std::endl;
            return false;
        }
        
        // Sleep a bit to avoid using 100% CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool detectObjects(SharedMem& shm, const std::string& imagePath) {
    // Load the image using OpenCV
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Failed to load image: " << imagePath << std::endl;
        return false;
    }
    
    // Convert the image to the format expected by the shared memory
    InfImage img;
    img.width = image.cols;
    img.height = image.rows;
    img.pitch = image.step;
    img.data = image.data;
    
    // Clear any previous commands
    shm.setCmd(CMD_NULL);
    if (!shm.setImage(img)) {
        std::cerr << "Failed to set image in shared memory" << std::endl;
        return false;
    }
    shm.setCmd(CMD_DET);
    if (!waitForResponse(shm, CMD_END)) {
        return false;
    }

    // Get the detection results
    SharedMemHeader* header = shm.getHeader();
    ObjInfo* objects = shm.getObjInfo();
    std::cout << "Detected " << header->objCount << " objects:" << std::endl;
    for (int i = 0; i < header->objCount; i++) {
        std::cout << "Object " << i + 1 << ": "
                  << "Class " << objects[i].cls
                  << " at (" << objects[i].x << ", " << objects[i].y << ") "
                  << "size " << objects[i].w << "x" << objects[i].h
                  << " with confidence " << objects[i].prob << std::endl;
    }
    shm.setCmd(CMD_NULL);
    return true;
}

int main() {
    const size_t MAX_IMAGE_SIZE = 800 * 800 * 3;
    const size_t MAX_OBJECTS = 100;
    const size_t HEADER_SIZE = sizeof(SharedMemHeader);
    const size_t TOTAL_SIZE = HEADER_SIZE + MAX_IMAGE_SIZE + (MAX_OBJECTS * sizeof(ObjInfo));

    SharedMem shm("/ai_smart_program", TOTAL_SIZE);
    if (!shm.Open()) {
        std::cerr << "Failed to open shared memory. Is the server running?" << std::endl;
        return 1;
    }
    bool success = false;
    success = detectObjects(shm, "test/test.bmp");
    return success ? 0 : 1;
}
