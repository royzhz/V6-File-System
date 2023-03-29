//
// Created by roy on 26/03/2023.
//

#ifndef OSFILE_OPENFILEMANAGER_H
#define OSFILE_OPENFILEMANAGER_H

#include "INode.h"
#include "File.h"
#include "FileSystem.h"

class INodeTable;
class OpenFileTable;

extern INodeTable g_InodeTable;
extern OpenFileTable g_OpenFileTable;

class INodeTable {
public:
    static const int NINODE = 100;           //内存INode的数量
private:
    INode m_INodeTable[NINODE];              //内存INode数组，每个打开文件都会占用一个内存INode
    FileSystem* fileSystem;                  //对全局对象g_FileSystem的引用

public:
    INodeTable();
    ~INodeTable();
    INode* IGet(int inumber);                //根据外存INode编号获取对应INode。如果该INode已经在内存中，返回该内存INode；
    //如果不在内存中，则将其读入内存后上锁并返回该内存INode
    void IPut(INode* pNode);                 //减少该内存INode的引用计数，如果此INode已经没有目录项指向它，
    //且无进程引用该INode，则释放此文件占用的磁盘块
    void UpdateINodeTable();                 //将所有被修改过的内存INode更新到对应外存INode中
    int IsLoaded(int inumber);               //检查编号为inumber的外存INode是否有内存拷贝，
    //如果有则返回该内存INode在内存INode表中的索引
    INode* GetFreeINode();                   //在内存INode表中寻找一个空闲的内存INode
    void Reset();
};

class OpenFileTable
{
public:
    /* static consts */
    //static const int NINODE	= 100;	/* 内存Inode的数量 */
    static const int NFILE	= 100;	/* 打开文件控制块File结构的数量 */

    /* Functions */
public:
    /* Constructors */
    OpenFileTable();
    /* Destructors */
    ~OpenFileTable();

    void Reset();

    // /*
    // * @comment 根据用户系统调用提供的文件描述符参数fd，
    // * 找到对应的打开文件控制块File结构
    // */
    // File* GetF(int fd);
    /*
     * @comment 在系统打开文件表中分配一个空闲的File结构
     */
    File* FAlloc();
    /*
     * @comment 对打开文件控制块File结构的引用计数f_count减1，
     * 若引用计数f_count为0，则释放File结构。
     */
    void CloseF(File* pFile);

    /* Members */
public:
    File m_File[NFILE];			/* 系统打开文件表，为所有进程共享，进程打开文件描述符表
								中包含指向打开文件表中对应File结构的指针。*/
};




#endif //OSFILE_OPENFILEMANAGER_H
