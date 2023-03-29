//
// Created by roy on 26/03/2023.
//
#include "INode.h"
#include "Kernel.h"

int INode::rablock = 0;

void INode::IUpdate(int time) {
    Buf* pBuf;
    DiskINode dINode;
    FileSystem& filesys = Kernel::Instance().GetFileSystem();
    BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
    //当IUPD和IACC标志之一被设置，才需要更新相应DiskINode
    //目录搜索，不会设置所途径的目录文件的IACC和IUPD标志
    if (this->i_flag & (INode::IUPD | INode::IACC)) {
        pBuf = bufMgr.Bread(FileSystem::INODE_START_SECTOR + this->i_number / FileSystem::INODE_NUMBER_PER_SECTOR);
        dINode.i_mode = this->i_mode;
        dINode.i_nlink = this->i_nlink;
        dINode.i_size0 = this->i_size0;
        memcpy(dINode.i_addr, i_addr, sizeof(dINode.i_addr));
        if (this->i_flag & INode::IACC)
            dINode.i_atime = time;
        if (this->i_flag & INode::IUPD)
            dINode.i_mtime = time;

        unsigned char* p = pBuf->b_addr + (this->i_number % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskINode);
        DiskINode* pNode = &dINode;
        memcpy(p, pNode, sizeof(DiskINode));
        bufMgr.Bwrite(pBuf);
    }
}

//将文件的逻辑块号转换成对应的物理盘块号
int INode::Bmap(int lbn)
{
    Buf* pFirstBuf;
    Buf* pSecondBuf;
    int phyBlkno;	/* 转换后的物理盘块号 */
    int* iTable;	/* 用于访问索引盘块中一次间接、两次间接索引表 */
    int index;
    User& u = Kernel::Instance().GetUser();
    BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
    FileSystem& fileSys = Kernel::Instance().GetFileSystem();

    /* 
     * Unix V6++的文件索引结构：(小型、大型和巨型文件)
     * (1) i_addr[0] - i_addr[5]为直接索引表，文件长度范围是0 - 6个盘块；
     * 
     * (2) i_addr[6] - i_addr[7]存放一次间接索引表所在磁盘块号，每磁盘块
     * 上存放128个文件数据盘块号，此类文件长度范围是7 - (128 * 2 + 6)个盘块；
     *
     * (3) i_addr[8] - i_addr[9]存放二次间接索引表所在磁盘块号，每个二次间接
     * 索引表记录128个一次间接索引表所在磁盘块号，此类文件长度范围是
     * (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
     */

    if(lbn >= INode::HUGE_FILE_BLOCK)
    {
        u.u_error = User::U_EFBIG;
        return 0;
    }

    if(lbn < 6)		/* 如果是小型文件，从基本索引表i_addr[0-5]中获得物理盘块号即可 */
    {
        phyBlkno = this->i_addr[lbn];

        /* 
         * 如果该逻辑块号还没有相应的物理盘块号与之对应，则分配一个物理块。
         * 这通常发生在对文件的写入，当写入位置超出文件大小，即对当前
         * 文件进行扩充写入，就需要分配额外的磁盘块，并为之建立逻辑块号
         * 与物理盘块号之间的映射。
         */
        if( phyBlkno == 0 && (pFirstBuf = fileSys.Alloc()) != NULL )
        {
            /* 
             * 因为后面很可能马上还要用到此处新分配的数据块，所以不急于立刻输出到
             * 磁盘上；而是将缓存标记为延迟写方式，这样可以减少系统的I/O操作。
             */
            bufMgr.Bdwrite(pFirstBuf);
            phyBlkno = pFirstBuf->b_blkno;
            /* 将逻辑块号lbn映射到物理盘块号phyBlkno */
            this->i_addr[lbn] = phyBlkno;
            this->i_flag |= INode::IUPD;
        }
        /* 找到预读块对应的物理盘块号 */
        INode::rablock = 0;
        if(lbn <= 4)
        {
            /* 
             * i_addr[0] - i_addr[5]为直接索引表。如果预读块对应物理块号可以从
             * 直接索引表中获得，则记录在INode::rablock中。如果需要额外的I/O开销
             * 读入间接索引块，就显得不太值得了。漂亮！
             */
            INode::rablock = this->i_addr[lbn + 1];
        }

        return phyBlkno;
    }
    else	/* lbn >= 6 大型、巨型文件 */
    {
        /* 计算逻辑块号lbn对应i_addr[]中的索引 */

        if(lbn < INode::LARGE_FILE_BLOCK)	/* 大型文件: 长度介于7 - (128 * 2 + 6)个盘块之间 */
        {
            index = (lbn - INode::SMALL_FILE_BLOCK) / INode::ADDRESS_PER_INDEX_BLOCK + 6;
        }
        else	/* 巨型文件: 长度介于263 - (128 * 128 * 2 + 128 * 2 + 6)个盘块之间 */
        {
            index = (lbn - INode::LARGE_FILE_BLOCK) / (INode::ADDRESS_PER_INDEX_BLOCK * INode::ADDRESS_PER_INDEX_BLOCK) + 8;
        }

        phyBlkno = this->i_addr[index];
        /* 若该项为零，则表示不存在相应的间接索引表块 */
        if( 0 == phyBlkno )
        {
            this->i_flag |= INode::IUPD;
            /* 分配一空闲盘块存放间接索引表 */
            if( (pFirstBuf = fileSys.Alloc()) == NULL )
            {
                return 0;	/* 分配失败 */
            }
            /* i_addr[index]中记录间接索引表的物理盘块号 */
            this->i_addr[index] = pFirstBuf->b_blkno;
        }
        else
        {
            /* 读出存储间接索引表的字符块 */
            pFirstBuf = bufMgr.Bread(phyBlkno);
        }
        /* 获取缓冲区首址 */
        iTable = (int *)pFirstBuf->b_addr;

        if(index >= 8)	/* ASSERT: 8 <= index <= 9 */
        {
            /* 
             * 对于巨型文件的情况，pFirstBuf中是二次间接索引表，
             * 还需根据逻辑块号，经由二次间接索引表找到一次间接索引表
             */
            index = ( (lbn - INode::LARGE_FILE_BLOCK) / INode::ADDRESS_PER_INDEX_BLOCK ) % INode::ADDRESS_PER_INDEX_BLOCK;

            /* iTable指向缓存中的二次间接索引表。该项为零，不存在一次间接索引表 */
            phyBlkno = iTable[index];
            if( 0 == phyBlkno )
            {
                if( (pSecondBuf = fileSys.Alloc()) == NULL)
                {
                    /* 分配一次间接索引表磁盘块失败，释放缓存中的二次间接索引表，然后返回 */
                    bufMgr.Brelse(pFirstBuf);
                    return 0;
                }
                /* 将新分配的一次间接索引表磁盘块号，记入二次间接索引表相应项 */
                iTable[index] = pSecondBuf->b_blkno;
                /* 将更改后的二次间接索引表延迟写方式输出到磁盘 */
                bufMgr.Bdwrite(pFirstBuf);
            }
            else
            {
                /* 释放二次间接索引表占用的缓存，并读入一次间接索引表 */
                bufMgr.Brelse(pFirstBuf);
                pSecondBuf = bufMgr.Bread(phyBlkno);
            }

            pFirstBuf = pSecondBuf;
            /* 令iTable指向一次间接索引表 */
            iTable = (int *)pSecondBuf->b_addr;
        }

        /* 计算逻辑块号lbn最终位于一次间接索引表中的表项序号index */

        if( lbn < INode::LARGE_FILE_BLOCK )
        {
            index = (lbn - INode::SMALL_FILE_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;
        }
        else
        {
            index = (lbn - INode::LARGE_FILE_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;
        }

        if( (phyBlkno = iTable[index]) == 0 && (pSecondBuf = fileSys.Alloc()) != NULL)
        {
            /* 将分配到的文件数据盘块号登记在一次间接索引表中 */
            phyBlkno = pSecondBuf->b_blkno;
            iTable[index] = phyBlkno;
            /* 将数据盘块、更改后的一次间接索引表用延迟写方式输出到磁盘 */
            bufMgr.Bdwrite(pSecondBuf);
            bufMgr.Bdwrite(pFirstBuf);
        }
        else
        {
            /* 释放一次间接索引表占用缓存 */
            bufMgr.Brelse(pFirstBuf);
        }
        /* 找到预读块对应的物理盘块号，如果获取预读块号需要额外的一次for间接索引块的IO，不合算，放弃 */
        rablock = 0;
        if( index + 1 < INode::ADDRESS_PER_INDEX_BLOCK)
        {
            rablock = iTable[index + 1];
        }
        return phyBlkno;
    }
}

void INode::ITrunc() {
/* 经由磁盘高速缓存读取存放一次间接、两次间接索引表的磁盘块 */
    BufferManager& bm = Kernel::Instance().GetBufferManager();
    /* 获取g_FileSystem对象的引用，执行释放磁盘块的操作 */
    FileSystem& filesys = Kernel::Instance().GetFileSystem();

    /* 如果是字符设备或者块设备则退出 */
    if( this->i_mode & (INode::IFCHR & INode::IFBLK) )
    {
        return;
    }

    /* 采用FILO方式释放，以尽量使得SuperBlock中记录的空闲盘块号连续。
     * 
     * Unix V6++的文件索引结构：(小型、大型和巨型文件)
     * (1) i_addr[0] - i_addr[5]为直接索引表，文件长度范围是0 - 6个盘块；
     * 
     * (2) i_addr[6] - i_addr[7]存放一次间接索引表所在磁盘块号，每磁盘块
     * 上存放128个文件数据盘块号，此类文件长度范围是7 - (128 * 2 + 6)个盘块；
     *
     * (3) i_addr[8] - i_addr[9]存放二次间接索引表所在磁盘块号，每个二次间接
     * 索引表记录128个一次间接索引表所在磁盘块号，此类文件长度范围是
     * (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
     */
    for(int i = 9; i >= 0; i--)		/* 从i_addr[9]到i_addr[0] */
    {
        /* 如果i_addr[]中第i项存在索引 */
        if( this->i_addr[i] != 0 )
        {
            /* 如果是i_addr[]中的一次间接、两次间接索引项 */
            if( i >= 6 && i <= 9 )
            {
                /* 将间接索引表读入缓存 */
                Buf* pFirstBuf = bm.Bread(this->i_addr[i]);
                /* 获取缓冲区首址 */
                int* pFirst = (int *)pFirstBuf->b_addr;

                /* 每张间接索引表记录 512/sizeof(int) = 128个磁盘块号，遍历这全部128个磁盘块 */
                for(int j = 128 - 1; j >= 0; j--)
                {
                    if( pFirst[j] != 0)	/* 如果该项存在索引 */
                    {
                        /* 
                         * 如果是两次间接索引表，i_addr[8]或i_addr[9]项，
                         * 那么该字符块记录的是128个一次间接索引表存放的磁盘块号
                         */
                        if( i >= 8 && i <= 9)
                        {
                            Buf* pSecondBuf = bm.Bread(pFirst[j]);
                            int* pSecond = (int *)pSecondBuf->b_addr;

                            for(int k = 128 - 1; k >= 0; k--)
                            {
                                if(pSecond[k] != 0)
                                {
                                    /* 释放指定的磁盘块 */
                                    filesys.Free(pSecond[k]);
                                }
                            }
                            /* 缓存使用完毕，释放以便被其它进程使用 */
                            bm.Brelse(pSecondBuf);
                        }
                        filesys.Free(pFirst[j]);
                    }
                }
                bm.Brelse(pFirstBuf);
            }
            /* 释放索引表本身占用的磁盘块 */
            filesys.Free(this->i_addr[i]);
            /* 0表示该项不包含索引 */
            this->i_addr[i] = 0;
        }
    }

    /* 盘块释放完毕，文件大小清零 */
    this->i_size0 = 0;
    /* 增设IUPD标志位，表示此内存INode需要同步到相应外存INode */
    this->i_flag |= INode::IUPD;
    /* 清大文件标志 和原来的RWXRWXRWX比特*/
    this->i_mode &= ~(INode::ILARG & INode::IRWXU & INode::IRWXG & INode::IRWXO);
    this->i_nlink = 1;
}

INode::INode() {
    i_mode=0;
}

INode::~INode() {

}

void INode::Reset() {
    *this=INode();
}

void INode::ReadI() {
    int lbn;	/* 文件逻辑块号 */
    int bn;		/* lbn对应的物理盘块号 */
    int offset;	/* 当前字符块内起始传送位置 */
    int nbytes;	/* 传送至用户目标区字节数量 */
    Buf* pBuf;
    User& u = Kernel::Instance().GetUser();
    BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
    DiskDriver& devMgr = Kernel::Instance().GetDiskDriver();

    if( 0 == u.u_IOParam.m_Count )
    {
        /* 需要读字节数为零，则返回 */
        return;
    }

    this->i_flag |= INode::IACC;

    /* 一次一个字符块地读入所需全部数据，直至遇到文件尾 */
    while( User::U_NOERROR == u.u_error && u.u_IOParam.m_Count != 0)
    {
        lbn = bn = u.u_IOParam.m_Offset / INode::BLOCK_SIZE;
        offset = u.u_IOParam.m_Offset % INode::BLOCK_SIZE;
        /* 传送到用户区的字节数量，取读请求的剩余字节数与当前字符块内有效字节数较小值 */
        nbytes = std::min(INode::BLOCK_SIZE - offset /* 块内有效字节数 */, u.u_IOParam.m_Count);

        int remain = this->i_size0 - u.u_IOParam.m_Offset;
        /* 如果已读到超过文件结尾 */
        if( remain <= 0)
        {
            return;
        }
        /* 传送的字节数量还取决于剩余文件的长度 */
        nbytes = std::min(nbytes, remain);

        /* 将逻辑块号lbn转换成物理盘块号bn ，Bmap有设置Inode::rablock。当UNIX认为获取预读块的开销太大时，
         * 会放弃预读，此时 Inode::rablock 值为 0。
         * */
        if( (bn = this->Bmap(lbn)) == 0 )
        {
            return;
        }

        pBuf = bufMgr.Bread(bn);

        /* 记录最近读取字符块的逻辑块号 */
        this->i_lastr = lbn;

        /* 缓存中数据起始读位置 */
        unsigned char* start = pBuf->b_addr + offset;

        /* 读操作: 从缓冲区拷贝到用户目标区
         * i386芯片用同一张页表映射用户空间和内核空间，这一点硬件上的差异 使得i386上实现 iomove操作
         * 比PDP-11要容易许多*/
        memcpy(u.u_IOParam.m_Base, start,  nbytes);

        /* 用传送字节数nbytes更新读写位置 */
        u.u_IOParam.m_Base += nbytes;
        u.u_IOParam.m_Offset += nbytes;
        u.u_IOParam.m_Count -= nbytes;

        bufMgr.Brelse(pBuf);	/* 使用完缓存，释放该资源 */
    }
}

void INode::WriteI() {
    int lbn;	/* 文件逻辑块号 */
    int bn;		/* lbn对应的物理盘块号 */
    int offset;	/* 当前字符块内起始传送位置 */
    int nbytes;	/* 传送字节数量 */
    Buf* pBuf;
    User& u = Kernel::Instance().GetUser();
    BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
    DiskDriver& devMgr = Kernel::Instance().GetDiskDriver();

    /* 设置Inode被访问标志位 */
    this->i_flag |= (INode::IACC | INode::IUPD);

    if( 0 == u.u_IOParam.m_Count)
    {
        /* 需要读字节数为零，则返回 */
        return;
    }

    while( User::U_NOERROR == u.u_error && u.u_IOParam.m_Count != 0 )
    {
        lbn = u.u_IOParam.m_Offset / INode::BLOCK_SIZE;
        offset = u.u_IOParam.m_Offset % INode::BLOCK_SIZE;
        nbytes = std::min(INode::BLOCK_SIZE - offset, u.u_IOParam.m_Count);

        if( (this->i_mode & INode::IFMT) != INode::IFBLK )
        {	/* 普通文件 */

            /* 将逻辑块号lbn转换成物理盘块号bn */
            if( (bn = this->Bmap(lbn)) == 0 )
            {
                return;
            }
        }

        if(INode::BLOCK_SIZE == nbytes)
        {
            /* 如果写入数据正好满一个字符块，则为其分配缓存 */
            pBuf = bufMgr.GetBlk(bn);
        }
        else
        {
            /* 写入数据不满一个字符块，先读后写（读出该字符块以保护不需要重写的数据） */
            pBuf = bufMgr.Bread(bn);
        }

        /* 缓存中数据的起始写位置 */
        unsigned char* start = pBuf->b_addr + offset;

        /* 写操作: 从用户目标区拷贝数据到缓冲区 */
        memcpy(start,u.u_IOParam.m_Base,  nbytes);

        /* 用传送字节数nbytes更新读写位置 */
        u.u_IOParam.m_Base += nbytes;
        u.u_IOParam.m_Offset += nbytes;
        u.u_IOParam.m_Count -= nbytes;

        if( u.u_error != User::U_NOERROR )	/* 写过程中出错 */
        {
            bufMgr.Brelse(pBuf);
        }
        bufMgr.Bdwrite(pBuf);


        /* 普通文件长度增加 */
        if( (this->i_size0 < u.u_IOParam.m_Offset) && (this->i_mode & (INode::IFBLK & INode::IFCHR)) == 0 )
        {
            this->i_size0 = u.u_IOParam.m_Offset;
        }

        this->i_flag |= INode::IUPD;
    }
}

void INode::Clean() {
    this->i_mode = 0;
    //this->i_count = 0;
    this->i_nlink = 0;
    //this->i_dev = -1;
    //this->i_number = -1;
    this->i_uid = -1;
    this->i_gid = -1;
    this->i_size0 = 0;
    this->i_lastr = -1;
    for(int i = 0; i < 10; i++)
    {
        this->i_addr[i] = 0;
    }
}

void INode::ICopy(Buf *bp, int inumber) {
    DiskINode dInode;
    DiskINode* pNode = &dInode;

    /* 将p指向缓存区中编号为inumber外存Inode的偏移位置 */
    unsigned char* p = bp->b_addr + (inumber % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskINode);
    /* 将缓存中外存Inode数据拷贝到临时变量dInode中，按4字节拷贝 */
    memcpy((int *)pNode, (int *)p, sizeof(DiskINode)/sizeof(int));

    //DiskINode& dINode = *(DiskINode*)(bp->b_addr + (inumber % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskINode));
    /* 将外存Inode变量dInode中信息复制到内存Inode中 */
    this->i_mode = dInode.i_mode;
    this->i_nlink = dInode.i_nlink;
    this->i_uid = dInode.i_uid;
    this->i_gid = dInode.i_gid;
    this->i_size0 = dInode.i_size0;
    for(int i = 0; i < 10; i++)
    {
        this->i_addr[i] = dInode.i_addr[i];
    }
}

DiskINode::DiskINode() {
    i_mode=0;
}

DiskINode::~DiskINode() {

}
