//
// Created by roy on 26/03/2023.
//

#ifndef OSFILE_FILESYSTEM_H
#define OSFILE_FILESYSTEM_H
#include "INode.h"


class SuperBlock
{
public:
    const static int MAX_NUMBER_FREE = 100;
    const static int MAX_NUMBER_INODE = 100;

public:
    int	s_isize;		        //外存INode区占用的盘块数 1022
    int	s_fsize;		        //文件系统的数据盘块总数  16384 - 1024 = 15360

    int	s_nfree;		        //直接管理的空闲盘块数量
    int	s_free[MAX_NUMBER_FREE];//直接管理的空闲盘块索引表

    int	s_ninode;		          //直接管理的空闲外存INode数量
    int	s_inode[MAX_NUMBER_INODE];//直接管理的空闲外存INode索引表


    int	s_fmod;			        //内存中super block副本被修改标志，意味着需要更新外存对应的Super Block
    int	s_time;			        //最近一次更新时间
    int	padding[50];	        //填充使SuperBlock块大小等于1024字节，占据2个扇区
};


class FileSystem {
public:
    static const int BLOCK_SIZE = 512;           //Block块大小，单位字节
    static const int DISK_SECTOR_NUMBER = 16384; //磁盘所有扇区数量 8MB / 512B = 16384
    static const int SUPER_BLOCK_START_SECTOR = 0;//定义SuperBlock位于磁盘上的扇区号，占据两个扇区
    static const int INODE_START_SECTOR = 2;     //外存INode区位于磁盘上的起始扇区号
    static const int INODE_SECTOR_NUMBER = 1022; //磁盘上外存INode区占据的扇区数
    static const int INODE_SIZE = sizeof(DiskINode);//64字节
    static const int INODE_NUMBER_PER_SECTOR = BLOCK_SIZE / INODE_SIZE;//外存INode对象长度为64字节，每个磁盘块可以存放512/64 = 8个外存INode
    static const int ROOT_INODE_NO = 0;          //文件系统根目录外存INode编号
    static const int INODE_NUMBER_ALL = INODE_SECTOR_NUMBER * INODE_NUMBER_PER_SECTOR; //外存INode的总个数
    static const int DATA_START_SECTOR = INODE_START_SECTOR + INODE_SECTOR_NUMBER;     //数据区的起始扇区号
    static const int DATA_END_SECTOR = DISK_SECTOR_NUMBER - 1;                         //数据区的最后扇区号
    static const int DATA_SECTOR_NUMBER = DISK_SECTOR_NUMBER - DATA_START_SECTOR;      //数据区占据的扇区数量

    FileSystem();
    ~FileSystem();

    void FormatSuperBlock();//格式化SuperBlock
    void FormatDevice();    //格式化整个文件系统

    /* 在操作系统初始化时，会将磁盘的SuperBlock读入一个内存的SuperBlock副本，以便于内核以更快的速度随时访问内存副本。
    一旦内存中的副本发生变化，会通过设置s_fmod标志，由内核将内存副本写入磁盘 */
    void LoadSuperBlock();  //系统初始化时读入SuperBlock
    void Update();          //将SuperBlock对象的内存副本更新到存储设备的SuperBlock中去

    /* 磁盘Inode节点的分配与回收算法设计与实现 */
    INode* IAlloc();        //在存储设备上分配一个空闲外存INode，一般用于创建新的文件
    void IFree(int number); //释放编号为number的外存INode，一般用于删除文件
    Buf* Alloc();    //在存储设备上分配空闲磁盘块
    void Free(int blkno);   //释放存储设备上编号为blkno的磁盘块
private:
    BufferManager* m_BufferManager;		/* FileSystem类需要缓存管理模块(BufferManager)提供的接口 */
    int updlock;				/* Update()函数的锁，该函数用于同步内存各个SuperBlock副本以及，
								被修改过的内存Inode。任一时刻只允许一个进程调用该函数 */
};



#endif //OSFILE_FILESYSTEM_H
