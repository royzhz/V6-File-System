//
// Created by roy on 26/03/2023.
//

#ifndef OSFILE_DISKMANAGER_H
#define OSFILE_DISKMANAGER_H

#include <cstdio>
#include <cstdlib>
#include <cstdint>

const char diskFileName[] = "myDisk.img";  //磁盘镜像文件名

class DiskDriver {
public:
    FILE* diskPointer;                     //磁盘文件指针

public:
    DiskDriver()
    {
        diskPointer = fopen(diskFileName, "rb+");
    }
    ~DiskDriver()
    {
        if (diskPointer != NULL) {
            fclose(diskPointer);
        }
    }
    void Reset()
    {
        if (diskPointer != NULL)
            fclose(diskPointer);
        diskPointer = fopen(diskFileName, "rb+");
    }
    //写磁盘函数
    void write(const uint8_t* in_buffer, uint32_t in_size, int offset = -1, uint32_t origin = SEEK_SET) {
        if (offset >= 0)
            fseek(diskPointer, offset, origin);
        fwrite(in_buffer, in_size, 1, diskPointer);
        return;
    }
    //读磁盘函数
    void read(uint8_t* out_buffer, uint32_t out_size, int offset = -1, uint32_t origin = SEEK_SET) {
        if (offset >= 0)
            fseek(diskPointer, offset, origin);
        fread(out_buffer, out_size, 1, diskPointer);
        return;
    }

    //检查镜像文件是否存在
    bool Exists()
    {
        return diskPointer != NULL;
    }
    //打开镜像文件
    void Construct()
    {
        diskPointer = fopen(diskFileName, "wb+");
        if (diskPointer == NULL)
            printf("Disk Construct Error!\n");
    }
};


#endif //OSFILE_DISKMANAGER_H
