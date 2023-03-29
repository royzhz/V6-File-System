//
// Created by roy on 27/03/2023.
//

#ifndef OSFILE_FILEMANAGER_H
#define OSFILE_FILEMANAGER_H

#include "File.h"
#include "OpenFileManager.h"

class DirectoryEntry
{
    /* static members */
public:
    static const int DIRSIZ = 28;	/* 目录项中路径部分的最大字符串长度 */

    /* Functions */
public:
    /* Constructors */
    DirectoryEntry();
    /* Destructors */
    ~DirectoryEntry();

    /* Members */
public:
    int m_ino;		/* 目录项中Inode编号部分 */
    char m_name[DIRSIZ];	/* 目录项中路径名部分 */
};

class FileManager
{
public:
    /* 目录搜索模式，用于NameI()函数 */
    enum DirectorySearchMode
    {
        OPEN = 0,		/* 以打开文件方式搜索目录 */
        CREATE = 1,		/* 以新建文件方式搜索目录 */
        DELETE = 2		/* 以删除文件方式搜索目录 */
    };

    /* Functions */
public:
    /* Constructors */
    FileManager();
    /* Destructors */
    ~FileManager();


    /*
     * @comment Open()系统调用处理过程
     */
    void Open();

    /*
     * @comment Creat()系统调用处理过程
     */
    void Creat();

    /*
     * @comment Open()、Creat()系统调用的公共部分
     */
    void Open1(INode* pInode, int mode, int trf);

    /*
     * @comment Close()系统调用处理过程
     */
    void Close();

    /*
     * @comment Seek()系统调用处理过程
     */
    void Seek();

    /*
     * @comment Read()系统调用处理过程
     */
    void Read();

    /*
     * @comment Write()系统调用处理过程
     */
    void Write();

    /*
     * @comment 读写系统调用公共部分代码
     */
    void Rdwr(enum File::FileFlags mode);

    /*
     * @comment 目录搜索，将路径转化为相应的Inode，
     * 返回上锁后的Inode
     */
    INode* NameI(enum DirectorySearchMode mode);

    /*
     * @comment 获取路径中的下一个字符
     */
    static char NextChar();

    /*
     * @comment 被Creat()系统调用使用，用于为创建新文件分配内核资源
     */
    INode* MakNode(unsigned int mode);

    /*
     * @comment 向父目录的目录文件写入一个目录项
     */
    void WriteDir(INode* pInode);

    /*
     * @comment 设置当前工作路径
     */
    void SetCurDir(char* pathname);

    /* 改变当前工作目录 */
    void ChDir();

    /* 取消文件 */
    void UnLink();

    /* 查看目录下所有文件*/
    void Ls();

public:
    /* 根目录内存Inode */
    INode* rootDirInode;

    /* 对全局对象g_FileSystem的引用，该对象负责管理文件系统存储资源 */
    FileSystem* m_FileSystem;

    /* 对全局对象g_InodeTable的引用，该对象负责内存Inode表的管理 */
    INodeTable* m_InodeTable;

    /* 对全局对象g_OpenFileTable的引用，该对象负责打开文件表项的管理 */
    OpenFileTable* m_OpenFileTable;
};


#endif //OSFILE_FILEMANAGER_H
