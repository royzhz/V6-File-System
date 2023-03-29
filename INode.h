//
// Created by roy on 26/03/2023.
//

#ifndef OSFILE_INODE_H
#define OSFILE_INODE_H

#include "BufferManager.h"



class INode
{
public:

    //INodeFlag中的标志位
    enum INodeFlag
    {
        IUPD = 0x1,//内存INode被修改过，需要更新对应外存INode
        IACC = 0x2 //内存INode被访问过，需要修改最近一次访问时间
    };

    static const unsigned int IALLOC = 0x8000;		/* 文件被使用 */
    static const unsigned int IFMT = 0x6000;		/* 文件类型掩码 */
    static const unsigned int IFDIR = 0x4000;		/* 文件类型：目录文件 */
    static const unsigned int IFCHR = 0x2000;		/* 字符设备特殊类型文件 */
    static const unsigned int IFBLK = 0x6000;		/* 块设备特殊类型文件，为0表示常规数据文件 */
    static const unsigned int ILARG = 0x1000;		/* 文件长度类型：大型或巨型文件 */
    static const unsigned int ISUID = 0x800;		/* 执行时文件时将用户的有效用户ID修改为文件所有者的User ID */
    static const unsigned int ISGID = 0x400;		/* 执行时文件时将用户的有效组ID修改为文件所有者的Group ID */
    static const unsigned int ISVTX = 0x200;		/* 使用后仍然位于交换区上的正文段 */
    static const unsigned int IREAD = 0x100;		/* 对文件的读权限 */
    static const unsigned int IWRITE = 0x80;		/* 对文件的写权限 */
    static const unsigned int IEXEC = 0x40;			/* 对文件的执行权限 */
    static const unsigned int IRWXU = (IREAD|IWRITE|IEXEC);		/* 文件主对文件的读、写、执行权限 */
    static const unsigned int IRWXG = ((IRWXU) >> 3);			/* 文件主同组用户对文件的读、写、执行权限 */
    static const unsigned int IRWXO = ((IRWXU) >> 6);			/* 其他用户对文件的读、写、执行权限 */

    static const int BLOCK_SIZE = 512;                                        //文件逻辑块大小：512字节
    static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int);      //每个间接索引表（或索引块）包含的物理盘块号

    static const int SMALL_FILE_BLOCK = 6;                                    //小型文件：直接索引表最多可寻址的逻辑块号
    static const int LARGE_FILE_BLOCK = 128 * 2 + 6;                          //大型文件：经一次间接索引表最多可寻址的逻辑块号
    static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6;
    //巨型文件：经二次间接索引最大可寻址文件逻辑块号

    /* static member */
    static int rablock;		/* 顺序读时，使用预读技术读入文件的下一字符块，rablock记录了下一逻辑块号
							经过bmap转换得到的物理盘块号。将rablock作为静态变量的原因：调用一次bmap的开销
							对当前块和预读块的逻辑块号进行转换，bmap返回当前块的物理盘块号，并且将预读块
							的物理盘块号保存在rablock中。 */

    char i_flag;//标志
    char i_count ;//引用计数
    //int i_dev;//设备号
    int i_number;//Inode号
    int i_mode ;//文件类型、权限
    char i_nlink;//文件的链接数
    char i_uid;//用户ID
    char i_gid;//组ID
    int i_size0 ;//文件长度的高位8
    //char *i_size1;//文件长度的低位16比特
    int i_addr [10];//文件数据的物理盘块号
    int i_lastr;//最近一次读取文件的逻辑块号


public:
    INode();
    ~INode();
    void Reset();
    void ReadI();                           //根据Inode对象中的物理磁盘块索引表，读取相应的文件数据
    void WriteI();                          //根据Inode对象中的物理磁盘块索引表，将数据写入文件
    int Bmap(int lbn);                      //将文件的逻辑块号转换成对应的物理盘块号
    void IUpdate(int time);                 //更新外存Inode的最后的访问时间、修改时间
    void ITrunc();                          //释放Inode对应文件占用的磁盘块
    void Clean();                           //清空Inode对象中的数据
    void ICopy(Buf* bp, int inumber);//将包含外存Inode字符块中信息拷贝到内存Inode中
};


class DiskINode//64字节
{
public:
    int     i_mode;//状态、控制信息。低位9比特表示权限。
    char    i_nlink ;//来自目录的参照数量
    char    i_uid;//用户ID
    char    i_gid;//组ID
    int    i_size0 ;//文件长度的高位8
    //char    *i_sizel ;//文件长度的低位16比特
    int     i_addr [10] ;//文件数据的物理盘块号
    int     i_atime ;//最近一次访问时间
    int     i_mtime ;//最近一次修改时间
    int     i_ctime ;//最近一次改变时间
public:
    DiskINode();
    ~DiskINode();
};

#endif //OSFILE_INODE_H
