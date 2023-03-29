//
// Created by roy on 26/03/2023.
//

#ifndef OSFILE_KERNEL_H
#define OSFILE_KERNEL_H
#include "BufferManager.h"
#include "DiskManager.h"
#include "FileSystem.h"
#include "User.h"
#include "FileManager.h"

class Kernel {
public:
    Kernel();
    ~Kernel();
    static Kernel& Instance();

    BufferManager& GetBufferManager();
    DiskDriver& GetDiskDriver();
    FileSystem& GetFileSystem();
    FileManager& GetFileManager();
    User& GetUser();
    SuperBlock& GetSuperBlock();
    void Reset();

private:
    static Kernel instance;
    DiskDriver diskDriver;
    SuperBlock superBlock;
    BufferManager bufferManager;
    FileSystem fileSystem;
    FileManager fileManager;
    User user;
};


#endif //OSFILE_KERNEL_H
