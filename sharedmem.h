#pragma once

#include <sys/mman.h>
#include <string>
struct InfImage {
    int width;
    int height;
    int pitch;
    unsigned char *data;
    InfImage() : width(0), height(0), pitch(0), data(nullptr) {}
};

struct ObjInfo {
    int x;
    int y;
    int w;
    int h;
    int cls;
    float prob;
};

enum cmdType {
    CMD_NULL = 0,
    CMD_DET,
    CMD_END,
};

struct SharedMemHeader {
    int cmd;
    int w;
    int h;
    int pitch;
    int objCount;
    // image data & obj data
};


class SharedMem {
public:
    SharedMem();
    ~SharedMem();
    bool Create();
    bool Open();
    void Close();

    SharedMemHeader *getHeader() {return static_cast<SharedMemHeader*>(mem);}
    unsigned char *getImageData() const;
    ObjInfo *getObjInfo() const;

    // cmd
    int getCmd() const;
    int setCmd(int cmd);

    bool setImage(const InfImage &img);
    bool setObjInfo(int objCount, ObjInfo *obj);

private:
    std::string shmName;
    size_t shmSize;
    void *mem;
    
    // memory layout
    const size_t imgSize = 800 * 800 * 3;
    const size_t maxObjCount = 100;
    const size_t headerSize = sizeof(SharedMemHeader);
    const size_t objOffset = headerSize + imgSize;
};