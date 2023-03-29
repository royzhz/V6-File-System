//
// Created by roy on 26/03/2023.
//

#include "FileSystem.h"
#include "User.h"
#include "Kernel.h"
#include "DiskManager.h"
#include "OpenFileManager.h"

#include <ctime>

INode* FileSystem::IAlloc()
{
    SuperBlock* sb;
    Buf* pBuf;
    INode* pNode;
    User& u = Kernel::Instance().GetUser();
    int ino;	/* 分配到的空闲外存Inode编号 */

    /* 获取相应设备的SuperBlock内存副本 */
    sb = &Kernel::Instance().GetSuperBlock();

    /*
     * SuperBlock直接管理的空闲Inode索引表已空，
     * 必须到磁盘上搜索空闲Inode。先对inode列表上锁，
     * 因为在以下程序中会进行读盘操作可能会导致进程切换，
     * 其他进程有可能访问该索引表，将会导致不一致性。
     */
    if(sb->s_ninode <= 0)
    {
        /* 空闲Inode索引表上锁 */
        //sb->s_ilock++;

        /* 外存Inode编号从0开始，这不同于Unix V6中外存Inode从1开始编号 */
        ino = -1;

        /* 依次读入磁盘Inode区中的磁盘块，搜索其中空闲外存Inode，记入空闲Inode索引表 */
        for(int i = 0; i < sb->s_isize; i++)
        {
            pBuf = this->m_BufferManager->Bread(FileSystem::INODE_START_SECTOR + i);

            /* 获取缓冲区首址 */
            int* p = (int *)pBuf->b_addr;

            /* 检查该缓冲区中每个外存Inode的i_mode != 0，表示已经被占用 */
            for(int j = 0; j < FileSystem::INODE_NUMBER_PER_SECTOR; j++)
            {
                ino++;

                int mode = *( p + j * sizeof(DiskINode)/sizeof(int) );

                /* 该外存Inode已被占用，不能记入空闲Inode索引表 */
                if(mode != 0)
                {
                    continue;
                }

                /*
                 * 如果外存inode的i_mode==0，此时并不能确定
                 * 该inode是空闲的，因为有可能是内存inode没有写到
                 * 磁盘上,所以要继续搜索内存inode中是否有相应的项
                 */
                if( g_InodeTable.IsLoaded(ino) == -1 )
                {
                    /* 该外存Inode没有对应的内存拷贝，将其记入空闲Inode索引表 */
                    sb->s_inode[sb->s_ninode++] = ino;

                    /* 如果空闲索引表已经装满，则不继续搜索 */
                    if(sb->s_ninode >= 100)
                    {
                        break;
                    }
                }
            }

            /* 至此已读完当前磁盘块，释放相应的缓存 */
            this->m_BufferManager->Brelse(pBuf);

            /* 如果空闲索引表已经装满，则不继续搜索 */
            if(sb->s_ninode >= 100)
            {
                break;
            }
        }
//        /* 解除对空闲外存Inode索引表的锁，唤醒因为等待锁而睡眠的进程 */
//        sb->s_ilock = 0;
//        Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&sb->s_ilock);

        /* 如果在磁盘上没有搜索到任何可用外存Inode，返回NULL */
        if(sb->s_ninode <= 0)
        {
            //Diagnose::Write("No Space On %d !\n", dev);
            u.u_error = User::U_ENOSPC;
            return NULL;
        }
    }

    /*
     * 上面部分已经保证，除非系统中没有可用外存Inode，
     * 否则空闲Inode索引表中必定会记录可用外存Inode的编号。
     */
    while(true)
    {
        /* 从索引表“栈顶”获取空闲外存Inode编号 */
        ino = sb->s_inode[--sb->s_ninode];

        /* 将空闲Inode读入内存 */
        pNode = g_InodeTable.IGet(ino);
        /* 未能分配到内存inode */
        if(NULL == pNode)
        {
            return NULL;
        }

        /* 如果该Inode空闲,清空Inode中的数据 */
        if(0 == pNode->i_mode)
        {
            pNode->Clean();
            /* 设置SuperBlock被修改标志 */
            sb->s_fmod = 1;
            return pNode;
        }
        else	/* 如果该Inode已被占用 */
        {
            g_InodeTable.IPut(pNode);
            continue;	/* while循环 */
        }
    }
    return NULL;	/* GCC likes it! */
}

Buf *FileSystem::Alloc() {
    int blkno;
    SuperBlock *sb;
    sb = &Kernel::Instance().GetSuperBlock();
    Buf* pCache;
    User& u = Kernel::Instance().GetUser();
    //从索引表“栈顶”获取空闲磁盘块编号
    blkno = sb->s_free[--sb->s_nfree];

    //若获取磁盘块编号为零，则表示已分配尽所有的空闲磁盘块
    if (blkno <= 0) {
        sb->s_nfree = 0;
        u.u_error = User::U_ENOSPC;
        return NULL;
    }

    //栈已空，新分配到空闲磁盘块中记录了下一组空闲磁盘块的编号
    //将下一组空闲磁盘块的编号读入sb的空闲磁盘块索引表s_free[100]中
    if (sb->s_nfree <= 0) {
        pCache = this->m_BufferManager->Bread(blkno);
        int* p = (int*)pCache->b_addr;
        sb->s_nfree = *p++;
        memcpy(sb->s_free, p, sizeof(sb->s_free));
        this->m_BufferManager->Brelse(pCache);
    }
    pCache = this->m_BufferManager->GetBlk(blkno);
    if (pCache)
        this->m_BufferManager->Bclear(pCache);
    sb->s_fmod = 1;
    return pCache;
}

void FileSystem::IFree(int number) {
    SuperBlock *sb;
    sb = &Kernel::Instance().GetSuperBlock();
    if (sb->s_ninode >= SuperBlock::MAX_NUMBER_INODE)
        return;
    sb->s_inode[sb->s_ninode++] = number;
    sb->s_fmod = 1;
}

void FileSystem::Free(int blkno) {
    Buf* pCache;
    SuperBlock *sb;
    sb = &Kernel::Instance().GetSuperBlock();
    if (sb->s_nfree >= SuperBlock::MAX_NUMBER_FREE) {
        pCache = this->m_BufferManager->GetBlk(blkno);
        int* p = (int*)pCache->b_addr;
        *p++ = sb->s_nfree;
        memcpy(p, sb->s_free, sizeof(int) * SuperBlock::MAX_NUMBER_FREE);
        sb->s_nfree = 0;
        this->m_BufferManager->Bwrite(pCache);
    }

    sb->s_free[sb->s_nfree++] = blkno;
    sb->s_fmod = 1;
}

void FileSystem::Update() {
    Buf* pCache;
    SuperBlock *sb;
    sb = &Kernel::Instance().GetSuperBlock();
    sb->s_fmod = 0;
    sb->s_time = (int)time(NULL);
    for (int j = 0; j < 2; j++) {
        int* p = (int*)sb + j * 128;
        pCache = this->m_BufferManager->GetBlk(FileSystem::SUPER_BLOCK_START_SECTOR + j);
        memcpy(pCache->b_addr, p, BLOCK_SIZE);
        this->m_BufferManager->Bwrite(pCache);
    }
    g_InodeTable.UpdateINodeTable();
    this->m_BufferManager->Bflush();

}
//直接磁盘读入即可
void FileSystem::LoadSuperBlock() {
    SuperBlock *sb=&Kernel::Instance().GetSuperBlock();
    DiskDriver* diskDriver = &Kernel::Instance().GetDiskDriver();
    fseek(diskDriver->diskPointer, 0, 0);
    diskDriver->read((uint8_t*)(sb), sizeof(SuperBlock), 0);
}

//格式化整个文件系统
void FileSystem::FormatDevice() {
    //格式化超级块
    FormatSuperBlock();
    DiskDriver* diskDriver = &Kernel::Instance().GetDiskDriver();
    //写入(先占位)
    diskDriver->Construct();
    SuperBlock *sb=&Kernel::Instance().GetSuperBlock();
    diskDriver->write((uint8_t*)(sb), sizeof(SuperBlock), 0);
    //写入根目录
    DiskINode emptyDINode, rootDINode;
    rootDINode.i_mode |= INode::IALLOC | INode::IFDIR;
    diskDriver->write((uint8_t*)&rootDINode, sizeof(rootDINode));
    //写入其它空闲Inode
    for (int i = 1; i < FileSystem::INODE_NUMBER_ALL; ++i) {
        if (sb->s_ninode < SuperBlock::MAX_NUMBER_INODE)
            sb->s_inode[sb->s_ninode++] = i;
        diskDriver->write((uint8_t*)&emptyDINode, sizeof(emptyDINode));
    }
    //写入空闲磁盘块
    char freeBlock[BLOCK_SIZE], freeBlock1[BLOCK_SIZE];
    memset(freeBlock, 0, BLOCK_SIZE);
    memset(freeBlock1, 0, BLOCK_SIZE);

    for (int i = 0; i < FileSystem::DATA_SECTOR_NUMBER; ++i) {
        if (sb->s_nfree >= SuperBlock::MAX_NUMBER_FREE) {
            //记录空闲磁盘块的数量
            memcpy(freeBlock1, &sb->s_nfree, sizeof(int) + sizeof(sb->s_free));
            diskDriver->write((uint8_t*)&freeBlock1, BLOCK_SIZE);
            sb->s_nfree = 0;
        }
        else
            diskDriver->write((uint8_t*)freeBlock, BLOCK_SIZE);
        sb->s_free[sb->s_nfree++] = i + FileSystem::DATA_START_SECTOR;
    }

    //确定时间后写入超级块
    time((time_t*)&sb->s_time);
    diskDriver->write((uint8_t*)(sb), sizeof(SuperBlock), 0);
}

void FileSystem::FormatSuperBlock() {
    SuperBlock *sb=&Kernel::Instance().GetSuperBlock();
    sb->s_isize = FileSystem::INODE_SECTOR_NUMBER;
    sb->s_fsize = FileSystem::DISK_SECTOR_NUMBER;
    sb->s_nfree = 0;
    sb->s_free[0] = -1;
    sb->s_ninode = 0;
    sb->s_fmod = 0;
    time((time_t*)&sb->s_time);
}

FileSystem::FileSystem() {
    updlock=0;
    this->m_BufferManager = &Kernel::Instance().GetBufferManager();
    DiskDriver* diskDriver = &Kernel::Instance().GetDiskDriver();
    if (!diskDriver->Exists())
        FormatDevice();
    else
        LoadSuperBlock();
}

FileSystem::~FileSystem() {
    Update();
}
