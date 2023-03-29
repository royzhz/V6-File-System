//
// Created by roy on 26/03/2023.
//

#include "Kernel.h"

Kernel Kernel::instance;

Kernel& Kernel::Instance()
{
    return Kernel::instance;
}

BufferManager &Kernel::GetBufferManager() {
    return bufferManager;
}

DiskDriver &Kernel::GetDiskDriver() {
    return diskDriver;
}

FileSystem &Kernel::GetFileSystem() {
    return fileSystem;
}

FileManager &Kernel::GetFileManager() {
    return fileManager;
}

User &Kernel::GetUser() {
    return user;
}

SuperBlock &Kernel::GetSuperBlock() {
    return superBlock;
}

Kernel::Kernel() {

}

Kernel::~Kernel() {

}

void Kernel::Reset() {
    //依次重启各个模块
    FileSystem fs=instance.GetFileSystem();
    fs.FormatDevice();
    instance.~Kernel();
    new(&instance) Kernel();

}
