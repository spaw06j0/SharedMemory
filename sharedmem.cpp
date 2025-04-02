#include "sharedmem.h"
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

SharedMem::SharedMem(const std::string &name) : shmName(name), shmSize(imgSize), mem(nullptr), shmid(-1) {}
SharedMem::SharedMem(const std::string &name, size_t size) : shmName(name), shmSize(size), mem(nullptr), shmid(-1) {}

SharedMem::~SharedMem() {
    if (mem) {
        Close();
    }
}

bool SharedMem::Create() {
    shm_unlink(shmName.c_str());
    shmid = shm_open(shmName.c_str(), O_CREAT | O_RDWR, 0666);
    if (shmid == -1) {
        std::cerr << "Error creating shared memory: " << strerror(errno) << std::endl;
        return false;
    }
    if (ftruncate(shmid, shmSize) == -1) {
        std::cerr << "Error setting size of shared memory: " << strerror(errno) << std::endl;
        close(shmid);
        shmid = -1;
        return false;
    }
    mem = mmap(nullptr, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
    if (mem == MAP_FAILED) {
        std::cerr << "Error mapping shared memory: " << strerror(errno) << std::endl;
        close(shmid);
        shmid = -1;
        return false;
    }

    isOwner = true;

    SharedMemHeader* header = getHeader();
    header->cmd = CMD_NULL;
    header->w = 0;
    header->h = 0;
    header->pitch = 0;
    header->objCount = 0;
    return true;
}

bool SharedMem::Open() {
    shmid = shm_open(shmName.c_str(), O_RDWR, 0666);
    if (shmid == -1) {
        std::cerr << "Error opening shared memory: " << strerror(errno) << std::endl;
        return false;
    }
    mem = mmap(nullptr, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
    if (mem == MAP_FAILED) {
        std::cerr << "Error mapping shared memory: " << strerror(errno) << std::endl;
        close(shmid);
        shmid = -1;
        return false;
    }
    return true;
}

void SharedMem::Close() {
    if (mem != nullptr && mem != MAP_FAILED) {
        munmap(mem, shmSize);
        mem = nullptr;
    }
    if (shmid != -1) {
        close(shmid);
        shmid = -1;
    }
}

bool SharedMem::Unlink() {
    if (isOwner) {
        return (shm_unlink(shmName.c_str()) == 0);
    }
    return false;
}

unsigned char *SharedMem::getImageData() const {
    if (!mem) {
        return nullptr;
    }
    return static_cast<unsigned char*>(mem) + headerSize;
}

ObjInfo *SharedMem::getObjInfo() const {
    if (!mem) {
        return nullptr;
    }
    return reinterpret_cast<ObjInfo*>(static_cast<char*>(mem) + objOffset);
}

InfImage SharedMem::GetImage() const {
    InfImage img;
    if (!mem) {
        return img;
    }
    SharedMemHeader* header = getHeader();
    
    img.width = header->w;
    img.height = header->h;
    img.pitch = header->pitch;
    img.data = getImageData();
    return img;
}

int SharedMem::getCmd() const {
    if (!mem) {
        return CMD_NULL;
    }
    SharedMemHeader* header = getHeader();
    return header->cmd;
}

int SharedMem::setCmd(int cmd) {
    if (!mem) {
        return CMD_NULL;
    }
    getHeader()->cmd = cmd;
    return cmd;
}

bool SharedMem::setImage(const InfImage &img) {
    if (!mem) return false;
    SharedMemHeader* header = getHeader();
    header->w = img.width;
    header->h = img.height;
    header->pitch = img.pitch;
    size_t dataSize = img.pitch * img.height;
    if (dataSize > imgSize) {
        std::cerr << "Image data size exceeds maximum allowed size" << std::endl;
        return false;
    }
    memcpy(getImageData(), img.data, dataSize);
    return true;
}

bool SharedMem::setObjInfo(int objCount, ObjInfo *obj) {
    if (!mem) return false;
    SharedMemHeader* header = getHeader();
    std::cout << "successful get header" << std::endl;
    if (objCount > (int)maxObjCount) {
        std::cerr << "Object count exceeds maximum allowed count" << std::endl;
        return false;
    }
    header->objCount = objCount;
    std::cout << "successful set objCount" << std::endl;
    ObjInfo *objInfo = getObjInfo();
    std::cout << "successful get objInfo" << std::endl;
    memcpy(objInfo, obj, objCount * sizeof(ObjInfo));
    return true;
}

void SharedMem::clearImage() {
    if (!mem) return;
    SharedMemHeader* header = getHeader();
    header->w = 0;
    header->h = 0;
    header->pitch = 0;
}

void SharedMem::clearObjInfo() {
    if (!mem) return;
    SharedMemHeader* header = getHeader();
    header->objCount = 0;
}
    
    